#include "protections/detour.h"
#include "protections/registry.h"
#include "protections/report.h"

// Network-message allocator exhaustion - DETECTION ONLY.
//
// Evidence lives in analysis/PROTECTIONS_ANCHORS.md in the IDA repo
// (E:\Projects\IDA\PS4\GTA5), section 13. Every offset below is read out of the
// PS4 disassembly; nothing here is ported from the PC build on faith.
//
// This filter is staged deliberately. The full recovery walks two live message
// queues and frees unacked reliables on a network thread, which is the most
// dangerous thing anywhere in Tier 1. It is not written until the detection has
// been seen to fire on a real session - a recovery with no observed problem to
// solve would be speculative work in the worst possible place. See the task
// report for the route the recovery would take if it is ever justified.
//
// Because there is nothing to block, this filter never calls should_block() and
// never alters the return value. Enforce is meaningless for it today; the mode
// only decides whether the occurrence is reported.
namespace protections {
namespace {
    // eboot RVA, CUSA00411 v1.57, imagebase 0.
    // rage::netConnection::AllocCritical(unsigned size) @ 0x1A541E0.
    //
    // ABI: void* f(rdi = netConnection*, esi = unsigned size).
    //
    //   1a541f1  mov    rdi, [rbx+18h]        ; m_Allocator
    //   1a541f5  mov    rax, [rdi]            ; sysMemAllocator vtable
    //   1a54202  call   qword ptr [rax+48h]   ; Allocate(size, 0, 0)
    //   1a54205  test   rax, rax
    //   1a54208  jnz    <return>
    //   1a5420e  lock inc dword ptr [rbx+0D4h]   ; m_NumFailedAllocs++
    //   1a5421c  mov    rdi, [rbx+0B0h]       ; m_CxnMgr
    //   1a54228  call   sub_1A54330           ; netConnectionManager::ReclaimMem
    //   1a54232  call   sub_1A53C10           ; ...or netConnection::ReclaimMem
    //   1a54245  call   qword ptr [rax+48h]   ; retry Allocate
    //   1a5424b  jnz    <return>
    //   1a5424d  lock inc dword ptr [r12]     ; m_NumFailedAllocs++ again
    //   1a542a0  call   rax                   ; m_OutOfMemoryCallback(ep, 1, size)
    //
    // The original therefore returns null only after reclaim-and-retry has
    // already failed AND retail has already fired its own fatal out-of-memory
    // callback for this endpoint. A report here is a post-mortem, not a warning:
    // it means the network heap is genuinely dry.
    const uint64_t RVA_ALLOC_CRITICAL = 0x1A541E0;

    // netConnection::m_NumFailedAllocs. Read twice out of the binary: once from
    // the two `lock inc dword ptr [rbx+0D4h]` in AllocCritical itself, and once
    // from netConnection::AllocOutFrame @ 0x1A51500, which inlines the
    // non-critical Alloc() and bumps the same field at 0x1A51610/0x1A5161A.
    const uint64_t OFF_NUM_FAILED_ALLOCS = 0xD4;

    typedef void* (*alloc_critical_fn)(void*, unsigned int);

    detour_slot g_alloc;

    // Chaining first is mandatory here, not a style choice: the failure is only
    // visible in the original's return value. There is no predicate to evaluate
    // beforehand.
    //
    // What the hook is allowed to do afterwards is narrow. It reads one 32-bit
    // field of the connection it was handed and calls report(), which is the one
    // network-thread-safe call in this codebase. It deliberately does NOT call
    // sysMemAllocator::GetMemoryAvailable() to fill in the "space available"
    // half of the report, even though the vtable slot is known: that is a call
    // into the game from a network thread and it walks a free list, which is not
    // proven side-effect-free. m_NumFailedAllocs is the safe substitute and is
    // arguably the more useful number - it says whether this is the first
    // failure on this connection or the hundredth.
    void* alloc_critical_hook(void* cxn, unsigned int size) {
        void* mem = PROT_CHAIN(g_alloc, alloc_critical_fn, cxn, size);

        if (!mem && should_report(filter_id::reliable_alloc)) {
            uint32_t failed = 0;
            if (cxn)
                failed = *(const volatile uint32_t*)((const char*)cxn + OFF_NUM_FAILED_ALLOCS);
            report(filter_id::reliable_alloc, -1, 0, (uint32_t)size, failed);
        }

        return mem;
    }
}

// Detour safety, measured against GoldHEN's Detour_Construct(DetourMode_x64),
// which writes a 14-byte jump and memcpy's the stolen whole instructions into
// the stub without relocating anything:
//
//   1a541e0  55           push rbp        ; 1   cumulative  1
//   1a541e1  48 89 e5     mov  rbp, rsp   ; 3   cumulative  4
//   1a541e4  41 57        push r15        ; 2   cumulative  6
//   1a541e6  41 56        push r14        ; 2   cumulative  8
//   1a541e8  41 54        push r12        ; 2   cumulative 10
//   1a541ea  53           push rbx        ; 1   cumulative 11
//   1a541eb  41 89 f6     mov  r14d, esi  ; 3   cumulative 14  <- exact boundary
//
// 14 bytes exactly, no padding needed. No RIP-relative operand, no relative
// branch of any kind (this is the trap that made rage::fwBasePool::New
// unhookable in section 12), and the entry basic block has no predecessors -
// IDA's flow chart gives 0x1A541E0-0x1A5420E preds=[], and the only four xrefs
// to the function are `call`s from its two callers.
bool install_reliable_alloc() {
    return install_detour(&g_alloc, RVA_ALLOC_CRITICAL, (void*)&alloc_critical_hook);
}
}
