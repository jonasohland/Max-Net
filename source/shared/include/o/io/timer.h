#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>

namespace o::io {

    namespace detail {

        template <typename Duration>
        class waiting_timer {

          public:
            waiting_timer(boost::asio::io_context& ctx, Duration d)
            : temp_timer_(new boost::asio::steady_timer(ctx)) {
                temp_timer_->expires_after(d);
            }

            template <typename Handler>
            std::shared_ptr<boost::asio::steady_timer> then(Handler&& handler) {

                auto output = temp_timer_;

                temp_timer_->async_wait([
                    handler_l = handler, capt_timer = std::move(temp_timer_)
                ](boost::system::error_code ec) { handler_l(ec); });

                return output;
            }

          private:
            std::shared_ptr<boost::asio::steady_timer> temp_timer_;
        };

        template <typename Duration>
        class repeating_timer {

          public:
            template <typename Handler>
            struct recursor {

                recursor(Handler&& input_handler, Duration input_duration,
                         std::shared_ptr<boost::asio::steady_timer>&& timer)
                    : handler(std::forward<Handler>(input_handler))
                    , duration(input_duration)
                    , tm_(timer) {}

                recursor(const recursor& other)
                    : handler(other.handler)
                    , duration(other.duration)
                    , tm_(other.tm_) {}

                void operator()(boost::system::error_code ec) {
                    if (!static_cast<bool>(handler(ec))) {
                        tm_->expires_after(duration);
                        tm_->async_wait(recursor<Handler>(
                            std::move(handler), duration, std::move(tm_)));
                    }
                }

                Handler handler;
                Duration duration;
                std::shared_ptr<boost::asio::steady_timer> tm_;
            };

            repeating_timer(boost::asio::io_context& ctx, Duration d)
                : temp_timer_(new boost::asio::steady_timer(ctx))
                , next_tick_(d) {
                temp_timer_->expires_after(d);
            }

            template <typename Handler>
            std::shared_ptr<boost::asio::steady_timer> then(Handler&& handler) {

                auto output = temp_timer_;

                temp_timer_->async_wait(
                    recursor<Handler>(std::forward<Handler>(handler),
                                      next_tick_, std::move(temp_timer_)));

                return output;
            }

          private:
            std::shared_ptr<boost::asio::steady_timer> temp_timer_;
            Duration next_tick_;
        };
    }

    template <typename Duration>
    detail::waiting_timer<Duration> wait(boost::asio::io_context& ctx,
                                         Duration d) {
        return detail::waiting_timer<Duration>(ctx, d);
    }

    template <typename Duration>
    detail::repeating_timer<Duration> repeat(boost::asio::io_context& ctx,
                                             Duration d) {
        return detail::repeating_timer<Duration>(ctx, d);
    }
}
