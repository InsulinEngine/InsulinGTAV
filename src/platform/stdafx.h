#pragma once

// Replaces Ozark's Windows stdafx.h. Every ported menu file includes this. It
// pulls in the mini-STL (so transformed `stl::` types resolve), the RAGE handle
// typedefs, the platform compat/log shims, and the compile-time joaat hasher +
// XOR passthrough Ozark's sources expect.

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "stl/string.h"
#include "stl/vector.h"
#include "stl/shared_ptr.h"
#include "stl/function.h"
#include "stl/stack.h"
#include "stl/pair.h"
#include "stl/tuple.h"
#include "stl/unordered_map.h"
#include "stl/algorithm.h"
#include "stl/initializer_list.h"

#include "rage/types/base_types.h"
#include "platform/compat.h"
#include "platform/log.h"

#define XOR(x) (x)        // no string encryption on PS4
#ifndef VERSION
#define VERSION 34
#endif

template<typename T, int N> constexpr int NUMOF(T(&)[N]) { return N; }

// --- compile-time joaat (from Ozark stdafx.h), for parity where a transformed
// file writes joaat("literal"). Runtime hashing also comes from
// native::get_hash_key.
constexpr char CharacterMap[] = {
    '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_',
    '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_',
    '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_',
    '_', '_', '_', '_', '_', '_', '_', '.', '/', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?', '@',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '_', '_',  '_', '_', '_', '_',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
};

constexpr uint32_t JenkinsHash32(const char* String, uint32_t CurrentHashValue = 0) {
    while (*String != 0) {
        CurrentHashValue += CharacterMap[(uint8_t)*(String++)];
        CurrentHashValue += (CurrentHashValue << 10);
        CurrentHashValue ^= (CurrentHashValue >> 6);
    }
    CurrentHashValue += (CurrentHashValue << 3);
    CurrentHashValue ^= (CurrentHashValue >> 11);
    CurrentHashValue += (CurrentHashValue << 15);
    return CurrentHashValue;
}

#define joaat(String) \
    []() -> uint32_t { constexpr uint32_t HashValue = JenkinsHash32(String); return HashValue; }()
