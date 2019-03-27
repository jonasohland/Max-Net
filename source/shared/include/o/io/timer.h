#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <type_traits>
#include "../types.h"

namespace o::io {

    namespace detail {

        template <typename Duration, typename Clock>
        class waiting_timer {
            
            using waitable_timer_type = boost::asio::basic_waitable_timer<Clock>;

          public:
            waiting_timer(boost::asio::io_context& ctx, Duration d)
                : wait_time_(d)
                , temp_timer_(new waitable_timer_type(ctx)) {}

            template <typename Handler>
            std::shared_ptr<waitable_timer_type> then(Handler&& handler) {

                // the wait handler must have the following signature:
                // void(boost::system::error_code)
                static_assert(o::type_traits::is_invocable<
                                  Handler, boost::system::error_code>::value,
                              "wait handler requirements not met");

                auto output = temp_timer_;
                
                temp_timer_->expires_after(wait_time_);

                temp_timer_->async_wait([
                    handler_l = handler, capt_timer = std::move(temp_timer_)
                ](boost::system::error_code ec) { handler_l(ec); });

                return output;
            }

          private:
            Duration wait_time_;
            std::shared_ptr<waitable_timer_type> temp_timer_;
        };

        template <typename Duration, typename Clock>
        class repeating_timer {

          public:
        
            using waitable_timer_type = boost::asio::basic_waitable_timer<Clock>;
            
            template <typename Handler>
            struct recursor {

                recursor(Handler&& input_handler, Duration input_duration,
                         std::shared_ptr<waitable_timer_type>&& timer)
                    : handler(std::forward<Handler>(input_handler))
                    , duration(input_duration)
                    , tm_(timer) {}

                recursor(const recursor& other)
                    : handler(other.handler)
                    , duration(other.duration)
                    , tm_(other.tm_) {}

                void operator()(boost::system::error_code ec) {
                    
                    // If you hit a static_assert failure here, you propably forgot
                    // to return a value from the wait handler for .repeat(Handler)
                    // In most cases it should be suitable to simply return the error_code

                    static_assert(
                        std::is_convertible<decltype(handler(
                                                boost::system::error_code())),
                                            bool>::value ||
                            std::is_same<decltype(handler(
                                             boost::system::error_code())),
                                         boost::system::error_code>::value,
                        "handler must be trivially convertible to bool or of "
                        "type boost::system::error_code");

                    if (!handler(ec)) {
                        tm_->expires_after(duration);
                        tm_->async_wait(recursor<Handler>(
                            std::move(handler), duration, std::move(tm_)));
                    }
                }

                Handler handler;
                Duration duration;
                std::shared_ptr<waitable_timer_type> tm_;
            };

            repeating_timer(boost::asio::io_context& ctx, Duration d)
                : temp_timer_(new waitable_timer_type(ctx))
                , next_tick_(d) {
            }

            template <typename Handler>
            std::shared_ptr<waitable_timer_type>
            repeat(Handler&& handler) {

                // the repeat handler must have the following signature:
                // bool(boost::system::error_code) or
                // boost::system::error_code(boost::system::error_code)
                static_assert(o::type_traits::is_invocable<
                                  Handler, boost::system::error_code>::value,
                              "repeat handler requirements not met");

                auto output = temp_timer_;
                
                temp_timer_->expires_after(next_tick_);

                temp_timer_->async_wait(
                    recursor<Handler>(std::forward<Handler>(handler),
                                      next_tick_, std::move(temp_timer_)));

                return output;
            }

          private:
            std::shared_ptr<waitable_timer_type> temp_timer_;
            Duration next_tick_;
        };
    }

    template <typename Duration, typename Clock = std::chrono::steady_clock>
    detail::waiting_timer<Duration, Clock> wait(boost::asio::io_context& ctx,
                                         Duration d) {
        return detail::waiting_timer<Duration, Clock>(ctx, d);
    }

    template <typename Duration, typename Clock = std::chrono::steady_clock>
    detail::repeating_timer<Duration, Clock> every(boost::asio::io_context& ctx,
                                            Duration d) {
        return detail::repeating_timer<Duration, Clock>(ctx, d);
    }
}
