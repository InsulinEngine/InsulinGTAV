#pragma once
#include "rage/invoker/invoker.h"
#include "rage/types/base_types.h"

// Base-menu natives that the generated natives.h omitted. Ozark's callsites use
// the short names below; these wrappers restore them with RVAs verified in the
// CUSA00411 v1.57 IDB (E:\Projects\IDA\PS4\GTA5\eboot_named.i64, imagebase 0x0
// so an impl address IS its RVA). Each native carries a `native NAME | hash=... |
// module=... | confidence=...` comment in that IDB; the address is the NAT_*
// impl the registration site (SetupScriptCommands_*) points at.
//
// VERIFIED (impl address from the IDB):
//   BEGIN_TEXT_COMMAND_GET_SCREEN_WIDTH_OF_DISPLAY_TEXT  0x9E0710  hud, verified
//   END_TEXT_COMMAND_GET_SCREEN_WIDTH_OF_DISPLAY_TEXT    0x9E0720  hud, aligned
//   BEGIN_TEXT_COMMAND_GET_NUMBER_OF_LINES_FOR_STRING    0x9E0750  hud, verified
//   END_TEXT_COMMAND_GET_NUMBER_OF_LINES_FOR_STRING      0x9E0760  hud, aligned
//   GET_CONTROL_INSTRUCTIONAL_BUTTONS_STRING             0xAA0FD0  pad, aligned
//
// NOT YET VERIFIED (not renamed in the IDB; safe fallbacks below, revisit in the
// same IDB when needed):
//   GET_RENDERED_CHARACTER_HEIGHT (get_text_scale_height) -> identity fallback
//   IS_INPUT_DISABLED                                     -> false (menu uses its
//                                                            own base input gate)
//
// Note for anything added here: gen_hash_natives.py reads THIS header as one of
// its EXISTING sets and skips any name it already finds. A stub written here
// therefore blocks the hash-based wrapper for the same native from ever being
// generated. PLAY_SOUND_FRONTEND sat here as a no-op for exactly that reason and
// is now served by natives_hash.h. Prefer deleting a stub over keeping it.
namespace native {

    // --- verified ---------------------------------------------------------
    static Void begin_text_command_width(const char* text) {
        return rage::invoker::invoke<Void>(0x9E0710, text);
    }
    static float end_text_command_get_width(int font) {
        return rage::invoker::invoke<float>(0x9E0720, font);
    }
    static Void begin_text_command_line_count(const char* entry) {
        return rage::invoker::invoke<Void>(0x9E0750, entry);
    }
    static int end_text_command_get_line_count(float x, float y) {
        return rage::invoker::invoke<int>(0x9E0760, x, y);
    }
    static const char* get_control_instructional_button(int inputGroup, int control, bool p2) {
        return rage::invoker::invoke<const char*>(0xAA0FD0, inputGroup, control, p2);
    }

    // --- fallbacks (RVA not yet verified in the v1.57 IDB) -----------------
    // Returns the requested size unchanged so renderer::get_normalized_font_scale
    // degrades to a no-op normalization (text renders at the requested scale)
    // instead of dividing by an unknown height. TODO: verify GET_RENDERED_
    // CHARACTER_HEIGHT's RVA and replace.
    static float get_text_scale_height(float size, int /*font*/) {
        return size;
    }
// The menu already gates input through menu::base's own disabled flag, so a
    // false here is correct behaviour, not just a stub. TODO: verify RVA if the
    // engine-level query is ever actually needed.
    static bool is_input_disabled(int /*inputGroup*/) {
        return false;
    }

    // --- scaleform method natives (instructional bar) ----------------------
    // These are the modern names for BEGIN/END_SCALEFORM_MOVIE_METHOD +
    // SCALEFORM_MOVIE_METHOD_ADD_PARAM_FLOAT, whose RVAs are the v1.57-validated
    // ones already used by scaleform.h (sf::). natives.h omits these names.
    static bool push_scaleform_movie_function(int handle, const char* method) {
        return rage::invoker::invoke<bool>(0x9D16D0, handle, method);
    }
    static Void push_scaleform_movie_function_parameter_float(float value) {
        return rage::invoker::invoke<Void>(0x9D19A0, value);
    }
    static Void push_scaleform_movie_function_parameter_int(int value) {
        return rage::invoker::invoke<Void>(0x9D1990, value);
    }
    static Void push_scaleform_movie_function_parameter_bool(bool value) {
        return rage::invoker::invoke<Void>(0x9D19C0, value);
    }
    static Void pop_scaleform_movie_function_void() {
        return rage::invoker::invoke<Void>(0x9D1870);
    }
    static Void draw_scaleform_movie_fullscreen(int scaleform, int r, int g, int b, int a, int p5) {
        return rage::invoker::invoke<Void>(0x9D11F0, scaleform, r, g, b, a, p5);
    }
    // SCALEFORM_MOVIE_METHOD_ADD_PARAM_PLAYER_NAME_STRING (hash 0xE83A3E3557A56640):
    // pushes a string that the movie treats as a player-name substring. Used by
    // the instructional bar to pass the control-button icon token.
    static Void _0xE83A3E3557A56640(const char* value) {
        return rage::invoker::invoke<Void>(0x9D1A50, value);
    }

    // Camera rotation. STILL A STUB, and no longer a cosmetic one: the globe it
    // was written for is gone, but game/aim_ray.h, weapon_gravity_gun.cpp and
    // player.cpp all derive directions from this. They are getting {0,0,0}, so
    // every one of them behaves as if the camera pointed along a fixed axis.
    // The fix is no longer an RVA hunt - GET_GAMEPLAY_CAM_ROT is in the source
    // header as hash 0xD84A545408A3099A, so deleting this stub lets
    // gen_hash_natives.py emit the real wrapper (see the note at the top of this
    // file about stubs shadowing generated natives). Left in place only because
    // it returns a Vector3, whose struct-return path through this invoker is the
    // thing that needs verifying first.
    static math::vector3<float> get_gameplay_cam_rot(int /*rotationOrder*/) {
        return math::vector3<float>(0.f, 0.f, 0.f);
    }

    // --- Vector3-returning coord natives (spawn positioning) --------------
    // RVAs IDA-verified against v1.57. The native writes its Vector3 into the
    // return slot, which the invoker reads back by value (same path the "Basic"
    // menu uses on-console).
    static math::vector3<float> get_entity_coords(Entity entity, bool alive) {
        return rage::invoker::invoke<math::vector3<float>>(0x9B2BC0, entity, alive);
    }
    static math::vector3<float> get_offset_from_entity_in_world_coords(Entity entity, float x, float y, float z) {
        return rage::invoker::invoke<math::vector3<float>>(0x9B3470, entity, x, y, z);
    }
}
