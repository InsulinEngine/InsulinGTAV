#pragma once
#include "platform/stdafx.h"

// Ozark's stacked on-screen key/value display (bottom-right info list, e.g.
// live stats). id-keyed rows that can be updated or hidden.
namespace menu::display {
    struct stacked_display_context {
        bool m_rendering = false;
        stl::string m_id;
        stl::string m_key;
        stl::string m_value;
        long long m_add_time = 0;
    };

    class stacked_display {
    public:
        void render();
        void update(stl::string id, stl::string key, stl::string value);
        void disable(stl::string id);
    private:
        stl::vector<stacked_display_context> m_messages;
    };

    stacked_display* get_stacked_display();

    inline void render() { get_stacked_display()->render(); }
    inline void update(stl::string id, stl::string key, stl::string value) { get_stacked_display()->update(id, key, value); }
    inline void disable(stl::string id) { get_stacked_display()->disable(id); }
}
