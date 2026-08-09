#pragma once
#include "platform/stdafx.h"
#include "global/ui_vars.h"
#include "rage/types/base_types.h"

// Generic side-panel system (the panel framework itself). Ozark's built-in
// panels (player info, friends, sessions, ...) are network features and are NOT
// ported; register your own panel_child with an m_update render callback.
namespace menu::panels {
    struct panel_child;

    struct panel_parent {
        bool m_render;                                         // render *any* children
        stl::string m_id;
        stl::string m_name;
        stl::unordered_map<int, math::vector2<float>> m_column_offset; // by column
        stl::vector<panel_child> m_children_panels;
    };

    struct panel_child {
        panel_parent* m_parent;
        bool m_render;
        stl::string m_id;
        stl::string m_name;

        bool m_double_sided = true;
        int m_index = 0;                  // render order
        int m_column = 0;                 // 0 = next to menu, 1 = next to that
        int m_panel_tick_left = 0;
        int m_panel_tick_right = 0;
        int m_panel_option_count_left = 0;
        int m_panel_option_count_right = 0;

        uint8_t m_custom_ptr[0x150];      // struct storage for passing data
        math::vector2<float>(*m_update)(panel_child&); // render callback -> total height
    };

    class panel {
    public:
        panel(panel_child& child, color_rgba header_color);

        void item(stl::string name, stl::string value, int font = 0, float font_scale = 0.3f, color_rgba color = { 255, 255, 255, 255 });
        void item(stl::string name, stl::string sprite_left, stl::string sprite_right, int font = 0, float font_scale = 0.3f, color_rgba color = { 255, 255, 255, 255 }, math::vector2<float> scale = { 0.0192f, 0.0336f });
        void item_full(stl::string name, stl::string value, int font = 0, float font_scale = 0.3f, color_rgba color = { 255, 255, 255, 255 });
        void item_full(stl::string name, stl::string sprite_left, stl::string sprite_right, int font = 0, float font_scale = 0.3f, color_rgba color = { 255, 255, 255, 255 }, math::vector2<float> scale = { 0.0192f, 0.0336f });

        math::vector2<float> get_rendering_position();

        math::vector2<float> get_render_scale() {
            if (m_child) {
                if (m_child->m_panel_tick_left == 0 && m_child->m_panel_tick_right == 0) {
                    return { 0.f, 0.f };
                }
            }
            return { m_width, m_height };
        }

        math::vector2<float> get_column_adjustment() { return m_column_adjustment; }
        panel_child* get_panel_child() { return m_child; }
        float get_column_offset() { return m_column_offset; }
    private:
        void panel_left_item(stl::string name, stl::string value, int font, float font_scale, color_rgba color);
        void panel_right_item(stl::string name, stl::string value, int font, float font_scale, color_rgba color);
        void panel_left_sprite_item(stl::string name, stl::string sprite_left, stl::string sprite_right, int font, float font_scale, color_rgba color = { 255, 255, 255, 255 }, math::vector2<float> scale = { 0.0192f, 0.0336f });
        void panel_right_sprite_item(stl::string name, stl::string sprite_left, stl::string sprite_right, int font, float font_scale, color_rgba color = { 255, 255, 255, 255 }, math::vector2<float> scale = { 0.0192f, 0.0336f });

        panel_child* m_child;
        float m_width = 0.f;
        float m_height = 0.f;
        float m_column_offset = 0.f;
        math::vector2<float> m_column_adjustment;
    };

    class panel_manager {
    public:
        void load();
        void update();
        void cleanup();

        void null_structure(panel_child& _this);
        void set_structure(panel_child& _this, void* data, int size);

        panel_parent* get_parent(stl::string id);
        panel_child& get_child(panel_parent* parent, stl::string id);
        void rearrange(panel_parent* parent, stl::string id, int new_column, int new_position);
        void toggle_panel_render(stl::string parent_id, bool toggle);

        stl::vector<panel_parent*>& get_panels() { return m_panels; }
    private:
        stl::vector<panel_parent*> m_panels;
        bool m_rearranging = false;
    };

    panel_manager* get_panel_manager();

    inline void load() { get_panel_manager()->load(); }
    inline void update() { get_panel_manager()->update(); }
    inline void cleanup() { get_panel_manager()->cleanup(); }
    inline void null_structure(panel_child& _this) { get_panel_manager()->null_structure(_this); }
    inline void set_structure(panel_child& _this, void* data, int size) { get_panel_manager()->set_structure(_this, data, size); }
    inline panel_parent* get_parent(stl::string id) { return get_panel_manager()->get_parent(id); }
    inline panel_child& get_child(panel_parent* parent, stl::string id) { return get_panel_manager()->get_child(parent, id); }
    inline void rearrange(panel_parent* parent, stl::string id, int new_column, int new_position) { get_panel_manager()->rearrange(parent, id, new_column, new_position); }
    inline void toggle_panel_render(stl::string parent_id, bool toggle) { get_panel_manager()->toggle_panel_render(parent_id, toggle); }
    inline stl::vector<panel_parent*>& get_panels() { return get_panel_manager()->get_panels(); }
}
