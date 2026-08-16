// Must precede every include: this toolchain's <signal.h> puts siginfo_t,
// struct sigaction and sigaction() itself inside a
// `#if defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE) || ...` block, so
// without a feature macro the header is effectively empty and the type errors
// that follow look like the API is missing entirely.
#define _GNU_SOURCE 1

#include "platform/fault_handler.h"
#include "platform/paths.h"
#include "rage/invoker/invoker.h"

#include <signal.h>
#include <string.h>
#include <stdint.h>
#include <orbis/libkernel.h>

// FreeBSD open(2) flags - the OpenOrbis headers don't provide them. Same values
// platform/log.cpp uses; duplicated rather than shared so this file depends on
// nothing that could itself be broken when it runs.
#define ORBIS_O_WRONLY 0x0001
#define ORBIS_O_APPEND 0x0008
#define ORBIS_O_CREAT  0x0200

// bits/signal.h defines SA_SIGINFO as 4 - the Linux value - and the deliberate
// test on 2026-08-16 proved this kernel ignores it: the handler ran but its
// second argument arrived as 0x1, i.e. the classic BSD `(sig, code, ctx)` form
// rather than `(sig, siginfo_t*, ucontext_t*)`. The PS4 kernel is
// FreeBSD-derived, so the value it actually honours is FreeBSD's 0x40. The
// header's macro is deliberately overridden rather than used.
#undef  SA_SIGINFO
#define SA_SIGINFO 0x0040

namespace platform::fault {

    namespace {

        // Everything below runs inside a signal handler on a process that is
        // already dying, so it uses no libc beyond memcpy-free primitives: no
        // vsnprintf, no malloc, no locks. sceKernelOpen/Write/Close is the same
        // path platform::log_line uses and is about as close to async-signal-safe
        // as this platform offers.

        char  g_buf[512];
        int   g_len = 0;

        void put(const char* s) {
            while (*s && g_len < (int)sizeof(g_buf) - 1) g_buf[g_len++] = *s++;
        }

        void put_hex(uint64_t v) {
            static const char* hexd = "0123456789ABCDEF";
            char tmp[17];
            int n = 0;
            if (!v) tmp[n++] = '0';
            while (v && n < 16) { tmp[n++] = hexd[v & 0xF]; v >>= 4; }
            put("0x");
            while (n-- > 0 && g_len < (int)sizeof(g_buf) - 1) g_buf[g_len++] = tmp[n];
        }

        void put_dec(int v) {
            char tmp[12];
            int n = 0;
            if (v < 0) { put("-"); v = -v; }
            if (!v) tmp[n++] = '0';
            while (v && n < 11) { tmp[n++] = (char)('0' + (v % 10)); v /= 10; }
            while (n-- > 0 && g_len < (int)sizeof(g_buf) - 1) g_buf[g_len++] = tmp[n];
        }

        void flush() {
            if (g_len <= 0) return;
            g_buf[g_len < (int)sizeof(g_buf) ? g_len : (int)sizeof(g_buf) - 1] = 0;
            int fd = sceKernelOpen(OZARK_LOG, ORBIS_O_WRONLY | ORBIS_O_APPEND | ORBIS_O_CREAT, 0666);
            if (fd >= 0) {
                sceKernelWrite(fd, g_buf, (size_t)g_len);
                sceKernelWrite(fd, "\n", 1);
                sceKernelClose(fd);
            }
            g_len = 0;
        }

        // An address inside the eboot image is worth reporting as an RVA, because
        // that is what the IDB is indexed by.
        bool in_eboot(uint64_t a, uint64_t base) {
            return base && a > base && a < base + 0x8000000ULL;
        }

        // ...and an address inside *this plugin* matters just as much, because
        // the very first question about any fault is whose code died. The test
        // fault on 2026-08-16 produced "no eboot pointers" and that was correct
        // but useless: it crashed in our own module, which the eboot filter can
        // never see. `self` is a function in this file, so the plugin's image
        // sits around it; the window is generous because we only need to
        // classify an address, not to locate it precisely.
        uint64_t self_anchor() { return (uint64_t)&in_eboot; }

        bool in_plugin(uint64_t a) {
            const uint64_t s = self_anchor();
            const uint64_t span = 0x400000ULL;   // 4 MB either way
            return a > s - span && a < s + span;
        }

        // If SA_SIGINFO turns out not to be honoured, `info` and `uctx` are
        // whatever happened to sit in rsi/rdx - so nothing here is dereferenced
        // without first checking it looks like a real userspace pointer.
        bool plausible(const void* p) {
            uint64_t a = (uint64_t)p;
            return a > 0x10000ULL && a < 0x0000800000000000ULL && (a & 7) == 0;
        }

        void report(int sig, siginfo_t* info, void* uctx) {
            const uint64_t base = rage::invoker::g_eboot_base;

            g_len = 0;
            put("[FAULT] sig=");   put_dec(sig);
            put(" sa_siginfo=");   put_dec(SA_SIGINFO);
            if (!plausible(info)) {
                put(" (no siginfo: ptr="); put_hex((uint64_t)info); put(")");
            }
            if (plausible(info)) {
                put(" code=");     put_dec(info->si_code);
                put(" addr=");     put_hex((uint64_t)info->si_addr);
                if (in_eboot((uint64_t)info->si_addr, base)) {
                    put(" addr_rva="); put_hex((uint64_t)info->si_addr - base);
                }
            }
            put(" base=");         put_hex(base);
            put(" self=");         put_hex(self_anchor());
            flush();

            // The toolchain exposes no named mcontext fields, so rather than
            // guessing a FreeBSD struct offset, sweep the head of the context and
            // report every qword that points at code. One of them is RIP and the
            // others are return addresses - the IDB says which on the first real
            // fault, and the index can be named after that. Bounded and
            // read-only: a wild context pointer would already have taken us down
            // before this line.
            //
            // Both ranges are reported and tagged, because "did the game die or
            // did we?" is the first fork in every diagnosis: E<rva> is inside the
            // eboot and goes straight into the IDB, P<offset> is inside this
            // plugin and means the bug is ours.
            if (plausible(uctx)) {
                const uint64_t* q = (const uint64_t*)uctx;
                const uint64_t  s = self_anchor();
                g_len = 0;
                put("[FAULT] code-pointers in ctx:");
                int found = 0;
                for (int i = 0; i < 96 && found < 16; i++) {
                    if (in_eboot(q[i], base)) {
                        put(" [");  put_dec(i);
                        put("]=E"); put_hex(q[i] - base);
                        found++;
                    } else if (in_plugin(q[i])) {
                        // Absolute, not an offset from the anchor: the anchor is
                        // an arbitrary function in this file, so addresses land
                        // on both sides of it and an unsigned offset renders the
                        // ones below it as 0xFFFF... garbage. The absolute value
                        // subtracts against `self=` in the line above.
                        put(" [");  put_dec(i);
                        put("]=P"); put_hex(q[i]);
                        found++;
                    }
                }
                if (!found) put(" none");
                flush();
            }

            // Record, then die as we would have. Restoring the default and
            // returning lets the faulting instruction re-run and take the
            // process down properly, which keeps the crash honest instead of
            // papering over it.
            // `struct ::sigaction` and not `struct sigaction`: at global scope a
            // *function* of that name hides the struct, and inside a namespace
            // the unqualified elaborated specifier then declares a brand-new
            // incomplete type instead of finding the real one.
            struct ::sigaction sa;
            memset(&sa, 0, sizeof(sa));
            sa.sa_handler = SIG_DFL;
            sigaction(sig, &sa, nullptr);
        }

        void handler(int sig, siginfo_t* info, void* uctx) {
            // One report only. A handler that faults inside itself would
            // otherwise loop until the log fills the disk.
            static volatile bool reported = false;
            if (reported) return;
            reported = true;
            report(sig, info, uctx);
        }
    }

    bool install() {
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        // Assigned through the union member directly, because the header's
        // convenience macro is broken: it defines sa_sigaction as
        // `__sa_handler.sa_sigaction`, while the union actually names that
        // member `__sa_sigaction`. (sa_handler's macro is fine, so SIG_DFL
        // below can use it.)
        sa.__sa_handler.__sa_sigaction = &handler;
        sa.sa_flags = SA_SIGINFO;
        sigemptyset(&sa.sa_mask);

        static const int sigs[] = { SIGSEGV, SIGBUS, SIGILL, SIGFPE, SIGABRT };
        int ok = 0;
        for (unsigned i = 0; i < sizeof(sigs) / sizeof(sigs[0]); i++)
            if (sigaction(sigs[i], &sa, nullptr) == 0) ok++;
        return ok > 0;
    }

    void reinstall_if_stolen() {
        struct ::sigaction cur;
        memset(&cur, 0, sizeof(cur));
        if (sigaction(SIGSEGV, nullptr, &cur) != 0) return;
        if (cur.__sa_handler.__sa_sigaction == &handler) return;   // still ours

        // Report exactly once. If the game reinstalls its handler on every
        // session transition this would otherwise spam the log, and the fact we
        // care about - that something takes the handler away - is established by
        // the first line.
        static bool announced = false;
        if (!announced) {
            announced = true;
            g_len = 0;
            put("[FAULT] handler was replaced (now ");
            put_hex((uint64_t)cur.__sa_handler.__sa_sigaction);
            put("), reinstalling ours at ");
            put_hex((uint64_t)&handler);
            flush();
        }
        install();
    }

    void trigger_test_fault() {
        // The whole point is the fault, so the write must survive the optimiser.
        volatile int* p = (volatile int*)0;
        *p = 0x1337;
    }
}
