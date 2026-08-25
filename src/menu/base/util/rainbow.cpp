#include "menu/base/util/rainbow.h"
#include "menu/base/util/rainbow_math.h"

namespace menu {
    void rainbow::configure(int min, int max, int steps) {
        m_min = min; m_max = max; m_steps = steps < 1 ? 1 : steps;
    }

    bool rainbow::contains(const color_rgba* c) const {
        for (int i = 0; i < (int)m_colors.size(); i++)
            if (m_colors[i] == c) return true;
        return false;
    }

    void rainbow::add(color_rgba* c) {
        if (!c || contains(c)) return;
        m_colors.push_back(c);
        m_originals.push_back(*c);
    }

    void rainbow::remove(color_rgba* c) {
        for (int i = 0; i < (int)m_colors.size(); i++) {
            if (m_colors[i] != c) continue;
            *m_colors[i] = m_originals[i];
            m_colors.erase(m_colors.begin() + i);
            m_originals.erase(m_originals.begin() + i);
            return;
        }
    }

    void rainbow::run() {
        if (!m_enabled || m_colors.empty()) return;

        rainbow_math::rgb c = rainbow_math::color_at(m_step, m_steps, m_min, m_max);
        m_step = (m_step + 1) % m_steps;

        for (int i = 0; i < (int)m_colors.size(); i++) {
            m_colors[i]->r = c.r;
            m_colors[i]->g = c.g;
            m_colors[i]->b = c.b;
            // Alpha is never touched: cycling opacity makes the menu flicker
            // transparent.
        }
    }

    void rainbow::stop() {
        for (int i = 0; i < (int)m_colors.size(); i++)
            *m_colors[i] = m_originals[i];
        m_colors.clear();
        m_originals.clear();
        m_step = 0;
        m_enabled = false;
    }

    rainbow* get_rainbow() {
        static rainbow instance;   // function-local: no .init_array in this plugin
        return &instance;
    }
}
