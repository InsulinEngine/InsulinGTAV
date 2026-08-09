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
// Ozark/Basic Input enum). Named to match Ozark's ControlFrontend* usage.
enum {
    ControlFrontendDown   = 187,
    ControlFrontendUp     = 188,
    ControlFrontendLeft   = 189,
    ControlFrontendRight  = 190,
    ControlFrontendAccept = 201,   // Cross
    ControlFrontendCancel = 202,   // Circle
    ControlFrontendX      = 203,   // Square
    ControlFrontendY      = 204,   // Triangle
    ControlFrontendLb     = 205,   // L1
    ControlFrontendRb     = 206,   // R1
    ControlFrontendLt     = 207,   // L2
    ControlFrontendRt     = 208,   // R2
};
