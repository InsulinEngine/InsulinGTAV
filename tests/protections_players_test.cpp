// Host unit tests for per-player protection block state.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/protections_players_test.cpp -o build/protections_players_test.exe
//   ./build/protections_players_test.exe
// C headers only: MSVC's C++ stdlib rejects the installed clang (STL1000).
#include "protections/players.h"
#include <stdio.h>

static int g_failed = 0;

static void check_true(const char* what, bool ok) {
    if (!ok) { printf("FAIL %s\n", what); g_failed++; }
    else     { printf("ok   %s\n", what); }
}

int main() {
    using namespace protections;

    blocks b;

    for (int i = 0; i < 32; i++)
        check_true("starts unblocked", !b.is_blocked(block_kind::net_events, i));

    // One player's bit must not disturb its neighbours.
    b.set_blocked(block_kind::net_events, 5, true);
    check_true("player 5 blocked", b.is_blocked(block_kind::net_events, 5));
    check_true("player 4 unaffected", !b.is_blocked(block_kind::net_events, 4));
    check_true("player 6 unaffected", !b.is_blocked(block_kind::net_events, 6));

    // The three kinds are independent.
    check_true("clone_sync unaffected", !b.is_blocked(block_kind::clone_sync, 5));
    b.set_blocked(block_kind::clone_sync, 5, true);
    check_true("clone_sync now set", b.is_blocked(block_kind::clone_sync, 5));
    check_true("net_events still set", b.is_blocked(block_kind::net_events, 5));

    b.set_blocked(block_kind::net_events, 6, true);
    b.set_blocked(block_kind::net_events, 5, false);
    check_true("player 5 cleared", !b.is_blocked(block_kind::net_events, 5));
    check_true("player 6 still blocked", b.is_blocked(block_kind::net_events, 6));

    // Boundary indices are real players.
    b.set_blocked(block_kind::net_events, 0, true);
    b.set_blocked(block_kind::net_events, 31, true);
    check_true("index 0 works", b.is_blocked(block_kind::net_events, 0));
    check_true("index 31 works", b.is_blocked(block_kind::net_events, 31));

    // Out-of-range is inert, never undefined. A corrupt player index arriving
    // from the wire must not shift by 32+, and must not report blocked.
    check_true("index 32 not blocked", !b.is_blocked(block_kind::net_events, 32));
    check_true("index -1 not blocked", !b.is_blocked(block_kind::net_events, -1));
    check_true("index 255 not blocked", !b.is_blocked(block_kind::net_events, 255));
    const uint32_t before = b.mask(block_kind::net_events);
    b.set_blocked(block_kind::net_events, 32, true);
    b.set_blocked(block_kind::net_events, -1, true);
    check_true("out-of-range set is ignored", b.mask(block_kind::net_events) == before);

    b.clear();
    check_true("clear resets net_events", b.mask(block_kind::net_events) == 0);
    check_true("clear resets clone_sync", b.mask(block_kind::clone_sync) == 0);
    check_true("clear resets clone_create", b.mask(block_kind::clone_create) == 0);

    player_blocks().set_blocked(block_kind::clone_create, 2, true);
    check_true("global instance works", player_blocks().is_blocked(block_kind::clone_create, 2));
    player_blocks().clear();

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed != 0;
}
