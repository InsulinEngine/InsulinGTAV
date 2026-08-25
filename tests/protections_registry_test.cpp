// Host unit tests for the protections filter registry.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/protections_registry_test.cpp src/protections/registry.cpp -o build/protections_registry_test.exe
//   ./build/protections_registry_test.exe
// C headers only: MSVC's C++ stdlib rejects the installed clang (STL1000).
#include "protections/registry.h"
#include <stdio.h>
#include <string.h>

static int g_failed = 0;

static void check_true(const char* what, bool ok) {
    if (!ok) { printf("FAIL %s\n", what); g_failed++; }
    else     { printf("ok   %s\n", what); }
}

int main() {
    using namespace protections;

    // Every filter has a non-empty, unique name, and names are stable config
    // keys - a duplicate would make two filters share a persisted setting.
    check_true("registry is non-empty", count() > 0);
    for (int i = 0; i < count(); i++) {
        filter* f = at(i);
        check_true("filter has a name", f->name && f->name[0]);
        for (int j = i + 1; j < count(); j++)
            check_true("names are unique", strcmp(f->name, at(j)->name) != 0);
    }

    // find() round-trips against at().
    for (int i = 0; i < count(); i++)
        check_true("find matches at", find(at(i)->id) == at(i));

    // An id outside the table resolves to nothing rather than reading past it.
    check_true("unknown id finds nothing", find((filter_id)0x7FFF) == nullptr);
    check_true("unknown id names safely", name_of((filter_id)0x7FFF) != nullptr);

    // Mode gating: off reports nothing, log reports without blocking,
    // enforce does both. This is the whole log-before-enforce contract.
    const filter_id probe = at(0)->id;

    set_mode(probe, mode::off);
    check_true("off does not report", !should_report(probe));
    check_true("off does not block", !should_block(probe));

    set_mode(probe, mode::log);
    check_true("log reports", should_report(probe));
    check_true("log does not block", !should_block(probe));

    set_mode(probe, mode::enforce);
    check_true("enforce reports", should_report(probe));
    check_true("enforce blocks", should_block(probe));

    // An out-of-range mode value must not enable blocking.
    set_mode(probe, (mode)99);
    check_true("bogus mode does not block", !should_block(probe));

    // An unknown id is inert rather than a crash - hooks call these directly.
    check_true("unknown id does not report", !should_report((filter_id)0x7FFF));
    check_true("unknown id does not block", !should_block((filter_id)0x7FFF));

    // Every filter defaults to log, never enforce. New detections must prove
    // themselves against real traffic before they are allowed to drop it.
    for (int i = 0; i < count(); i++)
        check_true("defaults to log", at(i)->default_mode == (int)mode::log);

    // install_enabled_filters() covers the gap left by add_savable restoring a
    // mode without firing its change handler. With no install function set it
    // must be a safe no-op, and it must be idempotent - it runs once per boot
    // but nothing should break if it runs twice.
    set_mode(probe, mode::enforce);
    install_enabled_filters();
    install_enabled_filters();
    check_true("install_enabled_filters leaves mode alone", should_block(probe));
    for (int i = 0; i < count(); i++)
        check_true("no install fn means not installed", at(i)->install || !at(i)->installed);

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed != 0;
}
