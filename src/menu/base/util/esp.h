#pragma once
#include "platform/stdafx.h"
#include "global/ui_vars.h"

// ESP as a shared facility rather than a feature, following Ozark: a context
// says what to draw and in which colours, and any consumer can hand any entity
// to draw_entity(). Consumers own their contexts; the settings menu edits
// whichever one is current.
namespace menu::esp {

    struct esp_context {
        bool m_ped = false;              // gates skeleton and weapon
        bool m_name = false;
        bool m_snapline = false;
        bool m_2d_box = false;
        bool m_2d_corners = false;
        bool m_healthbar = false;
        bool m_3d_box = false;
        bool m_3d_axis = false;
        bool m_skeleton_bones = false;
        bool m_skeleton_joints = false;
        bool m_weapon = false;

        int m_name_type = 0;             // 0 = name, 1 = name + distance
        // Metres, and an int rather than a float on purpose: scroll_option only
        // binds `int&` (verified - `add_scroll(int&, int, int, scroll_struct*)`
        // is its only form), and a distance in whole metres loses nothing.
        int m_max_distance = 500;        // per-context cull
        int m_skeleton_distance = 75;    // bones and joints only within this

        color_rgba m_name_text_color        = color_rgba(255, 255, 255, 255);
        color_rgba m_name_bg_color          = color_rgba(0, 0, 0, 180);
        color_rgba m_snapline_color         = color_rgba(255, 0, 255, 255);
        color_rgba m_2d_box_color           = color_rgba(255, 0, 255, 255);
        color_rgba m_2d_corners_color       = color_rgba(255, 0, 255, 255);
        color_rgba m_healthbar_color        = color_rgba(0, 255, 0, 255);
        color_rgba m_3d_box_color           = color_rgba(255, 0, 255, 255);
        color_rgba m_skeleton_bones_color   = color_rgba(255, 0, 255, 255);
        color_rgba m_skeleton_joints_color  = color_rgba(255, 0, 255, 255);
        color_rgba m_weapon_color           = color_rgba(255, 255, 255, 255);

        bool m_name_text_rainbow = false;
        bool m_name_bg_rainbow = false;
        bool m_snapline_rainbow = false;
        bool m_2d_box_rainbow = false;
        bool m_2d_corners_rainbow = false;
        bool m_healthbar_rainbow = false;
        bool m_3d_box_rainbow = false;
        bool m_skeleton_bones_rainbow = false;
        bool m_skeleton_joints_rainbow = false;
        bool m_weapon_rainbow = false;

        // True when any element at all is on. Consumers check this before
        // walking their entity list, so a fully-off context costs one branch
        // per frame rather than a world scan.
        bool any() const {
            return m_name || m_snapline || m_2d_box || m_2d_corners || m_healthbar
                || m_3d_box || m_3d_axis || m_skeleton_bones || m_skeleton_joints
                || m_weapon;
        }
    };

    // Called once per frame, before any consumer draws. Resets the per-frame
    // entity budget and advances the ESP's frame clock. Touches no native, so
    // it is safe at the top of the tick - above the ShellUI overlay guard and
    // before the local player exists.
    void begin_frame();

    // Caches the local player's ped and coordinates for this frame, which every
    // element is defined relative to. Calls natives (five for the player entry
    // plus a coord fetch), so unlike begin_frame() it must run below the overlay
    // guard and only while game::player_valid() - immediately before the
    // consumer sweep. Any frame this does not run, draw_entity draws nothing
    // rather than reusing the previous frame's origin.
    void resolve_local_player();

    // Draw one entity through one context. Re-checks the entity itself: an
    // entity can die between the loop that selected it and this call, and every
    // consumer would otherwise need the same guard.
    // `name_override` may be null, in which case the model hash is shown.
    void draw_entity(const esp_context& ctx, Entity entity, const char* name_override);

    // How many entities the current frame has already drawn, and the cap.
    // `drawn_this_frame() >= frame_cap()` says the budget is spent, which a
    // consumer that must enumerate before it can draw checks first: consumers
    // run in registration order, so a full lobby can spend the whole budget
    // before a later one is called, and enumerating the vehicle pool (~9
    // natives per pooled vehicle) to draw nothing is pure waste.
    int  drawn_this_frame();
    int  frame_cap();
}
