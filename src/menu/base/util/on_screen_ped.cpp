#include "menu/base/util/on_screen_ped.h"
#include "rage/invoker/invoker.h"
#include "rage/invoker/natives.h"

// Ozark's on_screen_ped.cpp, retargeted to CUSA00411 v1.57. Ozark resolves its
// four addresses with a PC sigscan ("3DPED"); this build has no sigscanner, so
// they are RVAs into the eboot (imagebase 0), made absolute at call time via
// rage::invoker::g_eboot_base -- same pattern as rage/gfx.cpp and heap_guard.cpp.
//
// Every address and offset below was read out of E:\Projects\IDA\PS4\GTA5\
// eboot_named.i64, not translated from the PC numbers:
//
//   g_UI3DSceneManager  0x3393B78  a qword holding the manager*, exactly like
//                                  Ozark's g_ui_3d_draw_manager. Reached via
//                                  `lea rcx, g_UI3DSceneManager / mov rcx,[rcx]`
//                                  in NATa_UI3DSCENE_IS_AVAILABLE 0x9D1EB0.
//   UI3DScene__PushPreset      0x7D15F0  == Ozark push_scene_preset_to_manager
//   UI3DScene__AssignPedToSlot 0x7D1480  == Ozark add_element_to_scene
//   set element lighting       0x7D1540  == Ozark set_scene_element_lighting
//                                  (writes min(intensity, cap) to the slot's
//                                  +0x798 field; sub_7D1540 in the IDB)
//
// Two things differ from the PC source and are NOT cosmetic:
//
// 1. ABI. Ozark passes position_offset as a 12-byte struct by value; MSVC x64
//    turns that into a hidden pointer, which is what the callee wants. SysV
//    would pass it in XMM registers instead -- and the callee dereferences it
//    and reads SIXTEEN bytes (sub_7D1340 loads [a5], [a5+4], [a5+0xC]). So the
//    offset is materialised here as a padded vector4 and passed by address.
//
// 2. CPed flag offsets. Ozark uses the PC's +0x1163 / +0x114C. On this build
//    NATa_UI3DSCENE_ASSIGN_PED_TO_SLOT (0x9D1F40) does
//    `*(BYTE*)(ped+0x10D3) |= 0x40` and `*(BYTE*)(ped+0x10BC) |= 0xC0`.
//    UI3DScene__AssignPedToSlot itself does not set them, so we must.
namespace menu::screen::ped {

    // ---- eboot RVAs (CUSA00411 v1.57, imagebase 0) ---------------------------
    static constexpr uint64_t RVA_UI3D_MANAGER = 0x3393B78; // qword: UI3DScene manager*
    static constexpr uint64_t RVA_PUSH_PRESET  = 0x7D15F0;  // bool(mgr, u32* nameHash)
    static constexpr uint64_t RVA_ASSIGN_PED   = 0x7D1480;  // bool(mgr, u32* nameHash, slot, CPed*, vec4* offset, float)
    static constexpr uint64_t RVA_SET_LIGHTING = 0x7D1540;  // void(mgr, u32* nameHash, slot, float)
    static constexpr uint64_t RVA_GUID_TO_BASE = 0x1EE5180; // fwScriptGuid::GetBaseFromGuid(handle) -> fwEntity*

    // ---- manager field offsets (verified in UI3DScene__PushPreset) -----------
    static constexpr uint64_t MGR_PRESETS_OFF = 0x728; // scene_preset* array
    static constexpr uint64_t MGR_COUNT_OFF   = 0x730; // u16 preset count
    static constexpr uint64_t MGR_ACTIVE_OFF  = 0x920; // qword: preset pushed this frame

    // ---- CPed flag offsets (verified in NATa_UI3DSCENE_ASSIGN_PED_TO_SLOT) ---
    static constexpr uint64_t PED_UI3D_FLAG_OFF   = 0x10D3; // |= 0x40  draw in UI3D scene
    static constexpr uint64_t PED_RENDER_FLAG_OFF = 0x10BC; // |= 0xC0  force render / skip cull

    // joaat("face_creation_confirm"). Ozark hardcodes the hash; resolved here
    // against common.rpf/data/ui/uiscenes.meta, which defines the preset with 5
    // elements (element 0 enabled). The preset ships in the base game data, so
    // it is present on PS4 too -- it is only ever *used* by MP character
    // creation, which is why it is safe to borrow in Story Mode.
    static constexpr uint32_t SCENE_HASH = 0x390DCCF5;
    static constexpr uint32_t SCENE_SLOT = 0;

    // The trailing float of AssignPedToSlot lands in the slot's +0x794 field.
    // UI3DSCENE_ASSIGN_PED_TO_SLOT passes 0.0f there; Ozark passes 1.0f. 1.0f is
    // what this port uses and what was confirmed to render on-console.
    static constexpr float SCENE_ELEMENT_PARAM = 1.0f;
    static constexpr float SCENE_LIGHT_INTENSITY = 1.0f;

    // A UI3DDrawManager scene preset, 0x2A0 bytes. Ozark's struct is offset-
    // compatible but mis-partitioned (it starts the element array at +0x08 and
    // labels the wrong field m_enabled); this layout is the one the binary and
    // uiscenes.meta agree on:
    //   * element array at preset+0x10, stride 0x80, 5 entries, count at +0x290
    //     -- from the preset-remove shuffle (sub_7D2050: copies
    //     `count << 7` bytes from preset+16) and the destructor (sub_7D0D70 ->
    //     sub_22CED50, which releases refs at +0x00/0x80/0x100/0x180/0x200).
    //   * field order from the element copy operator sub_22CE9B0, matched
    //     one-to-one against the <Item> schema in uiscenes.meta.
    struct scene_preset {
        struct element {
            void*    m_assigned;                       // 0x00 ref-counted entity slot
            char     _0x0008[8];                       // 0x08
            math::vector4<float> m_position;           // 0x10
            math::vector4<float> m_position_43;        // 0x20
            math::vector4<float> m_rotation_xyz;       // 0x30
            math::vector4<float> m_bg_rect_xywh;       // 0x40 x/y = top-left, z/w = w/h
            math::vector4<float> m_bg_rect_xywh_43;    // 0x50 same, 4:3 variant
            uint32_t m_bg_rect_color;                  // 0x60
            uint32_t m_blend_color;                    // 0x64
            math::vector2<float> m_perspective_shear;  // 0x68
            uint32_t m_black_white_weights;            // 0x70
            uint32_t m_tint_color;                     // 0x74
            float    m_postfx_blend;                   // 0x78
            bool     m_enabled;                        // 0x7C
            char     _0x007D[3];                       // 0x7D
        };

        uint32_t m_name;            // 0x00 joaat of the <name> in uiscenes.meta
        char     _0x0004[12];       // 0x04
        element  m_elements[5];     // 0x10
        int32_t  m_element_count;   // 0x290
        char     _0x0294[12];       // 0x294
    };

    static_assert(sizeof(scene_preset::element) == 0x80, "UI3DScene element must be 0x80");
    static_assert(sizeof(scene_preset) == 0x2A0, "UI3DScene preset stride must be 0x2A0");
    static_assert(offsetof(scene_preset, m_elements) == 0x10, "element array at preset+0x10");
    static_assert(offsetof(scene_preset, m_element_count) == 0x290, "count at preset+0x290");
    static_assert(offsetof(scene_preset::element, m_bg_rect_xywh) == 0x40, "bgRect at element+0x40");

    typedef bool  (*push_preset_fn)(uintptr_t mgr, uint32_t* name_hash);
    typedef bool  (*assign_ped_fn)(uintptr_t mgr, uint32_t* name_hash, uint32_t slot,
                                   void* ped, const math::vector4<float>* offset, float param);
    typedef void  (*set_lighting_fn)(uintptr_t mgr, uint32_t* name_hash, uint32_t slot, float intensity);
    typedef void* (*guid_to_base_fn)(uint32_t guid);

    template<typename Fn> static inline Fn as_fn(uint64_t rva) {
        return (Fn)(rage::invoker::g_eboot_base + rva);
    }

    // The fields this module overwrites in the shared preset, saved on the first
    // patch so release() can put the metadata values back.
    struct saved_element {
        math::vector4<float> m_rotation_xyz;
        math::vector4<float> m_bg_rect_xywh;
        math::vector4<float> m_bg_rect_xywh_43;
        uint32_t             m_bg_rect_color;
    };
    static saved_element g_saved;
    static bool          g_patched = false;

    static uintptr_t get_manager() {
        if (!rage::invoker::g_eboot_base)
            return 0;
        return *(uintptr_t*)(rage::invoker::g_eboot_base + RVA_UI3D_MANAGER);
    }

    // Ozark's get_scene_preset, rewritten against the PS4 field offsets. Same
    // linear scan the game itself does in UI3DScene__PushPreset.
    static scene_preset* find_preset(uintptr_t mgr, uint32_t name_hash) {
        uint16_t  count = *(uint16_t*)(mgr + MGR_COUNT_OFF);
        uintptr_t array = *(uintptr_t*)(mgr + MGR_PRESETS_OFF);
        if (!count || !array)
            return nullptr;

        for (uint16_t i = 0; i < count; ++i) {
            scene_preset* preset = (scene_preset*)(array + sizeof(scene_preset) * i);
            if (preset->m_name == name_hash)
                return preset;
        }
        return nullptr;
    }

    static void patch_element(scene_preset::element& e,
                              math::vector2<float> pos,
                              math::vector2<float> scale) {
        if (!g_patched) {
            g_saved.m_rotation_xyz      = e.m_rotation_xyz;
            g_saved.m_bg_rect_xywh      = e.m_bg_rect_xywh;
            g_saved.m_bg_rect_xywh_43   = e.m_bg_rect_xywh_43;
            g_saved.m_bg_rect_color     = e.m_bg_rect_color;
            g_patched = true;
        }

        // The preset tilts the ped by -15 degrees for the character creator;
        // upright is what a menu preview wants.
        e.m_rotation_xyz.z = 0.0f;

        // Both variants get the same rect so the preview does not jump when the
        // game switches to its 4:3 layout.
        e.m_bg_rect_xywh.x    = pos.x;
        e.m_bg_rect_xywh.y    = pos.y;
        e.m_bg_rect_xywh.z    = scale.x;
        e.m_bg_rect_xywh.w    = scale.y;
        e.m_bg_rect_xywh_43.x = pos.x;
        e.m_bg_rect_xywh_43.y = pos.y;
        e.m_bg_rect_xywh_43.z = scale.x;
        e.m_bg_rect_xywh_43.w = scale.y;

        // Transparent backdrop: the menu draws its own.
        e.m_bg_rect_color = 0x0;
    }

    const char* to_string(scene_result result) {
        switch (result) {
            case scene_result::ok:                return "ok";
            case scene_result::bad_handle:        return "bad handle / wrong type";
            case scene_result::no_eboot_base:     return "no eboot base";
            case scene_result::pause_menu_active: return "pause menu active";
            case scene_result::no_manager:        return "no UI3D manager";
            case scene_result::scene_busy:        return "scene busy (already pushed)";
            case scene_result::no_entity_pointer: return "handle did not resolve";
            case scene_result::no_preset:         return "preset not found";
            case scene_result::push_refused:      return "PushPreset refused";
            case scene_result::assign_refused:    return "AssignPedToSlot refused";
        }
        return "unknown";
    }

    // Resolve the manager, confirm the scene is free this frame, and turn a
    // script handle into an entity pointer. Everything the ped and vehicle paths
    // share BEFORE their type-specific preparation.
    static scene_result begin_scene(Entity entity, uintptr_t& mgr_out, void*& ptr_out) {
        if (!rage::invoker::g_eboot_base)
            return scene_result::no_eboot_base;
        if (native::is_pause_menu_active())
            return scene_result::pause_menu_active;

        uintptr_t mgr = get_manager();
        if (!mgr)
            return scene_result::no_manager;

        // This is UI3DSCENE_IS_AVAILABLE (0x9D1EB0) inlined, minus its unguarded
        // manager dereference: a non-null active preset means something already
        // pushed this frame and PushPreset would refuse ours anyway.
        if (*(uintptr_t*)(mgr + MGR_ACTIVE_OFF) != 0)
            return scene_result::scene_busy;

        void* ptr = as_fn<guid_to_base_fn>(RVA_GUID_TO_BASE)((uint32_t)entity);
        if (!ptr)
            return scene_result::no_entity_pointer;

        mgr_out = mgr;
        ptr_out = ptr;
        return scene_result::ok;
    }

    // Patch the preset rect, then push / assign / light. Shared tail; the entity
    // is stored by the game without any type check, which is what makes the
    // vehicle path possible at all.
    static scene_result push_entity_to_scene(uintptr_t mgr, void* entity,
                                             math::vector2<float> pos,
                                             math::vector2<float> scale,
                                             math::vector3_<float> position_offset) {
        scene_preset* preset = find_preset(mgr, SCENE_HASH);
        if (!preset || preset->m_element_count <= (int32_t)SCENE_SLOT)
            return scene_result::no_preset;

        patch_element(preset->m_elements[SCENE_SLOT], pos, scale);

        // The callees take the hash by pointer and read it as an int.
        uint32_t scene_hash = SCENE_HASH;

        if (!as_fn<push_preset_fn>(RVA_PUSH_PRESET)(mgr, &scene_hash))
            return scene_result::push_refused;

        // Padded to 16 bytes on purpose -- see the ABI note in the file header.
        const math::vector4<float> offset = {
            position_offset.x, position_offset.y, position_offset.z, 0.0f
        };

        if (!as_fn<assign_ped_fn>(RVA_ASSIGN_PED)(mgr, &scene_hash, SCENE_SLOT,
                                                  entity, &offset, SCENE_ELEMENT_PARAM))
            return scene_result::assign_refused;

        as_fn<set_lighting_fn>(RVA_SET_LIGHTING)(mgr, &scene_hash, SCENE_SLOT,
                                                 SCENE_LIGHT_INTENSITY);
        return scene_result::ok;
    }

    scene_result draw_on_screen_ped(Ped ped,
                                    math::vector2<float> pos,
                                    math::vector2<float> scale,
                                    math::vector3_<float> position_offset) {
        // The flag writes below go to fixed CPed offsets, so the handle has to be
        // a ped -- a merely-live entity would get two bytes scribbled into
        // whatever its layout has there.
        if (!native::does_entity_exist(ped) || !native::is_entity_a_ped(ped))
            return scene_result::bad_handle;

        uintptr_t mgr = 0;
        void* ped_ptr = nullptr;
        scene_result begun = begin_scene(ped, mgr, ped_ptr);
        if (begun != scene_result::ok)
            return begun;

        *((uint8_t*)ped_ptr + PED_UI3D_FLAG_OFF)   |= 0x40;
        *((uint8_t*)ped_ptr + PED_RENDER_FLAG_OFF) |= 0xC0;

        return push_entity_to_scene(mgr, ped_ptr, pos, scale, position_offset);
    }

    void release() {
        if (!g_patched)
            return;

        uintptr_t mgr = get_manager();
        if (mgr) {
            scene_preset* preset = find_preset(mgr, SCENE_HASH);
            if (preset && preset->m_element_count > (int32_t)SCENE_SLOT) {
                scene_preset::element& e = preset->m_elements[SCENE_SLOT];
                e.m_rotation_xyz    = g_saved.m_rotation_xyz;
                e.m_bg_rect_xywh    = g_saved.m_bg_rect_xywh;
                e.m_bg_rect_xywh_43 = g_saved.m_bg_rect_xywh_43;
                e.m_bg_rect_color   = g_saved.m_bg_rect_color;
            }
        }
        g_patched = false;
    }
}
