#pragma once

// GTA V ped bone ids, as passed to GET_PED_BONE_COORDS. These are the game's
// own values and are stable across versions; only the fourteen the skeleton
// draws are listed, because an unused constant is a constant nobody checked.
//
// Cross-checked against Ozark's src/rage/types/base_types.h (the shipped PC
// menu this port is based on) on 2026-08-16 - every value below matched
// exactly, no corrections needed.
namespace rage::ped_bones {
    constexpr int SKEL_Head       = 0x796E;
    constexpr int SKEL_Neck_1     = 0x9995;
    constexpr int SKEL_Pelvis     = 0x2E28;
    constexpr int SKEL_L_UpperArm = 0xB1C5;
    constexpr int SKEL_R_UpperArm = 0x9D4D;
    constexpr int SKEL_L_Forearm  = 0xEEEB;
    constexpr int SKEL_R_Forearm  = 0x6E5C;
    constexpr int SKEL_L_Hand     = 0x49D9;
    constexpr int SKEL_R_Hand     = 0xDEAD;
    constexpr int SKEL_L_Foot     = 0x3779;
    constexpr int SKEL_R_Foot     = 0xCC4D;
    constexpr int SKEL_L_Toe0     = 0x83C;
    constexpr int SKEL_R_Toe0     = 0x512D;
    constexpr int MH_L_Knee       = 0xB3FE;
    constexpr int MH_R_Knee       = 0x3FCF;
}
