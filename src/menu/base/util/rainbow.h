#pragma once
#include "platform/stdafx.h"
#include "global/ui_vars.h"

namespace menu {
    // Animates registered colours through the hue cycle. Holds each colour's
    // pre-rainbow value so stop() restores exactly what was there rather than a
    // guess at what it should have been.
    class rainbow {
    public:
        void configure(int min, int max, int steps);

        void add(color_rgba* c);          // remembers the current value
        void remove(color_rgba* c);       // restores it
        bool contains(const color_rgba* c) const;

        void run();                       // one step; called per frame
        void stop();                      // restore every registered colour

        bool m_enabled = false;
        int  m_min   = 25;
        int  m_max   = 250;
        int  m_steps = 80;
    private:
        int m_step = 0;
        stl::vector<color_rgba*> m_colors;
        stl::vector<color_rgba>  m_originals;   // parallel to m_colors
    };

    rainbow* get_rainbow();
}
