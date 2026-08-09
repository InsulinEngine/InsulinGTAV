#pragma once

namespace stl {
    template <typename It, typename Pred>
    It find_if(It first, It last, Pred p) {
        for (; first != last; ++first)
            if (p(*first)) return first;
        return last;
    }

    template <typename It, typename Pred>
    int count_if(It first, It last, Pred p) {
        int n = 0;
        for (; first != last; ++first)
            if (p(*first)) ++n;
        return n;
    }

    template <typename It>
    It begin(It c) { return c.begin(); }

    // Simple insertion sort (stable, fine for menu-sized ranges).
    template <typename It, typename Less>
    void sort(It first, It last, Less less) {
        for (It i = first; i != last; ++i) {
            for (It j = i; j != first; --j) {
                It prev = j; --prev;
                if (less(*j, *prev)) {
                    auto tmp = *j; *j = *prev; *prev = tmp;
                } else break;
            }
        }
    }
}
