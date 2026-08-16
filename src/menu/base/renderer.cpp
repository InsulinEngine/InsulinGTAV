#include "renderer.h"
#include "base.h"
#include "submenu_handler.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "menu/base/util/textures.h"
#include "menu/base/util/fonts.h"
#include "menu/base/util/menu_input.h"
#include "menu/base/util/animated_texture.h"
#include "menu/base/submenus/main.h"
#include "menu/base/util/color_math.h"
#include "rage/gfx.h"
#include <math.h>

static localization t_tooltip_1("put me in coach", true, true);

namespace menu::renderer {
    stl::pair<stl::string, stl::string> renderer::get_texture(menu_texture texture) {
        if (texture.m_texture != "sa7anisafaggot") {
            if (texture.m_enabled) {
                stl::vector<menu::textures::texture_context>& list = menu::textures::get_list();

                auto vit = stl::find_if(list.begin(), list.end(), [=](menu::textures::texture_context& context) { return context.m_name == texture.m_texture; });
                if (vit != list.end()) {
                    return { "insulin", vit->m_name };
                }
            }
        }

        return { "randomha", "yesyesyesyes" };
    }

    void renderer::render() {
        int current_option = base::get_current_option();
        int max_options = base::get_max_options();
        int scroll_offset = base::get_scroll_offset();
        int total_options = menu::submenu::handler::get_total_options();

        m_render_count = math::clamp(current_option - scroll_offset > max_options ? max_options : (current_option - scroll_offset + 1), 0, max_options);

        int option_count = total_options > max_options ? max_options : total_options;
        int scroller_position = math::clamp(current_option - scroll_offset > max_options ? max_options : current_option - scroll_offset, 0, max_options);

        // main header -- once the custom banner is loaded, rage::gfx has injected
        // it into the txd store as "insulin"/"logo"; draw it at full colour.
        // Otherwise fall back to the sentinel / game header texture.
        stl::pair<stl::string, stl::string> texture = get_texture(global::ui::m_header);
        // Header source, in order: the "slot_header" animation's current frame,
        // the "banner" animation's current frame, the static custom logo, then
        // the game/sentinel texture below.
        menu::animated_texture* slot_anim = menu::animation::get("slot_header");
        menu::animated_texture* banner_anim = menu::animation::get("banner");
        if (slot_anim && slot_anim->ready()) {
            draw_sprite_aligned(menu::animation::slot_asset("slot_header", global::ui::m_header.m_texture), { global::ui::g_position.x, global::ui::g_position.y - 0.08f }, { global::ui::g_scale.x, 0.08f }, 0.f, { 255, 255, 255, 255 });
        } else if ((banner_anim && banner_anim->ready()) || rage::gfx::banner_ready()) {
            draw_sprite_aligned(menu::animation::header_asset(), { global::ui::g_position.x, global::ui::g_position.y - 0.08f }, { global::ui::g_scale.x, 0.08f }, 0.f, { 255, 255, 255, 255 });
        } else {
            // A picture applied but its animation not ready (e.g. still-only)
            // resolves here to {"insulin", <stem>}. g_main_header defaults to
            // opaque black, which would paint the user's picture as a black
            // rectangle - tint only the sentinel / game-texture paths, which are
            // meant to be tinted, and draw a custom-dictionary texture at full
            // colour.
            color_rgba header_color = rage::gfx::is_custom_dict(texture.first.c_str()) ? color_rgba(255, 255, 255, 255) : global::ui::g_main_header;
            draw_sprite_aligned(texture, { global::ui::g_position.x, global::ui::g_position.y - 0.08f }, { global::ui::g_scale.x, 0.08f }, 0.f, header_color);
        }

        // background
        texture = get_texture(global::ui::m_background);
        menu::animated_texture* bg_anim = menu::animation::get("slot_background");
        if (bg_anim && bg_anim->ready()) {
            draw_sprite_aligned(menu::animation::slot_asset("slot_background", global::ui::m_background.m_texture),
                                global::ui::g_position, { global::ui::g_scale.x, option_count * global::ui::g_option_scale },
                                0.f, { 255, 255, 255, 255 });
        } else {
            if (texture.first == "randomha") texture = { "commonmenu", "gradient_bgd" };
            // Same reasoning as the header fallback above: draw a custom
            // background picture at full colour instead of through g_background
            // (opaque black by default), and keep the tint for the sentinel's
            // gradient_bgd substitute and any game texture.
            color_rgba bg_color = rage::gfx::is_custom_dict(texture.first.c_str()) ? color_rgba(255, 255, 255, 255) : global::ui::g_background;
            draw_sprite_aligned(texture, global::ui::g_position, { global::ui::g_scale.x, option_count * global::ui::g_option_scale },
                                0.f, bg_color);
        }

        // scroller
        if (global::ui::g_scroll_lerp) {
            m_smooth_scroll = math::lerp(m_smooth_scroll, global::ui::g_position.y + (scroller_position * global::ui::g_option_scale), global::ui::g_delta * global::ui::g_scroll_lerp_speed);
        } else m_smooth_scroll = global::ui::g_position.y + (scroller_position * global::ui::g_option_scale);

        texture = get_texture(global::ui::m_scroller);
        draw_sprite_aligned(texture, { global::ui::g_position.x, m_smooth_scroll }, { global::ui::g_scale.x, global::ui::g_option_scale }, 0.f, global::ui::g_scroller);

        stl::vector<stl::shared_ptr<base_option>> options = menu::submenu::handler::get_current()->get_options();
        int count = stl::count_if(options.begin(), options.end(), [](stl::shared_ptr<base_option> option) { return option->is_visible(); });
        int render_count = count;

        if (render_count > max_options) {
            render_count = max_options;
        }

        // footer
        texture = get_texture(global::ui::m_footer);
        draw_sprite_aligned(texture, { global::ui::g_position.x, global::ui::g_position.y + (render_count * global::ui::g_option_scale) }, { global::ui::g_scale.x, global::ui::g_option_scale }, 0.f, global::ui::g_footer);

        draw_sprite({ "commonmenu", "shop_arrows_upanddown" }, { global::ui::g_position.x + ((global::ui::g_scale.x) * 0.5f), global::ui::g_position.y + (render_count * global::ui::g_option_scale) + (global::ui::g_option_scale * 0.5f) }, { 0.015f, 0.027f }, 0.f, { 255, 255, 255, 255 });

        // proportional scrollbar on the right edge when the list is longer than
        // the visible window (thumb size = visible/total, thumb pos = offset/total).
        if (total_options > max_options) {
            const float track_x = global::ui::g_position.x + global::ui::g_scale.x - 0.0035f;
            const float track_h = max_options * global::ui::g_option_scale;
            const float thumb_h = track_h * ((float)max_options / (float)total_options);
            const float denom = (float)(total_options - max_options);
            const float thumb_y = global::ui::g_position.y + (denom > 0.f ? (track_h - thumb_h) * ((float)scroll_offset / denom) : 0.f);
            draw_rect({ track_x, global::ui::g_position.y }, { 0.0022f, track_h }, global::ui::g_scroller.opacity(70));
            draw_rect({ track_x, thumb_y }, { 0.0022f, thumb_h }, global::ui::g_scroller);
        }

        // option counter
        char counter[50];
        snprintf(counter, sizeof(counter), "%i ~s~&#8226; %i", current_option + 1, count);
        draw_text(counter, { global::ui::g_position.x + 0.004f, global::ui::g_position.y + (render_count * global::ui::g_option_scale) + 0.004f }, get_normalized_font_scale(global::ui::g_sub_header_font, 0.30f), global::ui::g_sub_header_font, global::ui::g_sub_header_text, JUSTIFY_RIGHT, { 0.f, (1.0f - (1.0f - (global::ui::g_position.x + (0.315f / 2.f) - (0.23f - global::ui::g_scale.x)) - .068f)) });
    }

    void renderer::render_title(stl::string title) {
        if (global::ui::g_disable_title) return;
        for (size_t i = 0; i < title.length(); ++i) {
            char c = title[i];
            if (c >= 'a' && c <= 'z') title[i] = c - 32;
        }

        if (menu::submenu::handler::get_current() == main_menu::get()) {
            menu::animated_texture* slot_anim  = menu::animation::get("slot_header");
            menu::animated_texture* title_anim = menu::animation::get("banner");
            // Any picture in the header is the branding, whether it came from the
            // banner button or from the image picker, and whether it animates or
            // not. m_enabled covers the still-only case, where a picture is applied
            // but its animation is not ready.
            const bool header_shows_a_picture =
                (slot_anim && slot_anim->ready()) ||
                (title_anim && title_anim->ready()) ||
                rage::gfx::banner_ready() ||
                global::ui::m_header.m_enabled;
            if (!header_shows_a_picture) {
                draw_text("~s~&#248;ZARK " VERSION_TYPE, { global::ui::g_position.x + 0.005f, global::ui::g_position.y - 0.061f }, 0.77f, global::ui::g_header_font, global::ui::g_title, JUSTIFY_LEFT);
            }
        } else {
            stl::string input_string = title;
            stl::string final_string = title;

            float width = calculate_string_width(input_string, global::ui::g_header_font, 0.77f);
            while (width > global::ui::g_scale.x + (global::ui::g_header_font == menu::fonts::get_font_id("RDR") ? 0.055f : 0.f)) {
                if (input_string.length() > 0) {
                    input_string = input_string.substr(0, input_string.length() - 1);

                    while (input_string.length() > 0 && input_string[input_string.length() - 1] == ' ') {
                        input_string = input_string.substr(0, input_string.length() - 1);
                    }
                }

                final_string = input_string + "...";
                width = calculate_string_width(final_string, global::ui::g_header_font, 0.77f);
            }

            draw_text(final_string, { global::ui::g_position.x + 0.005f, global::ui::g_position.y - 0.061f }, 0.77f, global::ui::g_header_font, global::ui::g_title, JUSTIFY_LEFT);
        }
    }

    void renderer::render_tooltip(stl::string tooltip) {
        if (!tooltip.empty()) {
            global::ui::g_rendering_tooltip = tooltip;
            float y = 0.f;

            stl::vector<stl::shared_ptr<base_option>> options = menu::submenu::handler::get_current()->get_options();
            int count = stl::count_if(options.begin(), options.end(), [](stl::shared_ptr<base_option> option) { return option->is_visible(); });

            if (count > base::get_max_options()) {
                y = global::ui::g_position.y + ((base::get_max_options() + 1) * global::ui::g_option_scale) + 0.0025f;
            } else {
                y = global::ui::g_position.y + ((count + 1) * global::ui::g_option_scale) + 0.0025f;
            }

            if (global::ui::g_rendering_color) {
                y += 0.032f;
            }

            float scaled_body_height = menu::renderer::get_normalized_font_scale(global::ui::g_tooltip_font, global::ui::g_option_height);
            native::set_text_font(global::ui::g_tooltip_font);
            native::set_text_scale(0.f, scaled_body_height);
            native::set_text_wrap(global::ui::g_position.x + 0.004f, (1.0f - (1.0f - (global::ui::g_position.x + 0.1575f - (0.23f - global::ui::g_scale.x)) - global::ui::g_wrap)));
            native::begin_text_command_line_count("STRING");
            native::add_text_component_substring_player_name(tooltip.c_str());

            float height = global::ui::g_option_scale;

            int lines = native::end_text_command_get_line_count(global::ui::g_position.x + 0.004f, y + 0.005f);
            if (lines > 1) {
                // One wrapped line ~= one option-row height. The old formula used
                // get_text_scale_height(), whose PS4 fallback returns the text
                // SCALE (~0.35) instead of a real ~0.02 line height, ballooning the
                // box ~20x (see missing_natives.h GET_RENDERED_CHARACTER_HEIGHT).
                height = lines * global::ui::g_option_scale;
            }

            stl::pair<stl::string, stl::string> texture = get_texture(global::ui::m_tooltip_background);
            draw_sprite_aligned(texture, { global::ui::g_position.x, y }, { global::ui::g_scale.x, height }, 0.f, global::ui::g_tooltip);
            draw_text(tooltip.c_str(), { global::ui::g_position.x + 0.004f, y + 0.005f }, scaled_body_height, global::ui::g_tooltip_font, { 255, 255, 255, 255 }, JUSTIFY_LEFT, { global::ui::g_position.x + 0.004f, (1.0f - (1.0f - (global::ui::g_position.x + 0.1575f - (0.23f - global::ui::g_scale.x)) - global::ui::g_wrap)) });
        }

        global::ui::g_rendering_color = false;
    }

    void renderer::render_open_tooltip() {
        if (!global::ui::g_render_tooltip) return;

        char text[200];

        native::set_text_outline();

        if (native::is_input_disabled(2)) {
            snprintf(text, sizeof(text), "%s\n~c~%s", t_tooltip_1.get().c_str(), menu::input::g_key_names[menu::base::get_open_key()]);
            draw_text(text, { 0.5f, 0.09f }, get_normalized_font_scale(global::ui::g_open_tooltip_font, 0.40f), global::ui::g_open_tooltip_font, global::ui::g_open_tooltip, JUSTIFY_CENTER);
        } else {
            snprintf(text, sizeof(text), "%s\n~c~L1 + O", t_tooltip_1.get().c_str());
            draw_text(text, { 0.5f, 0.09f }, get_normalized_font_scale(global::ui::g_open_tooltip_font, 0.40f), global::ui::g_open_tooltip_font, global::ui::g_open_tooltip, JUSTIFY_CENTER);
        }
    }

    void renderer::render_color_preview(color_rgba color) {
        global::ui::g_rendering_color = true;

        stl::vector<stl::shared_ptr<base_option>> options = menu::submenu::handler::get_current()->get_options();
        int count = stl::count_if(options.begin(), options.end(), [](stl::shared_ptr<base_option> option) { return option->is_visible(); });

        if (count > base::get_max_options()) {
            count = base::get_max_options();
        }

        draw_rect({ global::ui::g_position.x, global::ui::g_position.y + ((count + 1) * global::ui::g_option_scale) + 0.0025f }, { global::ui::g_scale.x, global::ui::g_option_scale }, color);
    }

    float renderer::calculate_string_width(stl::string string, int font, float scale) {
        native::begin_text_command_width("STRING");
        native::add_text_component_substring_player_name(string.c_str());
        native::set_text_scale(0.f, scale);
        return native::end_text_command_get_width(font);
    }

    float renderer::get_normalized_font_scale(int font, float scale) {
        switch (font) {
            case 0: return (scale * 1.0f);
            case 1: return (scale * 1.3f);
            case 2: return (scale * 1.11f);
            case 4: return (scale * 1.11f);
            case 7: return (scale * 1.29f);
        }

        return scale;
    }

    void renderer::draw_rect(math::vector2<float> position, math::vector2<float> scale, color_rgba color) {
        if (global::ui::g_stop_rendering) return;
        native::draw_rect(position.x + (scale.x * 0.5f), position.y + (scale.y * 0.5f), scale.x, scale.y, color.r, color.g, color.b, color.a, 0);
    }

    void renderer::draw_rect_unaligned(math::vector2<float> position, math::vector2<float> scale, color_rgba color) {
        if (global::ui::g_stop_rendering) return;
        native::draw_rect(position.x, position.y, scale.x, scale.y, color.r, color.g, color.b, color.a, 0);
    }

    void renderer::draw_outlined_rect(math::vector2<float> position, math::vector2<float> scale, float thickness, color_rgba box_color, color_rgba border_color) {
        if (global::ui::g_stop_rendering) return;

        draw_rect({ position.x, position.y }, { scale.x, scale.y }, box_color);
        draw_rect({ position.x, position.y - thickness }, { scale.x, thickness }, border_color);
        draw_rect({ position.x, position.y + scale.y }, { scale.x, thickness }, border_color);

        draw_rect({ position.x - (thickness * 0.60f), position.y - thickness }, { (thickness * 0.60f), scale.y + (thickness * 2.f) }, border_color);
        draw_rect({ position.x + scale.x, position.y - thickness }, { (thickness * 0.60f), scale.y + (thickness * 2.f) }, border_color);
    }

    void renderer::draw_text(stl::string text, math::vector2<float> position, float scale, int font, color_rgba color, eJustify justification, math::vector2<float> wrap) {
        if (global::ui::g_stop_rendering) return;
        native::set_text_wrap(wrap.x, wrap.y);

        if (justification != JUSTIFY_LEFT) {
            native::set_text_justification(justification == JUSTIFY_CENTER ? 0 : 2);
        }

        native::set_text_centre(justification == JUSTIFY_CENTER);
        native::set_text_scale(0.f, scale);
        native::set_text_colour(color.r, color.g, color.b, color.a);
        native::set_text_font(font);

        if (text.length() >= 98) {
            global::ui::g_render_queue[global::ui::g_render_queue_index] = text;
            global::ui::g_render_queue_index++;
            global::ui::g_render_queue_index %= 100;
        }

        native::begin_text_command_display_text("STRING");
        native::add_text_component_substring_player_name(text.c_str());
        native::end_text_command_display_text(position.x, position.y, 0);
    }

    void renderer::draw_sprite(stl::pair<stl::string, stl::string> asset, math::vector2<float> position, math::vector2<float> scale, float rotation, color_rgba color) {
        if (global::ui::g_stop_rendering) return;

        // Sentinel dict: a disabled/missing menu_texture resolves to "randomha".
        // Ozark's texture-bypass hook turns those sprite draws into solid colour
        // quads. We don't port that game hook; instead draw the quad directly, so
        // the header/scroller/footer bars show as solid colour without any PNG.
        if (asset.first == "randomha") { draw_rect_unaligned(position, scale, color); return; }

        if (!native::has_streamed_texture_dict_loaded(asset.first.c_str()) && !rage::gfx::is_custom_dict(asset.first.c_str())) {
            native::request_streamed_texture_dict(asset.first.c_str(), true);
        }

        native::draw_sprite(asset.first.c_str(), asset.second.c_str(), position.x, position.y, scale.x, scale.y, rotation, color.r, color.g, color.b, color.a, 0);
    }

    void renderer::draw_sprite_aligned(stl::pair<stl::string, stl::string> asset, math::vector2<float> position, math::vector2<float> scale, float rotation, color_rgba color) {
        if (global::ui::g_stop_rendering) return;

        // See draw_sprite: sentinel -> solid colour quad (aligned).
        if (asset.first == "randomha") { draw_rect(position, scale, color); return; }

        if (!native::has_streamed_texture_dict_loaded(asset.first.c_str()) && !rage::gfx::is_custom_dict(asset.first.c_str())) {
            native::request_streamed_texture_dict(asset.first.c_str(), true);
        }

        native::draw_sprite(asset.first.c_str(), asset.second.c_str(), position.x + (scale.x * 0.5f), position.y + (scale.y * 0.5f), scale.x, scale.y, rotation, color.r, color.g, color.b, color.a, 1);
    }

    void renderer::draw_line(math::vector3<float> from, math::vector3<float> to, color_rgba color) {
        if (global::ui::g_stop_rendering) return;

        native::draw_line(from.x, from.y, from.z, to.x, to.y, to.z, color.r, color.g, color.b, color.a);
    }

    void renderer::draw_line_2d(math::vector3<float> from, math::vector3<float> to, color_rgba color) {
        if (global::ui::g_stop_rendering) return;

        // global::ui::m_line_2d is never allocated anywhere in this tree - it
        // sits null in .bss for the whole process - and nothing ever reads
        // g_line_2d_index or the buffer it indexes into, so filling it would
        // still draw nothing. The facility needs a render hook this port does
        // not have: Ozark's screen-space lines are consumed by a PC render
        // hook (menu/hooks/render_script_texture.cpp), which has no PS4
        // equivalent here. A caller that wants a line should call the 3D
        // menu::renderer::draw_line instead - it calls a real native and needs
        // no consumer.
        if (!global::ui::m_line_2d) return;

        if (global::ui::g_line_2d_index < 5000) {
            line_2d& line = global::ui::m_line_2d[global::ui::g_line_2d_index++];
            line.m_from.x = from.x;
            line.m_from.y = from.y;
            line.m_to.x = to.x;
            line.m_to.y = to.y;
            line.m_color = color;
        }
    }

    int renderer::get_render_count() {
        return m_render_count;
    }

    color_rgba renderer::hsv_to_rgb(float h, float s, float v, int original_alpha) {
        int r = 0, g = 0, b = 0;
        menu::color_math::hsv_to_rgb(h, s, v, &r, &g, &b);
        return color_rgba(r, g, b, original_alpha);
    }

    color_hsv renderer::rgb_to_hsv(color_rgba in) {
        menu::color_math::hsv h = menu::color_math::rgb_to_hsv(in.r, in.g, in.b);
        color_hsv out; out.h = h.h; out.s = h.s; out.v = h.v;
        return out;
    }

    renderer* get_renderer() {
        static renderer instance;
        return &instance;
    }
}
