#pragma once
#include <stddef.h>
#include <string.h>
#include "stl/new.h"

#ifndef STL_FUNCTION_CAP
#define STL_FUNCTION_CAP 64
#endif

namespace stl {
    template <typename Sig> class function;

    template <typename R, typename... Args>
    class function<R(Args...)> {
    public:
        function() {}

        template <typename F>
        function(F f) {
            static_assert(sizeof(F) <= STL_FUNCTION_CAP,
                          "stl::function capture too large; raise STL_FUNCTION_CAP");
            new (m_buf) F(f);
            m_call = &call_impl<F>;
            m_copy = &copy_impl<F>;
            m_dtor = &dtor_impl<F>;
        }

        function(const function& o) { copy_from(o); }
        function& operator=(const function& o) {
            if (this != &o) { reset(); copy_from(o); } return *this;
        }
        ~function() { reset(); }

        explicit operator bool() const { return m_call != nullptr; }

        R operator()(Args... args) const {
            return m_call(const_cast<void*>((const void*)m_buf), args...);
        }

    private:
        template <typename F>
        static R call_impl(void* buf, Args... args) { return (*(F*)buf)(args...); }
        template <typename F>
        static void copy_impl(void* dst, const void* src) { new (dst) F(*(const F*)src); }
        template <typename F>
        static void dtor_impl(void* buf) { ((F*)buf)->~F(); }

        void reset() {
            if (m_dtor) m_dtor(m_buf);
            m_call = nullptr; m_copy = nullptr; m_dtor = nullptr;
        }
        void copy_from(const function& o) {
            m_call = o.m_call; m_copy = o.m_copy; m_dtor = o.m_dtor;
            if (m_copy) m_copy(m_buf, o.m_buf);
        }

        alignas(16) char m_buf[STL_FUNCTION_CAP];
        R (*m_call)(void*, Args...) = nullptr;
        void (*m_copy)(void*, const void*) = nullptr;
        void (*m_dtor)(void*) = nullptr;
    };
}
