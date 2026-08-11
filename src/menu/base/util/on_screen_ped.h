#pragma once
#include "platform/stdafx.h"
#include "util/math.h"

// Ozark's on-screen ped preview (menu/base/util/on_screen_ped.cpp), ported to
// GTA V PS4 CUSA00411 v1.57.
//
// Renders a live ped into the game's UI3DScene system at an arbitrary screen
// rect -- the same machinery the pause menu and MP character creator use, so the
// ped is lit, animated and composited by the engine rather than drawn by us.
//
// The push is per-frame state: the manager clears its active preset every frame,
// so draw_on_screen_ped() must be called from the tick loop for as long as the
// preview should be visible. A single call renders one frame.
namespace menu::screen::ped {
    // Why a draw call did not reach the scene. Every early-out in the push path
    // gets its own value: a plain bool collapses nine distinct causes into one
    // symptom, which is useless for diagnosing the experimental vehicle path.
    enum class scene_result {
        ok,                 // pushed, assigned and lit
        bad_handle,         // handle is 0, dead, or the wrong entity type
        no_eboot_base,      // module loaded before the eboot base resolved
        pause_menu_active,  // pause menu owns the UI3D scene
        no_manager,         // manager pointer still null (no session yet)
        scene_busy,         // another preset was already pushed this frame
        no_entity_pointer,  // fwScriptGuid could not resolve the handle
        no_preset,          // FACE_CREATION_CONFIRM missing from the table
        push_refused,       // UI3DScene::PushPreset said no
        assign_refused,     // UI3DScene::AssignPedToSlot said no
    };

    // Short human-readable form, for notifications and logs.
    const char* to_string(scene_result result);

    // Draw `ped` (a script handle, e.g. get_player_ped(-1)) into scene slot 0 of
    // the FACE_CREATION_CONFIRM preset.
    //   pos    - bgRect x/y, normalised screen coords (0..1)
    //   scale  - bgRect w/h, normalised
    //   position_offset - ped offset inside the scene box; Ozark's default puts
    //                     the camera 2m out and 0.4m up, framing the upper body.
    // Returns scene_result::ok only if the ped actually got pushed this frame.
    // Safe to call with an invalid handle, a closed session, or before the eboot
    // base is resolved -- each of those reports its own reason instead of
    // touching memory.
    scene_result draw_on_screen_ped(Ped ped,
                            math::vector2<float> pos,
                            math::vector2<float> scale,
                            math::vector3_<float> position_offset = { 0.00f, -2.00f, 0.40f });

    // --- Why there is no draw_on_screen_vehicle -----------------------------
    // Tried and reverted: it crashes the game the moment a vehicle reaches the
    // scene. Do not re-attempt without reading this.
    //
    // The tempting parts are real: the slot store (sub_7D1340) takes a generic
    // ref-counted entity with no type check, and the final draw is a virtual
    // call through entity->m_pDrawHandler, which CVehicleDrawHandler implements
    // just like CPedDrawHandler. Every stock entry point being gated on
    // g_TypeDesc_CPed looks at first like an API convention.
    //
    // It is not. The render path (sub_7D5090) calls sub_8CFAB0 on the slot
    // entity, which funnels into sub_B72CD0 -- and that reads entity+0x1020,
    // +0x1029 and +0x102A as ped variation indices (the same CPed field region
    // as the +0x10BC/+0x10D3 flags) and uses them to index the TXD store hash
    // buckets. Hand it a CVehicle and those are arbitrary bytes driving a store
    // lookup: wild read, SIGSEGV. The type gate mirrors a genuine constraint in
    // the renderer.
    //
    // Getting past it would mean detouring sub_8CFAB0 to skip the ped-variation
    // setup for non-ped entities, and that only clears the ONE ped-specific
    // dependency that has been found so far. For a vehicle preview, a dedicated
    // camera pointed at a spawned vehicle is the cheaper and sounder route.

    // Restore the preset element this module overwrote. The scene presets are
    // shared game data loaded once from uiscenes.meta, so the patched rect would
    // otherwise persist for the rest of the session (visible only if something
    // else uses FACE_CREATION_CONFIRM -- MP character creation). Call when the
    // preview is switched off. No-op if nothing was patched.
    void release();
}
