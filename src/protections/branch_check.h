#pragma once
#include <stdint.h>

// Is a function's prologue safe to detour?
//
// GoldHEN's Detour_GetInstructionSize accumulates whole instructions until it
// reaches >= 14 (the size of its absolute jump), then memcpy's that range into
// the trampoline WITHOUT relocating operands (Detour.c:125-126). Two things in
// that range therefore break:
//
//   * a relative branch (jcc rel8/rel32, jmp rel8/rel32, call rel32) - the
//     copied displacement is relative to the stub, not the original;
//   * a RIP-relative memory operand - same problem, different encoding.
//
// Tier 1 found this the expensive way: rage::fwBasePool::New was correctly
// identified and then deliberately not hooked, because byte 10 of its forced
// 15-byte steal is a `jz rel8` that would have broken the pool-empty path -
// the exact path its guard existed to watch.
//
// This decoder is deliberately narrow. It knows the instruction forms that
// actually appear in function prologues and REFUSES on anything else: an
// unrecognised prologue is precisely when a guess is worst.
//
// ---------------------------------------------------------------------------
// Agreement with the SDK
//
// The thing that actually picks the steal length at runtime is hde64
// (C:\PS4\GoldHEN_Plugins_SDK\source\HDE64.c), driven by
// Detour_GetInstructionSize (Detour.c:26-41):
//
//     while (InstructionSize < MinSize) {
//         uint32_t temp = hde64_disasm(Address + InstructionSize, &hs);
//         if (hs.flags & F_ERROR) return 0;
//         InstructionSize += temp;
//     }
//
// So this file has to walk boundaries the same way hde64 does, or its verdict
// would be about a different byte range than the one that gets copied. What was
// taken from hde64, verbatim in behaviour:
//
//   * prefix bytes are consumed first (HDE64.c:20-52), then at most one REX;
//     a second byte in 0x40..0x4F right after a REX is an error (HDE64.c:65-68);
//   * disp sizing off ModRM.mod - mod=0 && rm=5 is disp32 (and, in long mode,
//     RIP-relative); mod=1 is disp8; mod=2 is disp32 (HDE64.c:230-243);
//   * a SIB byte whenever mod != 3 && rm == 4, and base==5 with mod even
//     forces disp32 (HDE64.c:245-252, the base==5 rule on :251);
//   * 0x66 shrinks an "iz" immediate to 2 bytes, including the rel32 of
//     E8/E9 (HDE64.c:272-292);
//   * REX.W turns B8..BF into a 8-byte movabs immediate (HDE64.c:61);
//   * F6/F7 carry an immediate only when ModRM.reg <= 1 (HDE64.c:223-228).
//
// What was deliberately NOT implemented, because refusing is always safe:
//
//   * hde64's opcode/group/lock/FPU validity tables. This file whitelists
//     opcodes instead; anything outside the whitelist refuses. That refusal is
//     the one a whitelist addition can fix, and it says so.
//   * the 0x67 address-size prefix. hde64 decodes a 0x67-prefixed memory
//     operand with 16-bit addressing rules (rm==6 => disp16, HDE64.c:232-233),
//     which is wrong in long mode: measured against the SDK's own binary,
//     `67 48 8B 05 <rel32>` comes back as length 4 for an 8-byte RIP-relative
//     load, with NO error flag. A silently wrong boundary on exactly the
//     operand class this check exists to catch. Refused - and note that no
//     whitelist addition can ever make such a target hookable, which is why it
//     refuses with its own reason string.
//   * the A0..A3 moffs forms, and VEX/EVEX (0xC4/0xC5/0x62), which hde64 does
//     not understand at all. All refused.
//   * 0xF0 LOCK - and this one is a gap in THIS decoder, not in the SDK.
//     hde64 carries a real lock-validity table (HDE64.c:127-150 walking
//     Table64.h's DELTA_OP_LOCK_OK / DELTA_OP2_LOCK_OK) and measures locked
//     forms correctly when the ModRM.reg field is permitted: one-byte
//     00/08/10/18/20/28/30 (add/or/adc/sbb/and/sub/xor, both the Eb,Gb and
//     Ev,Gv encodings, since the compare is on `op & -2`), 80/82 (grp1, /0../6),
//     86 (xchg), F6 (/2,/3), FE (/0,/1); and two-byte AB, B0, B1, B3, BA (/5../7),
//     BB, C0, C1, C7 (/1). Measured directly against the SDK: `lock add
//     [rdi],eax` = 3 bytes, `lock xadd [rdi],eax` = 4, `lock cmpxchg` = 5, all
//     with no error flag; only the mod=3 form errors, and that form is
//     architecturally invalid anyway. So a LOCK-prefixed target may well be
//     hookable, and its refusal says so - closing the gap means porting those
//     tables, which is real work but not impossible.
//
// Scope: detect and refuse. This does NOT relocate anything. Relocating a
// branch into the trampoline would unblock more targets and is recorded as a
// future option in analysis/PROTECTIONS_ANCHORS.md; it is a much larger change.
namespace protections {

    struct prologue_verdict {
        bool        safe;       // false => do not detour this function
        uint32_t    steal_len;  // whole-instruction length >= 14 when safe
        const char* reason;     // why it was refused; nullptr when safe
    };

    namespace branch_check_detail {

        // sizeof(Detour::JumpInstructions64) - the absolute jump GoldHEN writes
        // over the target, and therefore the minimum it must steal.
        enum { k_jump_bytes = 14 };

        // Immediate/displacement forms, named after the Intel operand codes.
        enum imm_kind {
            imm_none = 0,
            imm_ib,     // 1 byte
            imm_iw,     // 2 bytes
            imm_iz,     // 4 bytes, or 2 under a 0x66 prefix
            imm_io,     // 8 bytes under REX.W, else iz (movabs B8..BF only)
            rel_b,      // rel8  - position dependent
            rel_z       // rel32 - position dependent (2 under 0x66, as hde64 does)
        };

        struct opform {
            bool     known;
            bool     modrm;
            imm_kind imm;
            bool     terminal;  // control leaves the function here (ret/int3)
        };

        inline opform make_form(bool modrm, imm_kind imm, bool terminal = false) {
            opform f;
            f.known    = true;
            f.modrm    = modrm;
            f.imm      = imm;
            f.terminal = terminal;
            return f;
        }

        inline opform unknown_form() {
            opform f;
            f.known = false; f.modrm = false; f.imm = imm_none; f.terminal = false;
            return f;
        }

        // One-byte opcode map, restricted to forms that show up in real
        // prologues (and the branch/terminator forms we must recognise in order
        // to refuse them by name rather than as "unknown").
        inline opform classify_1byte(uint8_t op) {
            // ADD/OR/ADC/SBB/AND/SUB/XOR/CMP: 0x00..0x3D, low three bits pick
            // the form. Low bits 6 and 7 are the segment ops / prefixes / the
            // 0x0F escape, none of which reach here.
            if ((op & 0xC0) == 0x00 && (op & 0x07) <= 5) {
                switch (op & 0x07) {
                    case 0: case 1: case 2: case 3: return make_form(true,  imm_none);
                    case 4:                         return make_form(false, imm_ib);
                    default:                        return make_form(false, imm_iz);
                }
            }
            if (op >= 0x50 && op <= 0x5F) return make_form(false, imm_none);  // push/pop r64
            if (op >= 0x70 && op <= 0x7F) return make_form(false, rel_b);     // jcc rel8
            if (op >= 0x88 && op <= 0x8B) return make_form(true,  imm_none);  // mov
            if (op >= 0x90 && op <= 0x97) return make_form(false, imm_none);  // nop / xchg rAX
            // String ops. `rep stosq` is real prologue material: clang emits it
            // to zero a large stack buffer right after the frame is set up.
            if (op >= 0xA4 && op <= 0xA7) return make_form(false, imm_none);  // movs / cmps
            if (op == 0xA8)               return make_form(false, imm_ib);    // test al, ib
            if (op == 0xA9)               return make_form(false, imm_iz);    // test eAX, iz
            if (op >= 0xAA && op <= 0xAF) return make_form(false, imm_none);  // stos / lods / scas
            if (op >= 0xB0 && op <= 0xB7) return make_form(false, imm_ib);    // mov r8, ib
            if (op >= 0xB8 && op <= 0xBF) return make_form(false, imm_io);    // mov r32/r64, iz/io
            if (op >= 0xE0 && op <= 0xE3) return make_form(false, rel_b);     // loop / jrcxz

            switch (op) {
                case 0x63: return make_form(true,  imm_none);            // movsxd
                case 0x68: return make_form(false, imm_iz);              // push iz
                case 0x69: return make_form(true,  imm_iz);              // imul r,rm,iz
                case 0x6A: return make_form(false, imm_ib);              // push ib
                case 0x6B: return make_form(true,  imm_ib);              // imul r,rm,ib
                case 0x80: return make_form(true,  imm_ib);              // grp1 Eb, ib
                case 0x81: return make_form(true,  imm_iz);              // grp1 Ev, iz
                case 0x83: return make_form(true,  imm_ib);              // grp1 Ev, ib
                case 0x84: case 0x85: return make_form(true, imm_none);  // test
                case 0x86: case 0x87: return make_form(true, imm_none);  // xchg
                case 0x8D: return make_form(true,  imm_none);            // lea
                case 0x8F: return make_form(true,  imm_none);            // pop Ev
                case 0x98: case 0x99: return make_form(false, imm_none); // cdqe / cqo
                case 0x9C: case 0x9D: return make_form(false, imm_none); // pushfq / popfq
                case 0xC0: case 0xC1: return make_form(true, imm_ib);    // grp2 Ev, ib
                case 0xC2: return make_form(false, imm_iw, true);        // ret imm16
                case 0xC3: return make_form(false, imm_none, true);      // ret
                case 0xC6: return make_form(true,  imm_ib);              // mov Eb, ib
                case 0xC7: return make_form(true,  imm_iz);              // mov Ev, iz
                case 0xC9: return make_form(false, imm_none);            // leave
                case 0xCC: return make_form(false, imm_none, true);      // int3 (padding)
                case 0xD0: case 0xD1: case 0xD2: case 0xD3:
                           return make_form(true,  imm_none);            // grp2 Ev, 1/cl
                case 0xE8: return make_form(false, rel_z);               // call rel32
                case 0xE9: return make_form(false, rel_z);               // jmp rel32
                case 0xEB: return make_form(false, rel_b);               // jmp rel8
                case 0xF5: return make_form(false, imm_none);            // cmc
                case 0xF6: return make_form(true,  imm_none);            // grp3 Eb (imm added below)
                case 0xF7: return make_form(true,  imm_none);            // grp3 Ev (imm added below)
                case 0xF8: case 0xF9: case 0xFC: case 0xFD:
                           return make_form(false, imm_none);            // clc/stc/cld/std
                case 0xFE: return make_form(true,  imm_none);            // grp4 inc/dec Eb
                case 0xFF: return make_form(true,  imm_none);            // grp5 (terminal below)
                default:   return unknown_form();
            }
        }

        // Two-byte (0x0F xx) map. Only ModRM-shaped forms plus the rel32 jcc
        // block; anything else - 0x0F 0x0B (ud2) included - refuses.
        inline opform classify_2byte(uint8_t op) {
            if (op >= 0x80 && op <= 0x8F) return make_form(false, rel_z);   // jcc rel32
            if (op >= 0x90 && op <= 0x9F) return make_form(true, imm_none); // setcc
            if (op >= 0x40 && op <= 0x4F) return make_form(true, imm_none); // cmovcc
            if (op >= 0x10 && op <= 0x17) return make_form(true, imm_none); // movups/movlps/...
            if (op == 0x1E || op == 0x1F) return make_form(true, imm_none); // endbr64 / multi-nop
            if (op >= 0x28 && op <= 0x2F) return make_form(true, imm_none); // movaps/cvt*/ucomis*
            if (op >= 0x51 && op <= 0x5F) return make_form(true, imm_none); // sqrt/and/or/xor/add/...
            if (op >= 0x60 && op <= 0x6F) return make_form(true, imm_none); // punpck*/movd/movdqa
            if (op >= 0x70 && op <= 0x73) return make_form(true, imm_ib);   // pshufd / psrl-psll grp
            if (op >= 0x74 && op <= 0x76) return make_form(true, imm_none); // pcmpeq*
            if (op == 0x7E || op == 0x7F) return make_form(true, imm_none); // movd/movq/movdqa store
            if (op >= 0xD0 && op <= 0xDF) return make_form(true, imm_none); // psrl*/movq/pand/...
            if (op >= 0xE0 && op <= 0xEF) return make_form(true, imm_none); // pavg*/psra*/pxor/...
            // 0xF6 and 0xF7 are deliberately absent from this range. hde64
            // applies its group-3 immediate patch on the SECOND opcode byte
            // without checking opcode2 (HDE64.c:223-228), so for `0F F6 /0..1`
            // and `0F F7 /0..1` it reports one byte too many. That length is not
            // an instruction boundary at all, so a steal ending there would
            // leave the stub's jump-back pointing into the middle of the next
            // instruction. Refuse instead of matching a boundary that is wrong
            // either way. Verified against the SDK's own hde64.
            if (op >= 0xF1 && op <= 0xF5) return make_form(true, imm_none); // psll*/psub*
            if (op >= 0xF8 && op <= 0xFE) return make_form(true, imm_none); // psub*/padd*

            switch (op) {
                case 0x05: return make_form(false, imm_none);            // syscall
                case 0x0D: return make_form(true,  imm_none);            // prefetch
                case 0x18: case 0x19: case 0x1A: case 0x1B:
                case 0x1C: case 0x1D:
                           return make_form(true,  imm_none);            // hint nop
                case 0x31: return make_form(false, imm_none);            // rdtsc
                case 0xA2: return make_form(false, imm_none);            // cpuid
                case 0xA3: case 0xAB: case 0xB3: case 0xBB:
                           return make_form(true,  imm_none);            // bt/bts/btr/btc
                case 0xA4: case 0xAC: return make_form(true, imm_ib);    // shld/shrd, ib
                case 0xA5: case 0xAD: return make_form(true, imm_none);  // shld/shrd, cl
                case 0xAF: return make_form(true,  imm_none);            // imul r, rm
                case 0xB0: case 0xB1: return make_form(true, imm_none);  // cmpxchg
                case 0xB6: case 0xB7: return make_form(true, imm_none);  // movzx
                case 0xBA: return make_form(true,  imm_ib);              // grp8 bt/bts/btr/btc, ib
                case 0xBC: case 0xBD: return make_form(true, imm_none);  // bsf/bsr (tzcnt/lzcnt)
                case 0xBE: case 0xBF: return make_form(true, imm_none);  // movsx
                case 0xC0: case 0xC1: return make_form(true, imm_none);  // xadd
                case 0xC2: return make_form(true,  imm_ib);              // cmpps/cmpss, ib
                case 0xC3: return make_form(true,  imm_none);            // movnti
                case 0xC4: case 0xC5: return make_form(true, imm_ib);    // pinsrw / pextrw
                case 0xC6: return make_form(true,  imm_ib);              // shufps, ib

                // Watch-list: two-byte forms that are absent today and could
                // plausibly cost a future target a false refusal. Start here
                // rather than rediscovering the list. Each needs the same
                // treatment - add it, then re-run the hde64 differential.
                //
                //   0F AE      grp15: ldmxcsr/stmxcsr/fxsave/lfence/mfence
                //              (mixed mem and reg-only forms - check both)
                //   0F C7      grp9: cmpxchg8b/16b, rdrand, rdseed
                //   0F C8..CF  bswap r32/r64 - register-encoded, NO ModRM
                //   0F B8      popcnt (F3-prefixed)
                //   0F 50      movmskps/movmskpd
                //
                // And one one-byte form: 0xF0 LOCK, which gets its own
                // refusal class because it is neither a one-line whitelist
                // entry nor an engine limit - hde64 measures locked forms
                // correctly, so closing that gap means porting its
                // lock-validity tables. See the file header.
                default:   return unknown_form();
            }
        }

        // Why a decode was refused. These are mutually exclusive by
        // construction, and they are kept apart because each one tells whoever
        // reads the log to do something DIFFERENT:
        //
        //   why_unknown_opcode - an opcode outside this file's whitelist. The
        //     cheap case: add it to classify_1byte/classify_2byte and re-run the
        //     hde64 differential.
        //   why_lock - a 0xF0 LOCK prefix. NOT an engine limit: hde64 measures
        //     locked forms correctly whenever its lock-validity table permits
        //     the ModRM.reg field (see the file header for the measured list),
        //     so such a target may well be hookable. Closing this gap means
        //     porting those tables - real work, but possible.
        //   why_engine_limit - hde64 itself cannot measure the encoding at all:
        //     a 0x67 prefix (silently wrong length), a double REX, or a form
        //     over 15 bytes. Detour_GetInstructionSize returns 0 or, worse, a
        //     wrong boundary. No work in THIS file changes that.
        //   why_truncated - ran out of inspection window mid-instruction.
        //     Unreachable from install_detour, which always passes 32 bytes
        //     (the longest possible steal is 13 + 15 = 28), but reachable from
        //     a host test with a short buffer.
        enum refusal {
            why_none = 0,
            why_unknown_opcode,
            why_lock,
            why_engine_limit,
            why_truncated
        };

        struct insn {
            uint32_t len;
            bool     ok;        // false => not recognised; caller must refuse
            bool     relative;  // rel8/rel32 operand - position dependent
            bool     riprel;    // mod=0, rm=5 - position dependent
            bool     terminal;  // ret / int3 / indirect jmp: the function ends
            refusal  why;       // set iff ok == false
        };

        // Decode exactly one instruction at `b`, reading at most `avail` bytes.
        // Never reads past `avail`: a form that would need more bytes than are
        // available comes back with ok == false.
        inline insn decode_one(const uint8_t* b, uint32_t avail) {
            insn in;
            in.len = 0; in.ok = false; in.relative = false;
            in.riprel = false; in.terminal = false;
            in.why = why_unknown_opcode;

            uint32_t i = 0;
            bool p66 = false;

            // Legacy prefixes. 0x67 and 0xF0 are refused on purpose - see the
            // header comment. Five is already more than any real prologue byte
            // sequence carries (x86 allows at most one per prefix group, and
            // there are four groups); beyond that, refuse rather than keep
            // walking.
            for (uint32_t n = 0; n < 5; ++n) {
                if (i >= avail) { in.why = why_truncated; return in; }
                const uint8_t c = b[i];
                if (c == 0x66) { p66 = true; ++i; continue; }
                if (c == 0xF2 || c == 0xF3) { ++i; continue; }
                if (c == 0x2E || c == 0x36 || c == 0x3E || c == 0x26 ||
                    c == 0x64 || c == 0x65) { ++i; continue; }
                break;
            }
            if (i >= avail) { in.why = why_truncated; return in; }
            // LOCK and 0x67 both stop us here, for opposite reasons: hde64
            // handles LOCK properly and we do not, while hde64 handles 0x67
            // improperly and nobody can. Keep them apart.
            if (b[i] == 0xF0) { in.why = why_lock; return in; }
            if (b[i] == 0x67) { in.why = why_engine_limit; return in; }
            if (b[i] == 0x66 || b[i] == 0xF2 || b[i] == 0xF3) {
                // More prefixes than the loop above allows. hde64 would keep
                // consuming (up to 16), so this is our limit, not the engine's -
                // but no compiler emits it, so it stays an ordinary refusal.
                in.why = why_unknown_opcode;
                return in;
            }

            bool rexw = false;
            if ((b[i] & 0xF0) == 0x40) {
                rexw = (b[i] & 0x08) != 0;
                ++i;
                if (i >= avail) { in.why = why_truncated; return in; }
                // hde64 raises F_ERROR on a second REX-range byte
                // (HDE64.c:65-68), so the SDK cannot measure it either.
                if ((b[i] & 0xF0) == 0x40) { in.why = why_engine_limit; return in; }
            }

            if (i >= avail) { in.why = why_truncated; return in; }
            uint8_t op = b[i++];

            bool two = false;
            if (op == 0x0F) {
                if (i >= avail) { in.why = why_truncated; return in; }
                op = b[i++];
                two = true;
            }

            opform f = two ? classify_2byte(op) : classify_1byte(op);
            if (!f.known) return in;

            imm_kind imm = f.imm;

            if (f.modrm) {
                if (i >= avail) { in.why = why_truncated; return in; }
                const uint8_t modrm = b[i++];
                const uint8_t mod   = (uint8_t)(modrm >> 6);
                const uint8_t reg   = (uint8_t)((modrm >> 3) & 7);
                const uint8_t rm    = (uint8_t)(modrm & 7);

                // hde64 patches the group-3 immediate in from ModRM.reg
                // (HDE64.c:223-228).
                if (!two && reg <= 1) {
                    if (op == 0xF6)      imm = imm_ib;
                    else if (op == 0xF7) imm = imm_iz;
                }
                // grp5 /4 and /5 are jmp far/near through memory: control does
                // not come back, so the rest of the "prologue" is not code we
                // can keep stealing.
                if (!two && op == 0xFF && (reg == 4 || reg == 5))
                    in.terminal = true;

                uint32_t disp = 0;
                if (mod == 0) {
                    if (rm == 5) { disp = 4; in.riprel = true; }
                } else if (mod == 1) {
                    disp = 1;
                } else if (mod == 2) {
                    disp = 4;
                }

                if (mod != 3 && rm == 4) {
                    if (i >= avail) { in.why = why_truncated; return in; }
                    const uint8_t sib  = b[i++];
                    const uint8_t base = (uint8_t)(sib & 7);
                    // hde64: base==5 with an even mod forces disp32
                    // (HDE64.c:251). That form is absolute-with-index, not
                    // RIP-relative, so it needs no relocation.
                    if (base == 5 && (mod & 1) == 0) disp = 4;
                }

                if (disp > avail - i) { in.why = why_truncated; return in; }
                i += disp;
            }

            uint32_t immbytes = 0;
            switch (imm) {
                case imm_none: immbytes = 0; break;
                case imm_ib:   immbytes = 1; break;
                case imm_iw:   immbytes = 2; break;
                case imm_iz:   immbytes = p66 ? 2u : 4u; break;
                case imm_io:   immbytes = rexw ? 8u : (p66 ? 2u : 4u); break;
                case rel_b:    immbytes = 1; in.relative = true; break;
                case rel_z:    immbytes = p66 ? 2u : 4u; in.relative = true; break;
            }
            if (immbytes > avail - i) { in.why = why_truncated; return in; }
            i += immbytes;

            // hde64 flags anything past 15 bytes as F_ERROR_LENGTH and clamps
            // (HDE64.c:317-320), so Detour_GetInstructionSize returns 0 and the
            // SDK cannot measure it at all. Refuse rather than model the clamp.
            if (i == 0 || i > 15) { in.why = why_engine_limit; return in; }

            in.len = i;
            in.ok  = true;
            in.why = why_none;
            if (f.terminal) in.terminal = true;
            return in;
        }
    }

    // Walks `bytes` the way Detour_GetInstructionSize walks the live target,
    // accumulating whole instructions until the total reaches 14, and refuses
    // if anything in that range would be copied into the stub unrelocated.
    inline prologue_verdict check_prologue(const uint8_t* bytes, uint32_t len) {
        using namespace branch_check_detail;

        prologue_verdict v;
        v.safe = false; v.steal_len = 0; v.reason = "unexamined";

        if (!bytes) { v.reason = "null target address"; return v; }
        if (len == 0) { v.reason = "no bytes to inspect"; return v; }
        if (len < (uint32_t)k_jump_bytes) {
            v.reason = "fewer bytes available than the 14-byte jump needs";
            return v;
        }

        uint32_t off = 0;
        while (off < (uint32_t)k_jump_bytes) {
            const insn in = decode_one(bytes + off, len - off);
            if (!in.ok) {
                // Four refusals, because each one calls for a different
                // response from whoever reads the log.
                switch (in.why) {
                case why_engine_limit:
                    v.reason = "prologue uses an encoding the SDK's own decoder "
                               "cannot measure (0x67 address-size prefix, "
                               "double REX, or an over-15-byte form) - this "
                               "target is not hookable by this detour engine, "
                               "and no work in branch_check.h changes that";
                    break;
                case why_lock:
                    v.reason = "prologue carries a lock prefix, whose validity "
                               "tables this decoder does not model - hde64 "
                               "measures many locked forms correctly, so the "
                               "target may well be hookable; closing this gap "
                               "means porting HDE64.c's lock tables, not adding "
                               "an opcode";
                    break;
                case why_truncated:
                    v.reason = "prologue runs past the inspected window before "
                               "a whole-instruction boundary reaches 14";
                    break;
                default:
                    v.reason = "unrecognised instruction in the prologue - "
                               "refusing to guess where the steal ends";
                    break;
                }
                return v;
            }
            if (in.relative) {
                v.reason = "relative branch inside the stolen prologue - "
                           "the stub would copy it unrelocated";
                return v;
            }
            if (in.riprel) {
                v.reason = "rip-relative operand inside the stolen prologue - "
                           "the stub would copy it unrelocated";
                return v;
            }
            off += in.len;
            if (in.terminal && off < (uint32_t)k_jump_bytes) {
                v.reason = "function ends before the 14-byte jump fits - "
                           "the patch would run past its last instruction";
                return v;
            }
        }

        v.safe      = true;
        v.steal_len = off;
        v.reason    = nullptr;
        return v;
    }
}
