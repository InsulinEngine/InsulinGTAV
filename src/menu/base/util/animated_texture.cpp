#include "menu/base/util/animated_texture.h"
#include "menu/base/util/frame_clock.h"
#include "platform/log.h"

namespace menu {
    void animated_texture::add_frame(const char* texture_name, int delay_ms) {
        if (!texture_name || !texture_name[0]) return;
        if ((int)m_names.size() >= animation::k_max_frames) {
            LOG_WARN("anim: \"%s\" already holds %d frames, refusing \"%s\"",
                     m_dict.c_str(), animation::k_max_frames, texture_name);
            return;
        }
        if (delay_ms <= 0) delay_ms = animation::k_default_delay_ms;
        if (delay_ms > 65535) delay_ms = 65535;          // uint16_t storage
        m_names.push_back(stl::string(texture_name));
        m_delays.push_back((uint16_t)delay_ms);
    }

    void animated_texture::clear() {
        m_names.clear();
        m_delays.clear();
        m_elapsed_ms = 0;
    }

    void animated_texture::advance(float dt_seconds) {
        if (m_names.size() == 0 || dt_seconds <= 0.f) return;
        m_elapsed_ms += (int)(dt_seconds * 1000.f + 0.5f);

        // Bound the accumulator either way, so it cannot overflow across a long
        // session (signed overflow is UB, and int milliseconds run out after ~24
        // days of uptime). Looping wraps; one-shot saturates at the end, which
        // frame_clock already reads as "stay on the last frame".
        const int total = menu::frame_clock::total_ms(&m_delays[0], (int)m_delays.size());
        if (total > 0) {
            if (m_loop)                     m_elapsed_ms %= total;
            else if (m_elapsed_ms > total)  m_elapsed_ms = total;
        }
    }

    int animated_texture::current_index() const {
        if (m_names.size() == 0) return 0;
        return menu::frame_clock::frame_at(&m_delays[0], (int)m_delays.size(), m_elapsed_ms, m_loop);
    }

    stl::pair<stl::string, stl::string> animated_texture::current() const {
        if (m_names.size() == 0) return stl::make_pair(stl::string(""), stl::string(""));
        // Named rather than make_pair'd: stl::decay strips references but not cv,
        // so deducing from const members here would yield pair<const string,
        // const string>, which does not convert to the return type.
        return stl::pair<stl::string, stl::string>(m_dict, m_names[current_index()]);
    }

    namespace animation {
        struct slot { stl::string name; animated_texture anim; };

        static stl::vector<slot>& registry() {
            static stl::vector<slot> instance;   // function-local: no .init_array
            return instance;
        }

        animated_texture* get(const char* name) {
            if (!name) return nullptr;
            stl::vector<slot>& r = registry();
            for (size_t i = 0; i < r.size(); i++)
                if (r[i].name == name) return &r[i].anim;
            return nullptr;
        }

        animated_texture* create(const char* name) {
            if (!name || !name[0]) return nullptr;
            animated_texture* existing = get(name);
            if (existing) { existing->clear(); return existing; }

            slot s;
            s.name = name;
            registry().push_back(s);
            return &registry()[registry().size() - 1].anim;
        }

        void update(float dt_seconds) {
            stl::vector<slot>& r = registry();
            for (size_t i = 0; i < r.size(); i++) r[i].anim.advance(dt_seconds);
        }

        stl::pair<stl::string, stl::string> header_asset() {
            animated_texture* banner = get("banner");
            if (banner && banner->ready()) return banner->current();
            return stl::make_pair(stl::string("insulin"), stl::string("logo"));
        }
    }
}
