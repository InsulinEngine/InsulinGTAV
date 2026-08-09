#pragma once
#include "platform/stdafx.h"

// Minimal subset of Ozark's global::vars that the menu BASE actually reads.
// The full PC vars wall (network/ROS/engine pointers, arxan, window handle,
// steam, store manager, font tables) belongs to features or dropped platform
// paths and is intentionally absent. Only g_desktop_resolution survives: it
// gates a 5K-resolution scroll-speed special case in number/scroll options.
namespace global::vars {
    extern math::vector2<int> g_desktop_resolution;
    extern math::vector2<int> g_resolution;
    extern bool g_unloading;
}
