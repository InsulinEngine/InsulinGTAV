#pragma once
#include "stl/vector.h"

namespace stl {
    template <typename T> class stack {
    public:
        void push(const T& v) { m_data.push_back(v); }
        void pop() { if (!m_data.empty()) m_data.resize(m_data.size() - 1); }
        T& top() { return m_data.at(m_data.size() - 1); }
        const T& top() const { return m_data.at(m_data.size() - 1); }
        size_t size() const { return m_data.size(); }
        bool empty() const { return m_data.empty(); }
    private:
        vector<T> m_data;
    };
}
