#include "menu/base/util/theme.h"
#include "global/ui_vars.h"
#include "util/json.h"
#include "platform/log.h"

#include <orbis/libkernel.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

using namespace global::ui;

namespace menu::theme {
    const char* dir() { return "/data/insulin/themes"; }

    // ---- registries ---------------------------------------------------------
    struct nc { const char* name; color_rgba* p; };
    struct nf { const char* name; int* p; };
    struct nv { const char* name; math::vector2<float>* p; };

    static nc COLORS[] = {
        {"success",&g_success},{"error",&g_error},{"main_header",&g_main_header},{"sub_header",&g_sub_header},
        {"sub_header_text",&g_sub_header_text},{"background",&g_background},{"scroller",&g_scroller},{"footer",&g_footer},
        {"title",&g_title},{"open_tooltip",&g_open_tooltip},{"tooltip",&g_tooltip},{"option",&g_option},
        {"option_selected",&g_option_selected},{"toggle_on",&g_toggle_on},{"toggle_off",&g_toggle_off},{"break",&g_break},
        {"submenu_bar",&g_submenu_bar},{"clear_area_range",&g_clear_area_range},{"hotkey_bar",&g_hotkey_bar},
        {"color_grid_bar",&g_color_grid_bar},{"notify_bar",&g_notify_bar},{"notify_background",&g_notify_background},
        {"panel_bar",&g_panel_bar},{"stacked_display_bar",&g_stacked_display_bar},{"stacked_display_background",&g_stacked_display_background},
        {"panel_background",&g_panel_background},{"hotkey_background",&g_hotkey_background},{"color_grid_background",&g_color_grid_background},
        {"hotkey_input",&g_hotkey_input},{"instructional_background",&g_instructional_background},{"globe",&g_globe},
    };

    int color_count() { return (int)(sizeof(COLORS) / sizeof(COLORS[0])); }

    const char* color_name(int index) {
        if (index < 0 || index >= color_count()) return "";
        return COLORS[index].name;
    }

    color_rgba* color_ptr(int index) {
        if (index < 0 || index >= color_count()) return nullptr;
        return COLORS[index].p;
    }

    // Returned buffer is a function-local static: the result is only valid
    // until the next call, so build one display name at a time - never hold
    // two calls' results in the same expression (e.g. two of these as
    // printf args), the second call clobbers the first.
    const char* color_display_name(int index) {
        static char buf[64];
        if (index < 0 || index >= color_count()) return "";

        const char* src = COLORS[index].name;
        size_t i = 0;
        bool start_of_word = true;
        for (; src[i] != '\0' && i < sizeof(buf) - 1; i++) {
            char c = src[i];
            if (c == '_') {
                c = ' ';
                start_of_word = true;
            } else if (start_of_word) {
                if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
                start_of_word = false;
            }
            buf[i] = c;
        }
        buf[i] = '\0';
        return buf;
    }

    static nf FONTS[] = {
        {"header",&g_header_font},{"sub_header",&g_sub_header_font},{"option",&g_option_font},{"open_tooltip",&g_open_tooltip_font},
        {"tooltip",&g_tooltip_font},{"stacked_display",&g_stacked_display_font},{"notify_title",&g_notify_title_font},
        {"notify_body",&g_notify_body_font},{"panel",&g_panel_font},
    };
    static nv POS[] = {
        {"position",&g_position},{"scale",&g_scale},{"submenu_arrow_position",&g_submenu_arrow_position},
        {"submenu_arrow_scale",&g_submenu_arrow_scale},{"toggle_position",&g_toggle_position},{"toggle_scale",&g_toggle_scale},
        {"globe_position",&g_globe_position},{"globe_scale",&g_globe_scale},{"stacked_display_scale",&g_stacked_display_scale},
        {"stacked_display_position",&g_stacked_display_position},
    };

    // ---- save ---------------------------------------------------------------
    void save(const char* name) {
        sceKernelMkdir("/data/insulin", 0777);
        sceKernelMkdir(dir(), 0777);

        tj::json root;
        for (nc& c : COLORS) {
            tj::json& o = root["colors"][c.name];
            o["r"] = tj::json((long long)c.p->r);
            o["g"] = tj::json((long long)c.p->g);
            o["b"] = tj::json((long long)c.p->b);
            o["a"] = tj::json((long long)c.p->a);
        }
        for (nf& f : FONTS) root["fonts"][f.name] = tj::json((long long)*f.p);
        for (nv& v : POS) {
            tj::json& o = root["pos"][v.name];
            o["x"] = tj::json((double)v.p->x);
            o["y"] = tj::json((double)v.p->y);
        }
        root["misc"]["globe"] = tj::json(g_render_globe);
        root["misc"]["smooth"] = tj::json(g_scroll_lerp);
        root["misc"]["smooth_speed"] = tj::json((double)g_scroll_lerp_speed);
        root["misc"]["wrap"] = tj::json((double)g_wrap);

        char path[256];
        snprintf(path, sizeof(path), "%s/%s.json", dir(), name);
        root.save_to_file(path, 2);
        platform::logf("theme", "saved \"%s\"", path);
    }

    // ---- load ---------------------------------------------------------------
    bool load_file(const char* path) {
        tj::json root = tj::json::load_from_file(path);
        // load_from_file returns a null json() both when sceKernelOpen fails
        // (missing file) and when the file is empty - either way there is
        // nothing to apply, so callers that check the return value (boot-time
        // re-apply) can treat this the same as "missing".
        if (root.is_null()) {
            platform::logf("theme", "missing/unreadable \"%s\"", path);
            return false;
        }

        const tj::json* colors = root.try_get("colors");
        if (colors) for (nc& c : COLORS) {
            const tj::json* o = colors->try_get(c.name);
            if (o && o->is_object()) {
                c.p->r = (int)o->value_int("r", c.p->r);
                c.p->g = (int)o->value_int("g", c.p->g);
                c.p->b = (int)o->value_int("b", c.p->b);
                c.p->a = (int)o->value_int("a", c.p->a);
            }
        }
        const tj::json* fonts = root.try_get("fonts");
        if (fonts) for (nf& f : FONTS) {
            const tj::json* v = fonts->try_get(f.name);
            if (v && v->is_number()) *f.p = (int)v->get_int();
        }
        const tj::json* pos = root.try_get("pos");
        if (pos) for (nv& v : POS) {
            const tj::json* o = pos->try_get(v.name);
            if (o && o->is_object()) {
                v.p->x = (float)o->value_float("x", v.p->x);
                v.p->y = (float)o->value_float("y", v.p->y);
            }
        }
        const tj::json* misc = root.try_get("misc");
        if (misc) {
            g_render_globe = misc->value_bool("globe", g_render_globe);
            g_scroll_lerp = misc->value_bool("smooth", g_scroll_lerp);
            g_scroll_lerp_speed = (float)misc->value_float("smooth_speed", g_scroll_lerp_speed);
            g_wrap = (float)misc->value_float("wrap", g_wrap);
        }
        platform::logf("theme", "loaded \"%s\"", path);
        return true;
    }

    bool load_by_name(const char* name) {
        char path[256];
        snprintf(path, sizeof(path), "%s/%s.json", dir(), name);
        return load_file(path);
    }

    // ---- list ---------------------------------------------------------------
    stl::vector<stl::string> list() {
        stl::vector<stl::string> out;
        int fd = sceKernelOpen(dir(), 0 /* O_RDONLY */, 0);
        if (fd < 0) return out;

        char buf[4096];
        int n;
        while ((n = sceKernelGetdents(fd, buf, sizeof(buf))) > 0) {
            int pos = 0;
            while (pos < n) {
                struct dirent* de = (struct dirent*)(buf + pos);
                if (de->d_reclen == 0) break;
                int L = 0; while (de->d_name[L]) L++;
                if (L > 5) {
                    const char* e = de->d_name + L - 5;
                    if (e[0] == '.' && e[1] == 'j' && e[2] == 's' && e[3] == 'o' && e[4] == 'n') {
                        char stem[64]; int i = 0;
                        for (; i < L - 5 && i < 63; i++) stem[i] = de->d_name[i];
                        stem[i] = 0;
                        out.push_back(stl::string(stem));
                    }
                }
                pos += de->d_reclen;
            }
        }
        sceKernelClose(fd);
        return out;
    }

    // ---- default palette ----------------------------------------------------
    void reset_to_default() {
        g_render_globe = true; g_scroll_lerp = true; g_scroll_lerp_speed = 25.f; g_wrap = 0.063f;

        g_header_font = 0; g_sub_header_font = 4; g_option_font = 4; g_open_tooltip_font = 4; g_tooltip_font = 4;
        g_stacked_display_font = 0; g_notify_title_font = 0; g_notify_body_font = 0; g_panel_font = 4;

        g_position = { 0.70f, 0.3f }; g_scale = { 0.22f, 0.f };
        g_submenu_arrow_position = { 0.218f, 0.010f }; g_submenu_arrow_scale = { 0.007f, 0.013f };
        g_toggle_position = { 0.221f, 0.016f }; g_toggle_scale = { 0.007f, 0.011f };
        g_globe_position = { 0.4405f, 0.328f }; g_globe_scale = { 0.978f, 0.906f };
        g_stacked_display_scale = { 0.15f, 0.015f }; g_stacked_display_position = { 0.845f, 0.01f };

        g_success = { 70, 219, 37, 255 }; g_error = { 219, 37, 37, 255 }; g_main_header = { 220, 76, 81, 255 };
        g_sub_header = { 0, 0, 0, 220 }; g_sub_header_text = { 255, 255, 255, 255 }; g_background = { 0, 0, 0, 255 };
        g_scroller = { 186, 65, 69, 255 }; g_footer = { 220, 76, 81, 220 }; g_title = { 255, 255, 255, 255 };
        g_open_tooltip = { 220, 76, 81, 255 }; g_tooltip = { 220, 76, 81, 255 }; g_option = { 255, 255, 255, 255 };
        g_option_selected = { 255, 255, 255, 255 }; g_toggle_on = { 130, 214, 157, 255 }; g_toggle_off = { 200, 55, 80, 255 };
        g_break = { 255, 255, 255, 255 }; g_submenu_bar = { 255, 255, 255, 255 }; g_clear_area_range = { 220, 76, 81, 255 };
        g_hotkey_bar = { 220, 76, 81, 255 }; g_color_grid_bar = { 220, 76, 81, 255 }; g_notify_bar = { 220, 76, 81, 255 };
        g_notify_background = { 40, 40, 40, 255 }; g_panel_bar = { 220, 76, 81, 255 }; g_stacked_display_bar = { 220, 76, 81, 255 };
        g_stacked_display_background = { 0, 0, 0, 180 }; g_panel_background = { 0, 0, 0, 180 }; g_hotkey_background = { 0, 0, 0, 180 };
        g_color_grid_background = { 0, 0, 0, 180 }; g_hotkey_input = { 40, 40, 40, 200 }; g_instructional_background = { 0, 0, 0, 255 };
        g_globe = { 255, 255, 255, 255 };
    }
}
