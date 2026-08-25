#include "protections/detour.h"
#include "protections/registry.h"
#include "protections/report.h"
#include "rage/invoker/invoker.h"

// Counter guards. Both targets watch a global count rather than a pointer
// chain: the crash is an unbounded append past the end of a fixed array.
//
// Evidence lives in analysis/PROTECTIONS_ANCHORS.md in the IDA repo
// (E:\Projects\IDA\PS4\GTA5), section 11. Both targets were identified from the
// leaked dev_ng rage source plus the PS4 disassembly; every constant below is
// read out of the PS4 binary, none is ported from the PC build.
//
// Only ONE of the two guards is installed.
//
// `pool_exhaustion` - rage::fwBasePool::New() - is identified with certainty at
// RVA 0x1EF6A00 (PC twin 0x7FF6FF760AD8, which is what YimMenu's "CPI" signature
// resolves to, and which the IDB's own PC->PS4 bridge already maps to 0x1EF6A00).
// It is deliberately NOT hooked, because it cannot be detoured safely with the
// primitive this plugin has:
//
//   1ef6a00  48 63 4f 18   movsxd rcx, [rdi+18h]   ; 4   off 0
//   1ef6a04  31 c0         xor    eax, eax         ; 2   off 4
//   1ef6a06  48 83 f9 ff   cmp    rcx, -1          ; 4   off 6
//   1ef6a0a  74 6e         jz     locret_1EF6A7A   ; 2   off 10  <- rel8 BRANCH
//   1ef6a0c  8b 47 14      mov    eax, [rdi+14h]   ; 3   off 12
//
// Detour_GetInstructionSize accumulates whole instructions until the total
// reaches 14, so the first acceptable boundary is 15 - and 15 bytes include the
// `jz rel8` at offset 10. Detour_DetourFunction64 memcpy's the stolen range into
// the stub verbatim and relocates nothing, so in the stub that branch would
// resolve to StubPtr+12+0x6E: past the end of a 29-byte stub. The path it
// breaks is the pool-empty path - precisely the one this guard exists to
// observe - so the guard would convert the degradation it is meant to report
// into a jump into garbage. Not hooked; `install` stays nullptr in the registry.
// See the task report for the two routes that could still crack it.
//
// ---------------------------------------------------------------------------
// REPORT LEGEND for this file. The drain prints `a=%08x b=%08x` with no key.
//
//   Skeleton Extension   a = the m_Count that was read (>= 32, or negative,
//                            which can only come from a stomp on the dword)
//                        b = 0 (unused)
//
//   Pool Exhaustion      never reports - no detour, see above.
// ---------------------------------------------------------------------------
namespace protections {
namespace {
    // eboot RVA, CUSA00411 v1.57, imagebase 0.
    // rage::fwAltSkeletonExtension::GetOrAddExtension(fwEntity&) @ 0x1E27300.
    // ABI: void* f(rdi = fwEntity*); returns the extension, and the one caller
    // (sub_8C4E80 @ 0x8C4FBD) already tests the result for null, so returning
    // null is a path retail handles.
    const uint64_t RVA_ADD_SKELETON_EXTENSION = 0x1E27300;

    // The array is `atFixedArray<fwAltSkeletonExtension, N>` living at
    // 0x3E10750; atFixedArray places its `int m_Count` immediately after the
    // elements, and the function's own append reads it at base+0xA00 with a
    // stride of 80:
    //
    //   1e27367  lea    r15, byte_3E10750
    //   1e2736e  movsxd rax, dword ptr [r15+0A00h]   ; m_Count
    //   1e27375  lea    ecx, [rax+1]
    //   1e27378  mov    [r15+0A00h], ecx             ; m_Count++  <- NO bound check
    //   1e2737f  lea    r12, [rax+rax*4]             ; *5
    //   1e27383  shl    r12, 4                       ; *16  => stride 80
    //   1e27387  mov    [r15+r12+8], r14             ; m_entity
    //
    // 0xA00 / 80 = 32, so the capacity is 32 - taken from the binary's own
    // array/count layout, the same way the draw-list 512 came off the game's own
    // `cmp eax, 200h`. It agrees with MAX_ALT_SKELETON_EXTENSIONS = 32 in the
    // leaked rage header and with the PC build's layout (count - base = 0xA00
    // there too), but the number used here is the measured one.
    const uint64_t RVA_SKELETON_EXT_COUNT = 0x3E11150;   // 0x3E10750 + 0xA00
    const int      SKELETON_EXT_CAPACITY  = 32;

    typedef void* (*add_skeleton_extension_fn)(void*);

    detour_slot g_skeleton;

    // rage::fwAltSkeletonExtension::GetOrAddExtension(fwEntity& entity)
    //
    //   ext = entity.GetExtension<fwAltSkeletonExtension>();   // list walk
    //   if (!ext) {
    //       n = ms_AltSkeletonExtensions.m_Count;
    //       ms_AltSkeletonExtensions.m_Count = n + 1;           // <- unchecked
    //       ext = &ms_AltSkeletonExtensions[n];                 // base + 80*n
    //       ext->m_entity = &entity;
    //       ext->m_offset = Mat34V(V_IDENTITY);
    //       entity.GetExtensionList().Add(*ext);
    //   }
    //   return ext;
    //
    // What retail leaves unchecked: the count. `atFixedArray::Append()` guards
    // itself with TrapGE(m_Count, _MaxCount), and that trap is compiled out of
    // this build - the disassembly above increments and stores with no compare
    // at all. At n == 32 the entity pointer is written to base+0xA08
    // (0x3E11158) and the 64-byte identity matrix to base+0xA10..0xA4F
    // (0x3E11160..0x3E1119F), i.e. straight over the .bss that follows the
    // count. The immediate victim is `unk_3E11168`, another extension class's
    // object, which takes 56 of those 64 bytes; the leading 8 land in whatever
    // .bss precedes it. The other class's autoid at `dword_3E11214` is NOT hit
    // here - the count has to reach 34 for that, and 35 to land inside its
    // fixed array at `unk_3E11240`. Every further call walks 80 bytes further
    // out. (Full per-count blast-radius table in section 11 of the anchors
    // document; naming the row-34 victim as the row-32 victim is a mistake that
    // document records having made.)
    //
    // The refusal is safe because the sole caller already handles it:
    //   8c4fbd  call sub_1E27300
    //   8c4fc5  test rax, rax / jz  <skip the matrix update>
    // so a null return skips one first-person attachment offset update. That is
    // the same trade YimMenu makes, and it is the only bounded outcome available
    // once the array is full.
    //
    // A negative count is not a legal state either - it can only come from a
    // stomp on the count dword - so it is reported and refused as well.
    void* add_skeleton_extension_hook(void* entity) {
        if (should_report(filter_id::skeleton_extension)) {
            const int n = *(int*)(rage::invoker::g_eboot_base + RVA_SKELETON_EXT_COUNT);
            if (n >= SKELETON_EXT_CAPACITY || n < 0) {
                report(filter_id::skeleton_extension, -1, 0, (uint32_t)n, 0);
                if (should_block(filter_id::skeleton_extension))
                    return nullptr;
            }
        }
        return PROT_CHAIN(g_skeleton, add_skeleton_extension_fn, entity);
    }
}

bool install_skeleton_extension() {
    return install_detour(&g_skeleton, RVA_ADD_SKELETON_EXTENSION,
                          (void*)&add_skeleton_extension_hook);
}
}
