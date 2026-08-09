#include "panels.h"
#include "menu/base/renderer.h"
#include "global/ui_vars.h"
#include "global/vars.h"
#include "menu/base/base.h"
#include "rage/invoker/natives.h"

// Only the generic panel framework is ported; Ozark's built-in render_panel_*
// functions and their network load() are feature code and are omitted.
namespace menu::panels {
    math::vector2<float> panel::get_rendering_position() {
        math::vector2<float> _return;

        if (m_child->m_column == 0) {
            if (global::ui::g_position.x >= 0.40f) {
                _return.x = global::ui::g_position.x - 0.283f;
                if (!m_child->m_double_sided) _return.x += 0.14f;
            } else {
                _return.x = global::ui::g_position.x + global::ui::g_scale.x + 0.003f;
            }
        } else {
            float calculated_offset = 0.f;

            for (int i = 0; i < m_child->m_column; i++) {
                if (m_child->m_parent->m_column_offset.find(i) != m_child->m_parent->m_column_offset.end()) {
                    calculated_offset += m_child->m_parent->m_column_offset[i].x;
                }
            }

            if (global::ui::g_position.x >= 0.40f) {
                _return.x = global::ui::g_position.x - 0.283f;
                _return.x -= calculated_offset;

                if (m_child->m_double_sided && m_child->m_parent->m_column_offset[m_child->m_column - 1].x == 0.14f) {
                    _return.x -= 0.14f;
                }
                if (!m_child->m_double_sided) _return.x += 0.14f;
            } else {
                _return.x = global::ui::g_position.x + global::ui::g_scale.x + 0.003f;
                _return.x += calculated_offset;
            }
        }

        if (m_child->m_column != 0) {
            if (global::ui::g_position.x >= 0.40f) _return.x -= (m_child->m_column * 0.002f);
            else _return.x += (m_child->m_column * 0.002f);
        }

        return _return;
    }

    panel::panel(panel_child& child, color_rgba header_color) {
        m_child = &child;

        int max_count = child.m_panel_option_count_left;
        if (child.m_panel_option_count_right > max_count) max_count = child.m_panel_option_count_right;

        child.m_panel_option_count_left = child.m_panel_tick_left - 1;
        child.m_panel_option_count_right = child.m_panel_option_count_right - 1;

        if (m_child->m_parent->m_column_offset.find(child.m_column) == m_child->m_parent->m_column_offset.end()) {
            m_column_offset = 0.f;
        } else {
            m_column_offset = (m_child->m_parent->m_column_offset[child.m_column].y);
        }

        m_column_adjustment = get_rendering_position();

        float x = m_column_adjustment.x;
        float width = child.m_double_sided ? 0.28f : 0.14f;

        menu::renderer::draw_rect({ x, (global::ui::g_position.y - 0.08f) + m_column_offset }, { width, (0.007875f / 2.f) }, header_color);

        stl::pair<stl::string, stl::string> texture = menu::renderer::get_texture(global::ui::m_panel_background);
        menu::renderer::draw_sprite_aligned(texture, { x, (global::ui::g_position.y - 0.08f) + (0.007875f / 2.f) + m_column_offset }, { width, (0.03f * max_count) }, 0.f, global::ui::g_panel_background);

        m_width = width;
        m_height = (0.03f * max_count) + (0.007875f / 2.f) + 0.004f;

        child.m_panel_tick_left = 1;
        child.m_panel_tick_right = 1;
    }

    void panel::item(stl::string name, stl::string value, int font, float font_scale, color_rgba color) {
        if (!m_child->m_double_sided) { panel_left_item(name, value, font, font_scale, color); return; }
        if (m_child->m_panel_tick_left > m_child->m_panel_tick_right) panel_right_item(name, value, font, font_scale, color);
        else panel_left_item(name, value, font, font_scale, color);
    }

    void panel::item(stl::string name, stl::string sprite_left, stl::string sprite_right, int font, float font_scale, color_rgba color, math::vector2<float> scale) {
        if (!m_child->m_double_sided) { panel_left_sprite_item(name, sprite_left, sprite_right, font, font_scale, color, scale); return; }
        if (m_child->m_panel_tick_left > m_child->m_panel_tick_right) panel_right_sprite_item(name, sprite_left, sprite_right, font, font_scale, color, scale);
        else panel_left_sprite_item(name, sprite_left, sprite_right, font, font_scale, color, scale);
    }

    void panel::item_full(stl::string name, stl::string value, int font, float font_scale, color_rgba color) {
        if (font == 0 && global::ui::g_panel_font != 0) { font = global::ui::g_panel_font; font_scale = menu::renderer::get_normalized_font_scale(font, font_scale); }

        if (m_child->m_panel_tick_left != m_child->m_panel_tick_right) {
            if (m_child->m_panel_tick_left > m_child->m_panel_tick_right) m_child->m_panel_tick_right = m_child->m_panel_tick_left;
            else m_child->m_panel_tick_left = m_child->m_panel_tick_right;
        }

        float adjustment = m_column_adjustment.x;
        float wrap = (adjustment + 0.28f - 0.004f);
        float x = adjustment + 0.003f;

        menu::renderer::draw_text(name, { x, (global::ui::g_position.y - 0.08f) + ((m_child->m_panel_tick_left - 1) * 0.03f) + 0.006f + ((0.007875f / 2.f) - 0.0017875f) + m_column_offset }, font_scale, font, color);
        menu::renderer::draw_text(value, { 0.f, (global::ui::g_position.y - 0.08f) + ((m_child->m_panel_tick_left - 1) * 0.03f) + 0.006f + ((0.007875f / 2.f) - 0.0017875f) + m_column_offset }, font_scale, font, color, JUSTIFY_RIGHT, { 0.f, wrap });

        m_child->m_panel_tick_left++;
        m_child->m_panel_tick_right++;
    }

    void panel::item_full(stl::string name, stl::string sprite_left, stl::string sprite_right, int font, float font_scale, color_rgba color, math::vector2<float> scale) {
        if (font == 0 && global::ui::g_panel_font != 0) { font = global::ui::g_panel_font; font_scale = menu::renderer::get_normalized_font_scale(font, font_scale); }

        if (m_child->m_panel_tick_left != m_child->m_panel_tick_right) {
            if (m_child->m_panel_tick_left > m_child->m_panel_tick_right) m_child->m_panel_tick_right = m_child->m_panel_tick_left;
            else m_child->m_panel_tick_left = m_child->m_panel_tick_right;
        }

        float adjustment = m_column_adjustment.x;
        float x = adjustment + 0.003f;

        menu::renderer::draw_sprite({ sprite_left, sprite_right }, { x + 0.006f, (global::ui::g_position.y - 0.08f) + 0.017f + ((m_child->m_panel_tick_left - 1) * 0.03f) + ((0.007875f / 2.f) - 0.0017875f) + m_column_offset }, scale, 0.f, color);
        menu::renderer::draw_text(name, { x + 0.014f, (global::ui::g_position.y - 0.08f) + ((m_child->m_panel_tick_left - 1) * 0.03f) + 0.006f + ((0.007875f / 2.f) - 0.0017875f) + m_column_offset }, font_scale, font, color);

        m_child->m_panel_tick_left++;
        m_child->m_panel_tick_right++;
    }

    void panel::panel_left_item(stl::string name, stl::string value, int font, float font_scale, color_rgba color) {
        if (font == 0 && global::ui::g_panel_font != 0) { font = global::ui::g_panel_font; font_scale = menu::renderer::get_normalized_font_scale(font, font_scale); }

        float x = m_column_adjustment.x;
        float x2 = x + 0.003f;
        float wrap = (x + 0.14f - 0.004f);

        menu::renderer::draw_text(name, { x2, (global::ui::g_position.y - 0.08f) + ((m_child->m_panel_tick_left - 1) * 0.03f) + 0.006f + ((0.007875f / 2.f) - 0.0017875f) + m_column_offset }, font_scale, font, color);
        menu::renderer::draw_text(value, { 0.f, (global::ui::g_position.y - 0.08f) + ((m_child->m_panel_tick_left - 1) * 0.03f) + 0.006f + ((0.007875f / 2.f) - 0.0017875f) + m_column_offset }, font_scale, font, color, JUSTIFY_RIGHT, { 0.f, wrap });

        m_child->m_panel_tick_left++;
    }

    void panel::panel_right_item(stl::string name, stl::string value, int font, float font_scale, color_rgba color) {
        if (font == 0 && global::ui::g_panel_font != 0) { font = global::ui::g_panel_font; font_scale = menu::renderer::get_normalized_font_scale(font, font_scale); }

        float x = m_column_adjustment.x;
        float wrap = (x + 0.28f - 0.004f);

        menu::renderer::draw_rect({ x + 0.14f, (global::ui::g_position.y - 0.08f) + ((m_child->m_panel_tick_right - 1) * 0.03f) + (0.007875f / 2.f) + m_column_offset + 0.00525f }, { 0.001f, 0.021f }, { 255, 255, 255, 255 });
        menu::renderer::draw_text(name, { x + 0.14f + 0.004f, (global::ui::g_position.y - 0.08f) + ((m_child->m_panel_tick_right - 1) * 0.03f) + 0.006f + ((0.007875f / 2.f) - 0.0017875f) + m_column_offset }, font_scale, font, color);
        menu::renderer::draw_text(value, { 0.f, (global::ui::g_position.y - 0.08f) + ((m_child->m_panel_tick_right - 1) * 0.03f) + 0.006f + ((0.007875f / 2.f) - 0.0017875f) + m_column_offset }, font_scale, font, color, JUSTIFY_RIGHT, { 0.f, wrap });

        m_child->m_panel_tick_right++;
    }

    void panel::panel_left_sprite_item(stl::string name, stl::string sprite_left, stl::string sprite_right, int font, float font_scale, color_rgba color, math::vector2<float> scale) {
        if (font == 0 && global::ui::g_panel_font != 0) { font = global::ui::g_panel_font; font_scale = menu::renderer::get_normalized_font_scale(font, font_scale); }

        float x = m_column_adjustment.x;
        float x2 = x + 0.003f;

        if (m_child->m_double_sided) {
            menu::renderer::draw_rect({ x + 0.14f, (global::ui::g_position.y - 0.08f) + ((m_child->m_panel_tick_left - 1) * 0.03f) + (0.007875f / 2.f) + m_column_offset + 0.00525f }, { 0.001f, 0.021f }, { 255, 255, 255, 255 });
        }

        menu::renderer::draw_sprite({ sprite_left, sprite_right }, { x2 + 0.006f, (global::ui::g_position.y - 0.08f) + 0.017f + ((m_child->m_panel_tick_left - 1) * 0.03f) + ((0.007875f / 2.f) - 0.0017875f) + m_column_offset }, scale, 0.f, color);
        menu::renderer::draw_text(name, { x2 + 0.014f, (global::ui::g_position.y - 0.08f) + ((m_child->m_panel_tick_left - 1) * 0.03f) + 0.006f + ((0.007875f / 2.f) - 0.0017875f) + m_column_offset }, font_scale, font, color);

        m_child->m_panel_tick_left++;
    }

    void panel::panel_right_sprite_item(stl::string name, stl::string sprite_left, stl::string sprite_right, int font, float font_scale, color_rgba color, math::vector2<float> scale) {
        if (font == 0 && global::ui::g_panel_font != 0) { font = global::ui::g_panel_font; font_scale = menu::renderer::get_normalized_font_scale(font, font_scale); }

        float x = m_column_adjustment.x;
        float x2 = x + 0.14f + 0.003f;

        menu::renderer::draw_sprite({ sprite_left, sprite_right }, { x2 + 0.009f, (global::ui::g_position.y - 0.08f) + 0.017f + ((m_child->m_panel_tick_right - 1) * 0.03f) + ((0.007875f / 2.f) - 0.0017875f) + m_column_offset }, scale, 0.f, color);
        menu::renderer::draw_text(name, { x + 0.14f + 0.004f + 0.018f, (global::ui::g_position.y - 0.08f) + ((m_child->m_panel_tick_right - 1) * 0.03f) + 0.006f + ((0.007875f / 2.f) - 0.0017875f) + m_column_offset }, font_scale, font, color);

        m_child->m_panel_tick_right++;
    }

    void panel_manager::update() {
        if (global::vars::g_unloading || m_rearranging || !menu::base::is_open() || global::ui::g_input_open) return;

        for (panel_parent* panel : m_panels) {
            if (global::vars::g_unloading || m_rearranging) return;
            for (auto& e : panel->m_column_offset) e.second = math::vector2<float>(0.f, 0.f);

            if (panel->m_render) {
                if (!panel->m_children_panels.empty()) {
                    for (int j = 0; j < 20; j++) {
                        for (size_t i = 0; i < panel->m_children_panels.size(); i++) {
                            if (global::vars::g_unloading || m_rearranging) return;

                            panel_child& child = panel->m_children_panels[i];
                            if (child.m_column == j && child.m_render && child.m_update) {
                                math::vector2<float> scale = child.m_update(child);

                                if (panel->m_column_offset[child.m_column].x < scale.x) panel->m_column_offset[child.m_column].x = scale.x;
                                panel->m_column_offset[child.m_column].y += scale.y;
                            }
                        }
                    }
                }
            }
        }
    }

    void panel_manager::cleanup() {
        for (panel_parent* panel : m_panels) {
            if (panel) delete panel;
        }
    }

    panel_parent* panel_manager::get_parent(stl::string id) {
        panel_parent** search = stl::find_if(m_panels.begin(), m_panels.end(), [=](panel_parent* element) { return element->m_id == id; });
        if (search != m_panels.end()) return *search;
        return nullptr;
    }

    panel_child& panel_manager::get_child(panel_parent* parent, stl::string id) {
        static panel_child _static;
        if (!parent) return _static;

        panel_child* search = stl::find_if(parent->m_children_panels.begin(), parent->m_children_panels.end(), [=](panel_child& element) { return element.m_id == id; });
        if (search != parent->m_children_panels.end()) return *search;
        return _static;
    }

    void panel_manager::toggle_panel_render(stl::string parent_id, bool toggle) {
        panel_parent* parent = get_parent(parent_id);
        if (parent) parent->m_render = toggle;
    }

    void panel_manager::rearrange(panel_parent* parent, stl::string id, int column, int position) {
        if (!parent) return;
        m_rearranging = true;

        panel_child& child = get_child(parent, id);
        if (child.m_index != position) child.m_index = position;

        if (column != child.m_column) {
            int index = 0;
            for (panel_child& elem : parent->m_children_panels) {
                if (elem.m_column == column && elem.m_index > index) index = elem.m_index;
            }
            child.m_index = index + 1;
            child.m_column = column;
        }

        stl::sort(parent->m_children_panels.begin(), parent->m_children_panels.end(), [=](panel_child& left, panel_child& right) { return left.m_index < right.m_index; });
        m_rearranging = false;
    }

    void panel_manager::null_structure(panel_child& _this) {
        memset(_this.m_custom_ptr, 0, sizeof(_this.m_custom_ptr));
    }

    void panel_manager::set_structure(panel_child& _this, void* data, int size) {
        memcpy(_this.m_custom_ptr, data, size);
    }

    // No built-in panels (those are network features); register your own.
    void panel_manager::load() {}

    panel_manager* get_panel_manager() {
        static panel_manager instance;
        return &instance;
    }
}
