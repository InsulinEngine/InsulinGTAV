// Host unit tests for detour prologue safety analysis.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/protections_branch_check_test.cpp -o build/protections_branch_check_test.exe
//   ./build/protections_branch_check_test.exe
// C headers only: MSVC's C++ stdlib rejects the installed clang (STL1000).
#include "protections/branch_check.h"
#include <stdio.h>
#include <string.h>

static int g_failed = 0;

static void check_true(const char* what, bool ok) {
    if (!ok) { printf("FAIL %s\n", what); g_failed++; }
    else     { printf("ok   %s\n", what); }
}

static void check_eq(const char* what, unsigned got, unsigned want) {
    if (got != want) { printf("FAIL %s: got %u, want %u\n", what, got, want); g_failed++; }
    else             { printf("ok   %s = %u\n", what, got); }
}

int main() {
    using namespace protections;

    // CScriptedGameEvent::Decide @ 0x16C5F10 - Tier 2's highest-value target.
    // push rbp / mov rbp,rsp / push r14 / push rbx / sub rsp,0x1d0
    // Boundaries land on exactly 14, and the RIP-relative load that follows
    // starts at offset 20 - outside the steal.
    const uint8_t decide[] = {
        0x55, 0x48,0x89,0xE5, 0x41,0x56, 0x53, 0x48,0x81,0xEC,0xD0,0x01,0x00,0x00,
        0x48,0x89,0xF1, 0x48,0x89,0xFE, 0x48,0x8B,0x05,0x45,0x17,0xA5,0x01
    };
    prologue_verdict v = check_prologue(decide, sizeof(decide));
    check_true("scripted-game-event Decide is safe", v.safe);
    check_eq("its steal is exactly 14", v.steal_len, 14);

    // rage::fwBasePool::New @ 0x1EF6A00 - Tier 1 identified it and refused to
    // hook it. Byte 10 of the forced 15-byte steal is `jz rel8`, which the stub
    // would copy unrelocated. This is the case the whole check exists for.
    const uint8_t pool_new[] = {
        0x4C,0x8B,0xD1, 0x48,0x63,0x49,0x18, 0x83,0xF9,0xFF, 0x74,0x6E,
        0x33,0xC0, 0xC3
    };
    prologue_verdict p = check_prologue(pool_new, sizeof(pool_new));
    check_true("fwBasePool::New is refused", !p.safe);
    check_true("and the reason names a relative branch",
               p.reason && strstr(p.reason, "branch") != nullptr);

    // A RIP-relative operand INSIDE the steal must also be refused: the stub
    // sits at a different address, so the displacement resolves elsewhere.
    const uint8_t rip_inside[] = {
        0x55, 0x48,0x89,0xE5, 0x48,0x8B,0x05,0x11,0x22,0x33,0x44,
        0x53, 0x41,0x56, 0x90, 0x90
    };
    prologue_verdict r = check_prologue(rip_inside, sizeof(rip_inside));
    check_true("rip-relative inside the steal is refused", !r.safe);
    check_true("and the reason says so",
               r.reason && strstr(r.reason, "rip") != nullptr);

    // A call rel32 is a relative branch too, and a common prologue opener in
    // instrumented builds.
    const uint8_t call_rel[] = {
        0x55, 0x48,0x89,0xE5, 0xE8,0x00,0x01,0x00,0x00, 0x53, 0x41,0x56,
        0x48,0x83,0xEC,0x20
    };
    check_true("call rel32 is refused", !check_prologue(call_rel, sizeof(call_rel)).safe);

    // A 14-byte boundary is fine; a steal that would have to split an
    // instruction must extend to the next whole boundary, not truncate.
    // push rbp / mov rbp,rsp / push r15..rbx (13 bytes) then a 4-byte insn.
    const uint8_t past14[] = {
        0x55, 0x48,0x89,0xE5, 0x41,0x57, 0x41,0x56, 0x41,0x55, 0x41,0x54, 0x53,
        0x49,0x89,0xFE, 0x90, 0x90
    };
    prologue_verdict q = check_prologue(past14, sizeof(past14));
    check_true("a split instruction extends the steal", q.safe);
    check_eq("to 16, not 14", q.steal_len, 16);

    // Too few bytes to reach 14 is a refusal, never a short steal.
    const uint8_t tiny[] = { 0x55, 0x48,0x89,0xE5, 0xC3 };
    check_true("a function shorter than the jump is refused",
               !check_prologue(tiny, sizeof(tiny)).safe);

    // An opcode the decoder does not know must refuse rather than guess.
    const uint8_t unknown[] = {
        0x55, 0x48,0x89,0xE5, 0x0F,0x0B, 0x53, 0x41,0x56, 0x48,0x83,0xEC,0x20, 0x90
    };
    prologue_verdict u = check_prologue(unknown, sizeof(unknown));
    if (!u.safe) check_true("an unknown opcode refuses", true);
    else         check_true("an unknown opcode refuses", false);

    // Null and zero-length are refusals, not crashes.
    check_true("null is refused", !check_prologue(nullptr, 32).safe);
    check_true("zero length is refused", !check_prologue(decide, 0).safe);

    // Every refusal carries a reason - the log line is the whole point.
    check_true("refusals always explain themselves",
               p.reason && r.reason && p.reason[0] && r.reason[0]);

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed != 0;
}
