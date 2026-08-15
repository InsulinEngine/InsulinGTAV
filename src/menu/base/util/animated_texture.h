#pragma once
#include "platform/stdafx.h"
#include "stl/string.h"
#include "stl/vector.h"
#include "stl/pair.h"
#include <stdint.h>

// A frame sequence living in one custom texture dictionary, played by handing
// the renderer a different texture name each tick.
//
// This type knows nothing about files or image formats: it holds names and
// delays. That is the seam a future runtime GIF decoder plugs into -- it would
// call add_frame() with textures it created and nothing here would change.
namespace menu {
    class animated_texture {
    public:
        animated_texture() : m_elapsed_ms(0), m_loop(true) {}

        void set_dictionary(const char* dict) { m_dict = dict ? dict : ""; }
        void set_loop(bool loop)              { m_loop = loop; }

        // delay_ms <= 0 is replaced by the 66 ms default (GIFs commonly store 0).
        void add_frame(const char* texture_name, int delay_ms);

        // dt in seconds -- pass global::ui::g_delta. Frames are never stepped one
        // at a time: the accumulator is absolute, so a dropped frame catches up
        // instead of putting the animation into slow motion.
        void advance(float dt_seconds);
        void reset() { m_elapsed_ms = 0; }

        // Drop all frames and rewind. Used when an animation is reloaded.
        void clear();

        // {dictionary, texture name} of the current frame, ready for draw_sprite.
        // Both strings are empty when the animation holds no frames.
        stl::pair<stl::string, stl::string> current() const;
        int  current_index() const;

        int  frame_count() const { return (int)m_names.size(); }
        bool ready() const       { return m_names.size() > 0; }

    private:
        stl::string              m_dict;
        stl::vector<stl::string> m_names;
        stl::vector<uint16_t>    m_delays;
        int                      m_elapsed_ms;
        bool                     m_loop;
    };

    // The registry of loaded animations. Registration is a function-local static
    // (no .init_array on GoldHEN); animations live for the session.
    namespace animation {
        // Default frame delay in ms when a manifest omits or zeroes one.
        static const int k_default_delay_ms = 66;
        // Frames beyond this are refused, with a log line -- never silently.
        static const int k_max_frames = 16;

        // The animation registered under `name`, or nullptr.
        animated_texture* get(const char* name);

        // Register (or replace) an empty animation under `name` and return it.
        animated_texture* create(const char* name);

        // Load frames from `dir` into the shared "insulin" dictionary and register
        // them under `name`. Reads <dir>/frames.json when present; otherwise scans
        // the directory for *.png / *.dds in name order at the default delay.
        // Textures are added as "<name>_000", "<name>_001", ... -- explicit names,
        // because two animations whose files both start at 000 would collide in the
        // dictionary and commit() drops colliding codes.
        // Returns nullptr if the directory is missing, empty, or nothing loaded.
        animated_texture* load_from_dir(const char* name, const char* dir);

        // load_from_dir("banner", "/data/Ozark/anim/banner").
        animated_texture* load_banner();

        // Advance every registered animation. Called once per tick.
        void update(float dt_seconds);

        // Banner frame if the animation named "banner" is loaded and ready,
        // otherwise the static {"insulin", "logo"} pair.
        stl::pair<stl::string, stl::string> header_asset();
    }
}
