#pragma once
#include "stl/vector.h"
#include "stl/pair.h"

// Small vector-backed associative map. Menu-sized maps (dozens of entries),
// so linear lookup is fine and there is no hashing/allocation overhead beyond
// the backing vector. Iterators are pointers into the backing vector, so both
// Ozark idioms work: `map.find(k) != map.end()` and `for (auto& e : map)`.
// (As with std, a mutating op like operator[] insert invalidates iterators.)
namespace stl {
    template <typename K, typename V>
    class unordered_map {
    public:
        typedef pair<K, V> value_type;

        V& operator[](const K& k) {
            for (size_t i = 0; i < m_data.size(); ++i)
                if (m_data[i].first == k) return m_data[i].second;
            m_data.push_back(value_type(k, V()));
            return m_data[m_data.size() - 1].second;
        }

        value_type* begin() { return m_data.begin(); }
        value_type* end() { return m_data.end(); }
        const value_type* begin() const { return m_data.begin(); }
        const value_type* end() const { return m_data.end(); }

        value_type* find(const K& k) {
            for (value_type* it = m_data.begin(); it != m_data.end(); ++it)
                if (it->first == k) return it;
            return m_data.end();
        }

        bool contains(const K& k) { return find(k) != end(); }

        void erase(const K& k) {
            for (size_t i = 0; i < m_data.size(); ++i) {
                if (m_data[i].first == k) {
                    for (size_t j = i; j + 1 < m_data.size(); ++j)
                        m_data[j] = m_data[j + 1];
                    m_data.resize(m_data.size() - 1);
                    return;
                }
            }
        }

        size_t size() const { return m_data.size(); }
        bool empty() const { return m_data.empty(); }
        void clear() { m_data.clear(); }
        vector<value_type>& raw() { return m_data; }

    private:
        vector<value_type> m_data;
    };
}
