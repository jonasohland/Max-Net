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

    /**
     * A `shared_ptr` to a `asio::steady_timer`. Returned by locking a weak_ptr
     * a temporary waiting_timer or repeating_timer returned when scheduling a
     * callback. The callback also owns an instance a shared_ptr to the
     * `asio::steady_timer` it is scheduled on. This makes sure that the timer
     * will not be deleted as long as a callback is scheduled on it
     */
    using steady_timer = std::shared_ptr<boost::asio::steady_timer>;

    /**
     * A `weak_ptr` to a `asio::steady_timer`. Returned by a temporary
     * waiting_timer or repeating_timer when scheduling a callback.
     * The scheduled callback owns the shared_ptr this weak_ptr was created
     * from. So the weak_ptr could be expired by the time the callback has ran.
     * A common way to cancel a callback that may has or has not yet run could
     * look like this:
     * \code{.cpp}
     *      auto weak_timer = o::io::wait(ctx, 5s).then(callback())
     *      if(auto timer = weak_timer.lock()) timer->cancel();
     * \endcode
     */
    using weak_steady_timer = steady_timer::weak_type;

    /**
     * A `shared_ptr` to a
     * `boost::asio::basic_waitable_timer<std::chrono::system_clock>`
     */
    using system_timer = std::shared_ptr<
        boost::asio::basic_waitable_timer<std::chrono::system_clock>>;

    /**
     * A `weak_ptr` to a
     * `boost::asio::basic_waitable_timer<std::chrono::system_clock>`
     */
    using weak_system_timer = system_timer::weak_type;

    /**
     * A `shared_ptr` to a
     * `boost::asio::basic_waitable_timer<std::chrono::high_resolution_clock>`
     */
    using high_resolution_timer = std::shared_ptr<
        boost::asio::basic_waitable_timer<std::chrono::high_resolution_clock>>;

    /**
     * A `weak_ptr` to a
     * `boost::asio::basic_waitable_timer<std::chrono::high_resolution_clock>`
     */
    using weak_high_resolution_timer = high_resolution_timer::weak_type;

    /**
     * An abstract class that can be implemented for classes that want to act as
     * a callback for repeating timers.
     *
     * @author  Jonas Ohland
     * @date    27.03.2019
     */
    struct repeating_timer_callback {

        /**
         * Implement to perform the actual callback action.
         *
         * @author  Jonas Ohland
         * @date    27.03.2019
         *
         * @param   ec indicates an error or that the operation was cancelled
         *             from outside.
         *
         * @returns True if this should be the last operation.
         *          False otherwise.
         */
        virtual bool operator()(boost::system::error_code ec) = 0;
    };

    /**
     * An abstract class that can be implemented for classes that want to act as
     * a callback for waiting timers.
     *
     * @author  Jonas Ohland
     * @date    27.03.2019
     */
    struct timer_callback {

        /**
         * Implement to perform the actual callback action.
         *
         * @author  Jonas Ohland
         * @date    27.03.2019
         *
         * @param   ec indicates an error or that the operation was cancelled
         *             from outside.
         */
        virtual void operator()(boost::system::error_code ec) = 0;
    };

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
             * construct by reusing an existing timer object
             *
             * @author  Jonas Ohland
             * @date    27.03.2019
             *
             * @param [in,out]  ctx The context.
             * @param           d   A Duration to process.
             */
            waiting_timer(
                std::shared_ptr<boost::asio::basic_waitable_timer<Clock>> timer,
                Duration d)
                : wait_time_(d), temp_timer_(timer) {}

            /**
             * construct by reusing an existing timer object
             *
             * @author  Jonas Ohland
             * @date    27.03.2019
             *
             * @param [in,out]  ctx The context.
             * @param           d   A Duration to process.
             */
            waiting_timer(
                std::shared_ptr<boost::asio::basic_waitable_timer<Clock>> timer)
                : wait_time_(Duration::zero()), temp_timer_(timer) {}

            /**
             * Schedule the given handler
             *
             * @tparam  Handler Type of the handler.
             * @param [in,out]  handler The handler.
             *
             * @returns A std::shared_ptr&lt;waitable_timer_type&gt;
             */
            template <typename Handler>
            std::weak_ptr<waitable_timer_type> then(Handler&& handler) {

                // the wait handler must have the following signature:
                // void(boost::system::error_code)
                static_assert(o::type_traits::is_invocable<
                                  Handler, boost::system::error_code>::value,
                              "wait handler requirements not met");

                auto output = std::weak_ptr<waitable_timer_type>(temp_timer_);

                if (wait_time_.count()) temp_timer_->expires_after(wait_time_);

                temp_timer_->async_wait(
                    [handler_l = handler, capt_timer = std::move(temp_timer_)](
                        boost::system::error_code ec) mutable {
                        handler_l(ec);
                    });

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
                 * Construct a new recursor that takes ownership of all
                 * the resources of the last one.
                 *
                 * @author  Jonas Ohland
                 * @date    27.03.2019
                 *
                 * @param [in]  input_handler   The user-handler.
                 * @param [in]  input_duration  The duration between repeated
                 *                              calls.
                 * @param [in]  timer           The a shared_ptr to the timer
                 *                              that will call this recursor.
                 */
                recursor(Handler&& input_handler, Duration&& input_duration,
                         std::shared_ptr<waitable_timer_type>&& timer)
                    : handler(std::forward<Handler>(input_handler))
                    , duration(std::forward<Duration>(input_duration))
                    , tm_(std::forward<std::shared_ptr<waitable_timer_type>>(
                          timer)) {}

                /**
                 * default copy constructor
                 *
                 * @author  Jonas Ohland
                 * @date    27.03.2019
                 *
                 * @param   other   The other.
                 */
                recursor(const recursor& other) = default;

                /**
                 * Function call operator. This will call the user-handler and
                 * reschedule the call for a new execution if the handler
                 * returned false. This will transfer ownership of all resources
                 * of the current instance to the next WaitHandler waiting on
                 * this timer, leaving this instance in an invalid state.
                 * Calling this twice from the same instances results in
                 * undefined behaviour and the end of all life in the universe.
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
                        "handler return value must be convertible to bool or "
                        "of type boost::system::error_code");

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
            std::weak_ptr<waitable_timer_type> repeat(Handler&& handler) {

                // the repeat handler must have the following signature:
                // bool(boost::system::error_code) or
                // boost::system::error_code(boost::system::error_code)
                static_assert(o::type_traits::is_invocable<
                                  Handler, boost::system::error_code>::value,
                              "repeat handler requirements not met");

                auto output = std::weak_ptr<waitable_timer_type>(temp_timer_);

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
     * a callback at later time by reusing an existing timer. The caller must
     * ensure, that the timer is still valid.
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
    detail::waiting_timer<Duration, Clock>
    new_wait(std::shared_ptr<boost::asio::basic_waitable_timer<Clock>>& timer,
             Duration d) {
        return detail::waiting_timer<Duration, Clock>(timer, d);
    }

    /**
     * Create a new temporary waiting timer object, that may be used to execute
     * a callback at later time by reusing an existing timer. The caller must
     * ensure, that the timer is still valid. The waiting period will begin when
     * the callback is assigned to the timer.
     *
     * @tparam  Duration    Type of the duration. Example:
     *                      std::chrono::milliseconds.
     * @tparam  Clock       Type of the clock.
     * @param [in]  ctx The asio::io_context to wait on.
     * @param           d   The duration to wait between the repeated calls.
     *
     * @returns A temporary waiting_timer object.
     */
    template <typename Duration = std::chrono::milliseconds,
              typename Clock = std::chrono::steady_clock>
    detail::waiting_timer<Duration, Clock>
    new_wait(std::shared_ptr<boost::asio::basic_waitable_timer<Clock>>& timer) {
        return detail::waiting_timer<Duration, Clock>(timer);
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

    /**
     * Cancel a weak_timer if it is not expired.
     * This effectively locks the weak_ptr and checks if it is valid.
     * If it is it cancels the timer.
     *
     * @tparam  Timer   Type of the timer.
     * @param [in,out]  timer   The timer to cancel.
     */
    template <typename Timer>
    void weak_timer_cancel(Timer& timer) {
        if (auto locked_timer = timer.lock()) locked_timer->cancel();
    }

    /**
     * Cancel a timer if it is not expired.
     *
     * @tparam  Timer   Type of the timer.
     * @param [in,out]  timer   The timer.
     */
    template <typename Timer>
    void timer_cancel(Timer& timer) {
        if (timer) timer->cancel();
    }

    template <typename... Args>
    decltype(auto) timer_cb_bind(Args... args) {
        return std::bind(std::forward<Args>(args)..., std::placeholders::_1);
    }

} // namespace o::io
