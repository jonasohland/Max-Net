#pragma once

/**
 * @file
 *
 * Declares some useful timer extensions
 */

#include "../types.h"
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <type_traits>

namespace o::io {

    namespace detail {

        /**
         * A waiting timer.
         *
         * @author  Jonas Ohland
         * @date    27.03.2019
         *
         * @tparam  Duration    Type of the duration.
         * @tparam  Clock       Type of the clock.
         */
        template <typename Duration, typename Clock>
        class waiting_timer {

            /** Type of the waitable timer */
            using waitable_timer_type =
                boost::asio::basic_waitable_timer<Clock>;

          public:
            /**
             * Constructor
             *
             * @author  Jonas Ohland
             * @date    27.03.2019
             *
             * @param [in,out]  ctx The context.
             * @param           d   A Duration to process.
             */
            waiting_timer(boost::asio::io_context& ctx, Duration d)
                : wait_time_(d), temp_timer_(new waitable_timer_type(ctx)) {}

            /**
             * Thens the given handler
             *
             * @tparam  Handler Type of the handler.
             * @param [in,out]  handler The handler.
             *
             * @returns A std::shared_ptr&lt;waitable_timer_type&gt;
             */
            template <typename Handler>
            std::shared_ptr<waitable_timer_type> then(Handler&& handler) {

                // the wait handler must have the following signature:
                // void(boost::system::error_code)
                static_assert(o::type_traits::is_invocable<
                                  Handler, boost::system::error_code>::value,
                              "wait handler requirements not met");

                auto output = temp_timer_;

                temp_timer_->expires_after(wait_time_);

                temp_timer_->async_wait(
                    [handler_l = handler, capt_timer = std::move(temp_timer_)](
                        boost::system::error_code ec) { handler_l(ec); });

                return output;
            }

          private:
            /** The wait time */
            Duration wait_time_;
            /** The temporary timer */
            std::shared_ptr<waitable_timer_type> temp_timer_;
        };

        /**
         * A temporary timer object, that will schedule a
         * boost::asio::basic_waitable_timer&lt;Clock&gt; when a callback is
         * assigned.
         *
         * @author  Jonas Ohland
         * @date    27.03.2019
         *
         * @tparam  Duration    Type of the duration.
         * @tparam  Clock       Type of the clock.
         */
        template <typename Duration, typename Clock>
        class repeating_timer {

          public:
            /** Type of the waitable timer */
            using waitable_timer_type =
                boost::asio::basic_waitable_timer<Clock>;

            /**
             * A functor object that will reschedule itself on a timer after a
             * given duration is over. This class can be used to invoke a
             * callback repeatedly from the timer.
             *
             * @author  Jonas Ohland
             * @date    27.03.2019
             *
             * @tparam  Handler Handler type. Might be a result from
             * `std::bind`, a lambda or a functor. Must have the signature:
             *                  `bool(boost::system::error_code)`.
             */
            template <typename Handler>
            struct recursor {

                /**
                 * Constructor
                 *
                 * @author  Jonas Ohland
                 * @date    27.03.2019
                 *
                 * @param [in]  input_handler   The input handler.
                 * @param       input_duration  Duration of the input.
                 * @param [in]  timer           The timer.
                 */
                recursor(Handler&& input_handler, Duration&& input_duration,
                         std::shared_ptr<waitable_timer_type>&& timer)
                    : handler(std::forward<Handler>(input_handler))
                    , duration(std::forward<Duration>(input_duration))
                    , tm_(std::forward<std::shared_ptr<waitable_timer_type>>(
                          timer)) {}

                /**
                 * Constructor
                 *
                 * @author  Jonas Ohland
                 * @date    27.03.2019
                 *
                 * @param   other   The other.
                 */
                recursor(const recursor& other) = default;

                /**
                 * Function call operator
                 *
                 * @author  Jonas Ohland
                 * @date    27.03.2019
                 *
                 * @param   ec  The ec.
                 */
                void operator()(boost::system::error_code ec) {

                    // If you hit a static_assert failure here, you propably
                    // forgot to return a value from the wait handler for
                    // .repeat(Handler) In most cases it should be suitable to
                    // simply return the error_code

                    static_assert(
                        std::is_convertible<decltype(handler(
                                                boost::system::error_code())),
                                            bool>::value ||
                            std::is_same<decltype(handler(
                                             boost::system::error_code())),
                                         boost::system::error_code>::value,
                        "handler must be convertible to bool or of "
                        "type boost::system::error_code");

                    if (!handler(ec)) {
                        tm_->expires_after(duration);
                        tm_->async_wait(recursor<Handler>(std::move(handler),
                                                          std::move(duration),
                                                          std::move(tm_)));
                    }
                }

                Handler handler;

                Duration duration;
                /** the timer to wait on */
                std::shared_ptr<waitable_timer_type> tm_;
            };

            /**
             * Constructor
             *
             * @author  Jonas Ohland
             * @date    27.03.2019
             *
             * @param [in,out]  ctx The context.
             * @param           d   A Duration to process.
             */
            repeating_timer(boost::asio::io_context& ctx, Duration d)
                : temp_timer_(new waitable_timer_type(ctx)), next_tick_(d) {}

            /**
             * Start repeating the given Handler. The handler must have the
             * following signature: \code{.cpp} bool
             * func(boost::system::error_code ec){
             *     return my_continue_condition  || ec;
             * }
             * \endcode
             *
             * @tparam  Handler Type of the handler.
             * @param [in]  handler The handler.
             *
             * @returns A `std::shared_ptr&lt;waitable_timer_type&gt;` from
             * which the Handler will be called.
             */
            template <typename Handler>
            std::shared_ptr<waitable_timer_type> repeat(Handler&& handler) {

                // the repeat handler must have the following signature:
                // bool(boost::system::error_code) or
                // boost::system::error_code(boost::system::error_code)
                static_assert(o::type_traits::is_invocable<
                                  Handler, boost::system::error_code>::value,
                              "repeat handler requirements not met");

                auto output = temp_timer_;

                temp_timer_->expires_after(next_tick_);

                temp_timer_->async_wait(recursor<Handler>(
                    std::forward<Handler>(handler), std::move(next_tick_),
                    std::move(temp_timer_)));

                return output;
            }

          private:
            /** The temporary timer */
            std::shared_ptr<waitable_timer_type> temp_timer_;
            /** The next tick */
            Duration next_tick_;
        };
    } // namespace detail

    /**
     * Create a new temporary waiting timer object, that may be used to execute
     * a callback at later time. The waiting period will begin when the callback
     * is assigned to the timer.
     *
     * @tparam  Duration    Type of the duration. Example:
     *                      std::chrono::milliseconds.
     * @tparam  Clock       Type of the clock.
     * @param [in]  ctx The asio::io_context to wait on.
     * @param           d   The duration to wait between the repeated calls.
     *
     * @returns A temporary waiting_timer object.
     */
    template <typename Duration, typename Clock = std::chrono::steady_clock>
    detail::waiting_timer<Duration, Clock> wait(boost::asio::io_context& ctx,
                                                Duration d) {
        return detail::waiting_timer<Duration, Clock>(ctx, d);
    }

    /**
     * Create a new temporary waiting timer object, that may be used to execute
     * a callback repeatedly. Repeated execution will start when the callback is
     * assigned. is assigned to the timer.
     *
     * @tparam  Duration    Type of the duration.
     * @tparam  Clock       Type of the clock.
     * @param [in]  ctx The `boost::asio::io_context` to wait on.
     * @param           d   The duration to wait.
     *
     * @returns A temporary repeating_timer object.
     */
    template <typename Duration, typename Clock = std::chrono::steady_clock>
    detail::repeating_timer<Duration, Clock> every(boost::asio::io_context& ctx,
                                                   Duration d) {
        return detail::repeating_timer<Duration, Clock>(ctx, d);
    }
} // namespace o::io
