#pragma once
#include <stdint.h>
#include <orbis/libkernel.h>

// Time + control-index constants that replace the Windows/PC facilities Ozark's
// menu base reaches for. Kept tiny and header-only.
namespace platform {
    // Millisecond monotonic clock; replaces GetTickCount()/GetTickCount64()/timeGetTime().
    inline uint32_t now_ms() { return (uint32_t)(sceKernelGetProcessTime() / 1000ULL); }
    inline uint64_t now_ms64() { return sceKernelGetProcessTime() / 1000ULL; }

    // Tiny LCG for the renderer's random-tooltip pick (replaces <random>).
    inline uint32_t rand_u32() {
        static uint32_t s = 0x2545F491u;
        if (s == 0x2545F491u) s ^= (uint32_t)sceKernelGetProcessTime();
        s = s * 1664525u + 1013904223u;
        return s;
    }
}

// GTA control indices used by transformed menu input (RAGE eControl; see the
// Ozark/Basic Input enum). A named enum so it also serves as the overload tag
// for instructionals::add_instructional(text, eControls).
enum eControls {
    // Gameplay controls (group 0). The frontend ids below are group 2.
    ControlSprint         = 21,    // Cross held, on foot

    ControlPhoneSelect    = 176,
    ControlFrontendDown   = 187,
    ControlFrontendUp     = 188,
    ControlFrontendLeft   = 189,
    ControlFrontendRight  = 190,
    ControlFrontendRdown  = 191,
    ControlFrontendAccept = 201,   // Cross
    ControlFrontendCancel = 202,   // Circle
    ControlFrontendX      = 203,   // Square
    ControlFrontendY      = 204,   // Triangle
    ControlFrontendLb     = 205,   // L1
    ControlFrontendRb     = 206,   // R1
    ControlFrontendLt     = 207,   // L2
    ControlFrontendRt     = 208,   // R2
};

// Scaleform instructional-button icon ids (RAGE order; from the reference menu).
// Overload tag for add_instructional(text, eScaleformButtons) + real icon index.
enum eScaleformButtons {
    ARROW_UP, ARROW_DOWN, ARROW_LEFT, ARROW_RIGHT,
    BUTTON_DPAD_UP, BUTTON_DPAD_DOWN, BUTTON_DPAD_RIGHT, BUTTON_DPAD_LEFT,
    BUTTON_DPAD_BLANK, BUTTON_DPAD_ALL, BUTTON_DPAD_UP_DOWN, BUTTON_DPAD_LEFT_RIGHT,
    BUTTON_LSTICK_UP, BUTTON_LSTICK_DOWN, BUTTON_LSTICK_LEFT, BUTTON_LSTICK_RIGHT,
    BUTTON_LSTICK, BUTTON_LSTICK_ALL, BUTTON_LSTICK_UP_DOWN, BUTTON_LSTICK_LEFT_RIGHT,
    BUTTON_LSTICK_ROTATE, BUTTON_RSTICK_UP, BUTTON_RSTICK_DOWN, BUTTON_RSTICK_LEFT,
    BUTTON_RSTICK_RIGHT, BUTTON_RSTICK, BUTTON_RSTICK_ALL, BUTTON_RSTICK_UP_DOWN,
    BUTTON_RSTICK_LEFT_RIGHT, BUTTON_RSTICK_ROTATE, BUTTON_A, BUTTON_B, BUTTON_X,
    BUTTON_Y, BUTTON_LB, BUTTON_LT, BUTTON_RB, BUTTON_RT, BUTTON_START, BUTTON_BACK,
    RED_BOX, RED_BOX_1, RED_BOX_2, RED_BOX_3, LOADING_HALF_CIRCLE_LEFT,
    ARROW_UP_DOWN, ARROW_LEFT_RIGHT, ARROW_ALL,
};

// Windows VK codes the stubbed keyboard paths reference; harmless placeholders.
#ifndef VK_F12
#define VK_F12 0x7B
#endif
#ifndef VK_F4
#define VK_F4 0x73
#endif

// RAGE control (INPUT_*) indices used by base::update()'s control-disable list
// and by transformed input code. Values are the canonical eControl ordinals.
enum {
    INPUT_NEXT_CAMERA = 0,
    INPUT_ARREST = 49,
    INPUT_CONTEXT = 51,
    INPUT_HUD_SPECIAL = 48,
    INPUT_CHARACTER_WHEEL = 19,
    INPUT_VEH_CIN_CAM = 80,
    INPUT_VEH_HEADLIGHT = 74,
    INPUT_VEH_RADIO_WHEEL = 85,
    INPUT_VEH_SELECT_NEXT_WEAPON = 99,
    INPUT_MELEE_ATTACK_LIGHT = 140,
    INPUT_MELEE_ATTACK_HEAVY = 141,
    INPUT_MELEE_BLOCK = 143,
    INPUT_SELECT_CHARACTER_MICHAEL = 166,
    INPUT_SELECT_CHARACTER_FRANKLIN = 167,
    INPUT_SELECT_CHARACTER_TREVOR = 168,
    INPUT_SELECT_CHARACTER_MULTIPLAYER = 169,
    INPUT_FRONTEND_DOWN = 187,
    INPUT_FRONTEND_UP = 188,
    INPUT_FRONTEND_LEFT = 189,
    INPUT_FRONTEND_RIGHT = 190,
    INPUT_FRONTEND_RDOWN = 191,
    INPUT_FRONTEND_ACCEPT = 201,
    INPUT_FRONTEND_CANCEL = 202,
};
