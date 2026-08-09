#pragma once

namespace stl {
    template <typename It, typename Pred>
    It find_if(It first, It last, Pred p) {
        for (; first != last; ++first)
            if (p(*first)) return first;
        return last;
    }

    template <typename It>
    It begin(It c) { return c.begin(); }
}
