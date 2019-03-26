//
// This file is part of the Max-Net Project
//
// Copyright (c) 2019, Jonas Ohland
//
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <mutex>
#include <boost/asio/io_context.hpp>
#include <boost/tti/tti.hpp>
#include <boost/mpl/lambda.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/type_traits/is_same.hpp>

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#define O_NET_POSIX
#endif

namespace o {

    namespace type_traits {

        template <typename F, typename... Args>
        struct is_invocable
            : std::is_constructible<
                  std::function<void(Args...)>,
                  std::reference_wrapper<
                      typename std::remove_reference<F>::type>> {};

        template <typename R, typename F, typename... Args>
        struct is_invocable_r
            : std::is_constructible<
                  std::function<R(Args...)>,
                  std::reference_wrapper<
                      typename std::remove_reference<F>::type>> {};
    }

    namespace messages {

        namespace detail {
            BOOST_TTI_HAS_MEMBER_FUNCTION(set_direction);
            BOOST_TTI_HAS_MEMBER_FUNCTION(notify_send);
            BOOST_TTI_HAS_MEMBER_FUNCTION(notify_send_done);
        } // namespace detail

        template <typename Message>
        using is_direction_supported = detail::has_member_function_set_direction<
            Message, void, boost::mpl::vector<bool>,
            boost::function_types::const_qualified>;

        template <typename Message, typename Ty = void>
        using enable_if_direction_supported =
            std::enable_if<is_direction_supported<Message>::value>;

        template <typename Message>
        using is_notify_send_supported = detail::has_member_function_notify_send<
            Message, void, boost::mpl::vector<>, boost::function_types::const_qualified>;

        template <typename Message>
        using is_notify_send_done_supported =
            detail::has_member_function_notify_send_done<
                Message, void, boost::mpl::vector<>,
                boost::function_types::const_qualified>;

        template <typename Message>
        struct is_send_notification_supported {
            static constexpr const bool value =
                is_notify_send_supported<Message>::value &&
                is_notify_send_done_supported<Message>::value;
        };
    } // namespace messages

    namespace sessions {

        namespace roles {
            struct server {};
            struct client {};
        } // namespace roles

        namespace features {
            struct timeout {};
            struct statistics {};
        } // namespace features

        template <typename Role, typename T = void>
        struct enable_for_client {};

        template <typename Ty>
        struct enable_for_client<roles::client, Ty> {
            using type = Ty;
        };

        template <typename Role, typename T = void>
        struct enable_for_server {};

        template <typename Ty>
        struct enable_for_server<roles::server, Ty> {
            using type = Ty;
        };
    } // namespace sessions

    namespace ccy {

        // used to indicate that the object will be used in a single threaded context
        struct unsafe {
            static constexpr const int io_ctx_ccy_hint = 1;
        };

        // obvious
        struct safe {
            static constexpr const int io_ctx_ccy_hint = BOOST_ASIO_CONCURRENCY_HINT_SAFE;
        };
        
        struct none {
            static constexpr const int io_ctx_ccy_hint = BOOST_ASIO_CONCURRENCY_HINT_SAFE;
        };

        template <typename T>
        struct is_safe : public std::false_type {};

        template <>
        struct is_safe<ccy::safe> : public std::true_type {};

        template <class Op, class Ty = void>
        struct opt_enable_if_safe {};

        template <class Ty>
        struct opt_enable_if_safe<ccy::safe, Ty> {
            using type = Ty;
        };

        template <class Op, class Ty = void>
        struct opt_enable_if_unsafe {};

        template <class Ty>
        struct opt_enable_if_unsafe<ccy::unsafe, Ty> {
            using type = Ty;
        };

        template <class Op, class Ty = void>
        struct opt_enable_if_ccy_aware {};

        template <class Ty>
        struct opt_enable_if_ccy_aware<ccy::unsafe, Ty> {
            using type = Ty;
        };

        template <class Ty>
        struct opt_enable_if_ccy_aware<ccy::safe, Ty> {
            using type = Ty;
        };
        
        template <class Op, class Ty = void>
        struct opt_enable_if_ccy_unaware {};
        
        template <class Ty>
        struct opt_enable_if_ccy_unaware<ccy::none, Ty> {
            using type = Ty;
        };

        namespace detail {
            BOOST_TTI_HAS_TYPE(ccy_option);
        }

        template <typename T>
        struct has_safe_opt {
            static constexpr const bool value = detail::has_type_ccy_option<
                T, boost::is_same<boost::mpl::placeholders::_1, safe>>::value;
        };

        template <typename T, typename Ty = void>
        using enable_if_has_safe_opt =
            typename std::enable_if<has_safe_opt<T>::value, Ty>;

        namespace detail {

            template <bool HasMutex, typename Mutex = std::mutex>
            class single_mtx_base;

            template <typename Mutex>
            class single_mtx_base<true, Mutex> {

              public:
                Mutex& mutex() { return opt_mtx_; }

              private:
                Mutex opt_mtx_;
            };

            template <typename Mutex>
            class single_mtx_base<false, Mutex> {};
        }

    } // namespace threads

    template <typename Thing, bool DoLock, typename Mutex = std::mutex>
    class safe_visitable : public ccy::detail::single_mtx_base<DoLock, Mutex> {

        Thing thing;

      public:
        template <typename... Args>
        explicit safe_visitable(Args... args) : thing(std::forward<Args>(args)...) {}

        template <bool Enable = DoLock, typename Visitor>
        typename std::enable_if<Enable>::type apply(Visitor v) {
            std::lock_guard<Mutex> visit_lock(this->mutex());
            v(thing);
        }

        template <bool Enable = DoLock, typename Visitor>
        typename std::enable_if<!(Enable)>::type apply(Visitor v) {
            v(thing);
        }

        template <bool Enable = DoLock, typename Visitor>
        typename std::enable_if<Enable>::type apply_adopt(Visitor v) {
            v(thing, this->mutex());
            //       ^^^^^^^^^^^^^
            // if you are getting an error like: "calling private constructor of 'mutex'"
            // here, you tried to visit the mutex by copying, which is illegal. a legal
            // lambda to use would be:
            //
            //     [](auto& thing, Mutex& mtx){
            //         thing->do_stuff();
            //     }
        }

        Thing* operator->() { return thing; }

        const Thing* operator->() const { return thing; }

        Thing& operator*() { return thing; }

        const Thing& operator*() const { return thing; }

        static constexpr bool locks_enabled() { return DoLock; }
    };

    template <typename Visitable, bool DoLock, typename Mutex = std::mutex>
    class enable_safe_visit : public ccy::detail::single_mtx_base<DoLock, Mutex> {

      public:
        template <bool Enable = DoLock, typename Visitor>
        typename std::enable_if<Enable>::type apply(Visitor v) {
            std::lock_guard<Mutex> visit_lock(this->mutex());
            v(*static_cast<Visitable*>(this));
        }

        template <bool Enable = DoLock, typename Visitor>
        typename std::enable_if<!(Enable)>::type apply(Visitor v) {
            v(*static_cast<Visitable*>(this));
        }

        template <bool Enable = DoLock, typename Visitor>
        typename std::enable_if<Enable>::type apply_adopt(Visitor v) {
            v(*static_cast<Visitable*>(this), this->mutex());
            //                                ^^^^^^^^^^^^^
            // if you are getting an error like: "calling private constructor of 'mutex'"
            // here, you tried to visit the mutex by copying, which is illegal. a legal
            // lambda to use would be:
            //
            //     [](auto& thing, Mutex& mtx){
            //         thing->do_stuff();
            //     }
        }

        static constexpr bool locks_enabled() { return DoLock; }
    };

    namespace ccy {
        template <typename Thing, typename ThreadOption, typename Mutex = std::mutex>
        struct opt_safe_visitable
            : public safe_visitable<Thing, ccy::is_safe<ThreadOption>::value,
                                    Mutex> {
            using thread_option = ThreadOption;
        };

        template <typename Visitable, typename ThreadOption, typename Mutex = std::mutex>
        struct opt_enable_safe_visit
            : public enable_safe_visit<
                  Visitable, ccy::is_safe<ThreadOption>::value, Mutex> {
            using thread_option = ThreadOption;
        };
    }

    template <typename Visitor, typename SafeVisitable>
    void safe_visit(Visitor vis, SafeVisitable& svis) {
        svis.apply(vis);
    }

} // namespace o
