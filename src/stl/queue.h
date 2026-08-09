#pragma once
#include "stl/vector.h"

// FIFO queue over stl::vector (front = index 0). Fine for the small request
// queues the control manager uses.
namespace stl {
    template <typename T> class queue {
    public:
        void push(const T& v) { m_data.push_back(v); }
        void pop() { if (!m_data.empty()) m_data.erase(m_data.begin()); }
        T& front() { return m_data[0]; }
        const T& front() const { return m_data[0]; }
        size_t size() const { return m_data.size(); }
        bool empty() const { return m_data.empty(); }
    private:
        vector<T> m_data;
    };
}
