#pragma once
#include "platform/stdafx.h"
#include "global/ui_vars.h"

// Ozark's stacked notification system (top-left toasts with title + body,
// slide-in / fade-out animation). The network-only protection() helper is
// dropped (it depended on the network subsystem); stacked/stacked_lines/subtitle
// remain.
namespace menu::notify {
    struct notify_context {
        stl::vector<stl::string> m_text = {};
        stl::string m_rendering_text = "";
        stl::string m_title = "";
        int m_lines = 1;
        color_rgba m_color = {};
        float m_max_width = 0.20f;
        float x = 0.009f;
        float m_y = 0.0f;
        uint32_t m_start_time = 0;
        uint32_t m_time_limit = 5000;
        bool m_has_calculated = false;
        bool m_has_init = false;
        uint32_t m_alpha_start = 0;
        int m_alpha = 255;
        float m_title_width = 0.f;
    };

    class notify {
    public:
        void update();
        void stacked(stl::string title, stl::string text, color_rgba color = global::ui::g_notify_bar, uint32_t timeout = 6000);
        void stacked_lines(stl::string title, stl::vector<stl::string> text, color_rgba color = global::ui::g_notify_bar);
        void subtitle(const char* msg);

        stl::vector<notify_context>& get_contexts() { return m_context; }
    private:
        stl::vector<notify_context> m_context;
    };

    notify* get_notify();

    inline void update() { get_notify()->update(); }
    inline void subtitle(const char* msg) { get_notify()->subtitle(msg); }
    inline void stacked(stl::string title, stl::string text, color_rgba color = global::ui::g_notify_bar, uint32_t timeout = 6000) { get_notify()->stacked(title, text, color, timeout); }
    inline void stacked_lines(stl::string title, stl::vector<stl::string> text, color_rgba color = global::ui::g_notify_bar) { get_notify()->stacked_lines(title, text, color); }
    inline stl::vector<notify_context>& get_contexts() { return get_notify()->get_contexts(); }
}
