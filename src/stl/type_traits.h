#pragma once

namespace stl {
    template <typename A, typename B> struct is_same { static const bool value = false; };
    template <typename A> struct is_same<A, A> { static const bool value = true; };
}
