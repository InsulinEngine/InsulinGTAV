#include "menu/base/submenus/helper_esp_settings.h"
#include "menu/base/submenus/helper_esp.h"
#include "menu/base/submenus/helper_esp_settings_edit.h"
#include "menu/base/options/submenu_option.h"
#include "menu/base/options/button.h"

namespace {
    menu::esp::esp_context* g_built_for = nullptr;

    // Adding an entry means naming its colour and its rainbow flag once, here.
    // A table rather than twelve near-identical lambdas: the lambdas would each
    // capture a different pair of pointers, and stl::function caps captures at
    // 64 bytes.
    struct entry {
        const char* name;
        size_t      color_offset;
        size_t      rainbow_offset;
    };

    #define ESP_ENTRY(label, color_field, rainbow_field) \
        { label, offsetof(menu::esp::esp_context, color_field), \
                 offsetof(menu::esp::esp_context, rainbow_field) }

    const entry k_entries[] = {
        ESP_ENTRY("Name - Text",        m_name_text_color,       m_name_text_rainbow),
        ESP_ENTRY("Name - Background",  m_name_bg_color,         m_name_bg_rainbow),
        ESP_ENTRY("Snapline",           m_snapline_color,        m_snapline_rainbow),
        ESP_ENTRY("2D Box",             m_2d_box_color,          m_2d_box_rainbow),
        ESP_ENTRY("2D Corners",         m_2d_corners_color,      m_2d_corners_rainbow),
        ESP_ENTRY("Health Bar",         m_healthbar_color,       m_healthbar_rainbow),
        ESP_ENTRY("3D Box",             m_3d_box_color,          m_3d_box_rainbow),
        ESP_ENTRY("Skeleton - Bones",   m_skeleton_bones_color,  m_skeleton_bones_rainbow),
        ESP_ENTRY("Skeleton - Joints",  m_skeleton_joints_color, m_skeleton_joints_rainbow),
        ESP_ENTRY("Weapon",             m_weapon_color,          m_weapon_rainbow),
    };
    #undef ESP_ENTRY
}

void helper_esp_settings_menu::load() {
    set_name("Colours");
    set_parent<helper_esp_menu>();
    update_once();
}

void helper_esp_settings_menu::update_once() {
    clear_options(0);
    g_built_for = helper_esp_menu::current();

    if (!g_built_for) {
        add_option(button_option("~m~No ESP target").ref());
        return;
    }

    const int count = (int)(sizeof(k_entries) / sizeof(k_entries[0]));
    for (int i = 0; i < count; i++) {
        int idx = i;   // tiny capture: stl::function caps captures at 64 bytes
        add_option(submenu_option(k_entries[i].name)
            .add_submenu<helper_esp_settings_edit_menu>()
            .add_click([idx] {
                menu::esp::esp_context* c = helper_esp_menu::current();
                if (!c) return;
                uint8_t* base = (uint8_t*)c;
                helper_esp_settings_edit_menu::target(
                    (color_rgba*)(base + k_entries[idx].color_offset),
                    (bool*)(base + k_entries[idx].rainbow_offset),
                    k_entries[idx].name);
            }));
    }
}

void helper_esp_settings_menu::update() {
    if (g_built_for != helper_esp_menu::current()) update_once();
}

helper_esp_settings_menu* helper_esp_settings_menu::get() {
    static helper_esp_settings_menu instance;
    return &instance;
}
