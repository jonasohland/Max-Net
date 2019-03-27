#pragma once

#include "../../types.h"
#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <functional>

namespace o::io::net {

    /**
     * An UDP receiver.
     *
     * @author  Jonas Ohland
     * @date    16.03.2019
     *
     * @tparam  ThreadOption    Type of the thread option.
     */
    template <typename MessageContainer, typename ConcurrencyOption>
    class udp_device {

      public:
        struct use_v6 {};

        enum class error_case { connect, bind, read };

        udp_device() = delete;

        /**
         * Constructor
         *
         * @author  Jonas Ohland
         * @date    16.03.2019
         *
         * @param [in,out]  ctx The context.
         */
        udp_device(boost::asio::io_context& ctx) : sock_(ctx) {}

        /**
         * Handles data received signals
         *
         * @author  Jonas Ohland
         * @date    16.03.2019
         *
         * @param   parameter1  The first parameter.
         */
        virtual void on_data_received(MessageContainer&&) = 0;

        /**
         * Executes the UDP error action
         *
         * @author  Jonas Ohland
         * @date    16.03.2019
         *
         * @param   eca The eca.
         * @param   eco The eco.
         */
        virtual void on_udp_error(error_case eca,
                                  boost::system::error_code eco) {}

        /**
         * UDP bind
         *
         * @author  Jonas Ohland
         * @date    16.03.2019
         *
         * @param   remote_endp The remote endp.
         */
        void udp_bind(boost::asio::ip::udp::endpoint local_endp) {

            boost::system::error_code ec;

            if (!sock_.is_open()) sock_.open(local_endp.protocol());

            sock_.bind(local_endp, ec);

            if (ec) return on_udp_error(error_case::bind, ec);

            impl_udp_do_receive();
        }

        void udp_bind(short port) {
            udp_bind(boost::asio::ip::udp::endpoint(
                boost::asio::ip::udp::v4(), port));
        }

        void udp_bind(short port, use_v6 v6hint) {
            udp_bind(boost::asio::ip::udp::endpoint(
                boost::asio::ip::udp::v6(), port));
        }

        void udp_connect(boost::asio::ip::udp::endpoint remote_endp) {

            boost::system::error_code ec;

            if (!sock_.is_open()) sock_.open(remote_endp.protocol());

            sock_.connect(remote_endp, ec);

            if (ec) return on_udp_error(error_case::connect, ec);
        }

        /**
         * UDP close
         *
         * @author  Jonas Ohland
         * @date    16.03.2019
         */
        void udp_close() {
            if (sock_.is_open()) sock_.close();
        }

        void send(MessageContainer&& thing_to_send) {

            MessageContainer* output_str = new MessageContainer(
                std::forward<MessageContainer>(thing_to_send));

            sock_.async_send(
                boost::asio::buffer(output_str->data(), output_str->size()),
                std::bind(&udp_device::impl_udp_on_send_done, this,
                          std::placeholders::_1, output_str));
        }

        boost::asio::ip::udp::endpoint& last_remote() { return last_endp_; }

        boost::asio::basic_datagram_socket<boost::asio::ip::udp>& udp_sock() {
            return sock_;
        }

      private:
        // Data was received. Call the user-handler and do another receive.
        void udp_on_data_received(boost::system::error_code ec,
                                  size_t bytes_s) {

            if (ec) return on_udp_error(error_case::read, ec);

            buf_.commit(bytes_s);

            std::string out_bytes;

            on_data_received(boost::beast::buffers_to_string(buf_.data()));

            buf_.consume(bytes_s);

            impl_udp_do_receive();
        }

        void impl_udp_do_receive() {
            sock_.async_receive_from(
                buf_.prepare(1024), last_endp_,
                std::bind(&udp_device::udp_on_data_received, this,
                          std::placeholders::_1, std::placeholders::_2));
        }

        void impl_udp_on_send_done(boost::system::error_code ec,
                                   std::string* out_str) {
            delete out_str;
        }

        boost::asio::ip::udp::endpoint last_endp_;

        boost::asio::streambuf buf_;
        boost::asio::ip::udp::socket sock_;
    };

    template <typename ThreadOption>
    struct udp_port_mtx_base {};

    template <>
    struct udp_port_mtx_base<o::ccy::safe> {
        std::mutex udp_port_handler_mutex_;
    };

    template <>
    struct udp_port_mtx_base<o::ccy::unsafe> {};

    template <>
    struct udp_port_mtx_base<o::ccy::none> {};

    template <typename MessageContainer, typename ConcurrencyOption>
    class udp_port : public udp_device<MessageContainer, ConcurrencyOption>,
                     public udp_port_mtx_base<ConcurrencyOption> {

        using data_handler_type = std::function<void(std::string&&)>;
        using error_handler_type =
            std::function<void(boost::system::error_code)>;

      public:
        explicit udp_port(boost::asio::io_context& ctx)
            : udp_device<MessageContainer, ConcurrencyOption>(ctx) {}

        virtual void on_data_received(MessageContainer&& data) override {

            if (data_handler_) {

                opt_do_lock();

                data_handler_.get()(std::forward<MessageContainer>(data));

                opt_do_unlock();
            }
        }

        // TODO replace with std::optional and deprecate Xcode 9 (alias buy new
        // macbook)
        boost::optional<std::function<void(MessageContainer&&)>> data_handler_;
        boost::optional<std::function<void(boost::system::error_code)>>
            error_handler_;

        inline void set_data_handler(data_handler_type&& handler) {

            opt_do_lock();

            data_handler_ =
                boost::make_optional(std::forward<data_handler_type>(handler));

            opt_do_unlock();
        }

        inline void set_error_handler(error_handler_type&& handler) {

            opt_do_lock();

            error_handler_ =
                boost::make_optional(std::forward<error_handler_type>(handler));

            opt_do_unlock();
        }

      private:
        inline void opt_do_lock() {
            if constexpr (o::ccy::is_safe<ConcurrencyOption>::value)
                this->udp_port_handler_mutex_.lock();
        }

        inline void opt_do_unlock() {
            if constexpr (o::ccy::is_safe<ConcurrencyOption>::value)
                this->udp_port_handler_mutex_.unlock();
        }
    };

} // namespace o::io::net
