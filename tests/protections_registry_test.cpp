// Host unit tests for the protections filter registry.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/protections_registry_test.cpp src/protections/registry.cpp -o build/protections_registry_test.exe
//   ./build/protections_registry_test.exe
// C headers only: MSVC's C++ stdlib rejects the installed clang (STL1000).
#include "protections/registry.h"
#include <stdio.h>
#include <string.h>

// Host stubs for the detour installers the table now points at. Their real
// definitions live in src/protections/hooks_*.cpp, which pull in the GoldHEN
// detour API and the eboot base and therefore cannot be built on the host.
// install_enabled_filters() does call them, so they must exist and must
// report "not installed" - the table invariants are what this test guards, not
// the hooking.
namespace protections {
    bool install_render_ped()         { return false; }
    bool install_render_entity()      { return false; }
    bool install_render_big_ped()     { return false; }
    bool install_task_ambient_clips() { return false; }
    bool install_task_parachute()     { return false; }
    bool install_invalid_decal()      { return false; }
    bool install_searchlight()        { return false; }
    bool install_skeleton_extension() { return false; }
    bool install_reliable_alloc()     { return false; }
    bool install_script_event()       { return false; }
    // Learn-mode accessors live in hooks_script_event.cpp, which this host test
    // does not link; stub them so registry.h stays satisfiable on its own.
    int  learned_count()              { return 0; }
    bool learned_at(int, uint32_t*, uint32_t*, uint8_t*) { return false; }
    void learned_clear()              {}
    bool install_weapon_damage()      { return false; }
    bool install_give_weapon()        { return false; }
    bool install_remove_weapon()      { return false; }
    int  weapon_learned_count()       { return 0; }
    void weapon_learned_clear()       {}
    bool install_give_control()       { return false; }
    bool install_request_control()    { return false; }
    bool install_clear_ped_tasks()    { return false; }
    bool install_ragdoll_request()    { return false; }
    bool install_script_entity_state(){ return false; }
    bool install_play_sound()         { return false; }
    bool install_change_radio()       { return false; }
    bool install_door_break()         { return false; }
    int  sound_learned_count()        { return 0; }
    void sound_learned_clear()        {}
}

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

    // The honesty invariant. A filter with no detour has no call to refuse, so
    // it cannot block - and if the table ever claimed otherwise the drain would
    // print "prot BLOCK <name>" into the kernel log for something that was
    // never blocked, which is the one artefact the console phase reads. This is
    // the rule that keeps the table honest as filters are added; a new row that
    // forgets can_block fails here rather than on console.
    //
    // Only this direction is an invariant. reliable_alloc has an installer and
    // still cannot block (its hook never calls should_block), so
    // install != nullptr does NOT imply can_block.
    for (int i = 0; i < count(); i++)
        check_true("no install fn means cannot block", at(i)->install || !at(i)->can_block);

    // ...and the field is not vacuously false everywhere, which would satisfy
    // the invariant while disabling Enforce across the whole subsystem.
    int blockers = 0;
    for (int i = 0; i < count(); i++)
        if (at(i)->can_block) blockers++;
    check_true("some filters can block", blockers > 0);

    // The four filters that cannot block are named rather than counted, so that
    // flipping one of them on has to be a deliberate edit to this test too.
    check_true("self_test cannot block",        !find(filter_id::self_test)->can_block);
    check_true("fragment_physics cannot block", !find(filter_id::fragment_physics)->can_block);
    check_true("pool_exhaustion cannot block",  !find(filter_id::pool_exhaustion)->can_block);
    check_true("reliable_alloc cannot block",   !find(filter_id::reliable_alloc)->can_block);

    // should_block() is deliberately NOT gated on can_block: it is the hook's
    // mode gate and the hooks that cannot block simply never call it. The
    // distinction is applied where it is read - in the drain's label and in the
    // menu's item list.
    set_mode(filter_id::self_test, mode::enforce);
    check_true("should_block still follows the mode alone", should_block(filter_id::self_test));
    set_mode(filter_id::self_test, mode::log);

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
