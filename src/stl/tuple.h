#pragma once

// Ozark's base uses tuple only as a fixed 3-field bundle
// (vector<tuple<string,int,bool>> for per-option instructionals). A concrete
// 3-field struct covers that without a full variadic tuple.
namespace stl {
    template <typename A, typename B, typename C>
    struct tuple3 {
        A a{};
        B b{};
        C c{};
        tuple3() {}
        tuple3(const A& a_, const B& b_, const C& c_) : a(a_), b(b_), c(c_) {}
    };

    template <typename A, typename B, typename C>
    tuple3<A, B, C> make_tuple(const A& a, const B& b, const C& c) {
        return tuple3<A, B, C>(a, b, c);
    }
}
