#pragma once

namespace stl {
    // Decay helper for template parameter deduction
    // Handles: array->pointer, remove references
    template <typename T>
    struct decay { typedef T type; };

    // Remove references (applied after array decay)
    template <typename T>
    struct decay<T&> { typedef typename decay<T>::type type; };
    template <typename T>
    struct decay<T&&> { typedef typename decay<T>::type type; };

    // Array to pointer decay
    template <typename T>
    struct decay<T[]> { typedef T* type; };
    template <typename T, unsigned N>
    struct decay<T[N]> { typedef T* type; };

    template <typename T> using decay_t = typename decay<T>::type;

    template <typename A, typename B>
    struct pair {
        A first{};
        B second{};
        pair() {}
        pair(const A& a, const B& b) : first(a), second(b) {}
    };

    template <typename A, typename B>
    pair<decay_t<A>, decay_t<B>> make_pair(A&& a, B&& b) {
        return pair<decay_t<A>, decay_t<B>>(a, b);
    }
}
