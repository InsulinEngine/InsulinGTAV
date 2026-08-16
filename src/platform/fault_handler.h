#pragma once

// Last words. This plugin's crashes end the log without a line of their own:
// /data/sce_coredumps is empty on this console and the kernel-log port accepts a
// connection then delivers nothing, so a fault currently leaves nothing at all
// behind. This installs signal handlers that write one line to the unbuffered
// file log before the process goes down, carrying the faulting address as an
// eboot RVA - which is the number that turns a crash into an IDB lookup.
//
// The handler does not try to recover. It records, restores the default
// disposition and lets the fault kill the process as it would have: continuing
// past a SIGSEGV would mask the bug and risk corrupting more state.
namespace platform::fault {

    // Installs handlers for the fatal memory/instruction signals. Safe to call
    // from module_start - it touches no natives and no game state. Returns true
    // if at least one handler was installed.
    bool install();

    // Checks whether SIGSEGV still points at our handler and reinstalls it if
    // not, logging the first time it has to. Call periodically from the frame
    // hook: installing once at plugin load is not enough, because the game boots
    // *after* us and a real crash on 2026-08-16 produced no [FAULT] line at all -
    // the likeliest explanation being that GTA installs its own crash handler
    // later and silently replaces ours. Cheap: one sigaction query.
    void reinstall_if_stolen();

    // Deliberately dereferences a null pointer, to prove the handler actually
    // fires on this platform. Nothing in the shipped menu should call this
    // except the explicit debug option.
    void trigger_test_fault();
}
