#pragma once
#include "stl/string.h"
#include "stl/vector.h"
#include "stl/shared_ptr.h"
#include "stl/function.h"
#include "stl/stack.h"
#include "stl/pair.h"
#include "stl/tuple.h"
#include "stl/unordered_map.h"
#include "stl/algorithm.h"
#include "stl/initializer_list.h"

// Compile- and run-time exercise of the mini-STL surface the Ozark base needs.
// Returns true only if every check passes; module_start reports the result.
namespace stl_smoke {
    inline bool run() {
        stl::string s("insu");
        s += "lin";
        if (s != "insulin") return false;
        if (s[0] != 'i') return false;
        if (s.substr(4).compare("lin") != 0) return false;
        if (s.find("lin") != 4) return false;

        stl::vector<int> v;
        for (int i = 0; i < 10; ++i) v.push_back(i);
        int sum = 0;
        for (int x : v) sum += x;
        if (sum != 45) return false;

        stl::shared_ptr<int> p = stl::make_shared<int>(7);
        { stl::shared_ptr<int> q = p; if (*q != 7) return false; }
        if (!p || *p != 7) return false;

        stl::function<int(int)> f = [](int x) { return x * 2; };
        if (f(21) != 42) return false;

        stl::stack<int> st;
        st.push(1); st.push(2);
        if (st.top() != 2) return false;
        st.pop();
        if (st.top() != 1) return false;

        stl::unordered_map<stl::string, int> m;
        m["a"] = 1; m["b"] = 2;
        if (m["a"] != 1) return false;
        if (m.find("z") != m.end()) return false;
        if (m.find("b") == m.end()) return false;
        int msum = 0;
        for (auto& e : m) msum += e.second;
        if (msum != 3) return false;

        stl::vector<stl::tuple3<stl::string, int, bool>> instr;
        instr.push_back(stl::make_tuple(stl::string("x"), 5, true));
        if (instr.size() != 1 || instr[0].b != 5) return false;

        return true;
    }
}
