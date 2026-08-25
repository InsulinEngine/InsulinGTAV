#include "protections/registry.h"

namespace protections {
namespace {
    // Function-local static, not a namespace-scope object: this plugin has no
    // .init_array, so a global constructor would never run.
    filter* table(int* out_count) {
        static filter t[] = {
            { filter_id::self_test,          "Self Test",           1, (int)mode::log, (int)mode::log, false, nullptr },
            { filter_id::skeleton_extension, "Skeleton Extension",  1, (int)mode::log, (int)mode::log, false, &install_skeleton_extension },
            { filter_id::fragment_physics,   "Fragment Physics",    1, (int)mode::log, (int)mode::log, false, nullptr },
            { filter_id::invalid_decal,      "Invalid Decal",       1, (int)mode::log, (int)mode::log, false, &install_invalid_decal },
            { filter_id::searchlight,        "Searchlight",         1, (int)mode::log, (int)mode::log, false, &install_searchlight },
            { filter_id::task_ambient_clips, "Task Ambient Clips",  1, (int)mode::log, (int)mode::log, false, &install_task_ambient_clips },
            { filter_id::task_parachute,     "Task Parachute",      1, (int)mode::log, (int)mode::log, false, &install_task_parachute },
            { filter_id::render_ped,         "Render Ped",          1, (int)mode::log, (int)mode::log, false, &install_render_ped },
            { filter_id::render_entity,      "Render Entity",       1, (int)mode::log, (int)mode::log, false, &install_render_entity },
            { filter_id::render_big_ped,     "Render Big Ped",      1, (int)mode::log, (int)mode::log, false, &install_render_big_ped },
            { filter_id::pool_exhaustion,    "Pool Exhaustion",     1, (int)mode::log, (int)mode::log, false, nullptr },
            { filter_id::reliable_alloc,     "Reliable Allocator",  1, (int)mode::log, (int)mode::log, false, &install_reliable_alloc },
        };
        *out_count = (int)(sizeof(t) / sizeof(t[0]));
        return t;
    }
}

int count() {
    int n = 0;
    table(&n);
    return n;
}

filter* at(int index) {
    int n = 0;
    filter* t = table(&n);
    if (index < 0 || index >= n) return nullptr;
    return &t[index];
}

filter* find(filter_id id) {
    int n = 0;
    filter* t = table(&n);
    for (int i = 0; i < n; i++)
        if (t[i].id == id) return &t[i];
    return nullptr;
}

const char* name_of(filter_id id) {
    filter* f = find(id);
    return f ? f->name : "<unknown>";
}

mode mode_of(filter_id id) {
    filter* f = find(id);
    if (!f) return mode::off;
    const int m = __atomic_load_n(&f->current, __ATOMIC_RELAXED);
    if (m != (int)mode::log && m != (int)mode::enforce) return mode::off;
    return (mode)m;
}

void set_mode(filter_id id, mode m) {
    filter* f = find(id);
    if (!f) return;
    __atomic_store_n(&f->current, (int)m, __ATOMIC_RELAXED);
}

bool should_report(filter_id id) {
    const mode m = mode_of(id);
    return m == mode::log || m == mode::enforce;
}

bool should_block(filter_id id) {
    return mode_of(id) == mode::enforce;
}

bool ensure_installed(filter_id id) {
    filter* f = find(id);
    if (!f) return false;
    if (!f->install) return true;      // nothing to hook
    if (f->installed) return true;
    f->installed = f->install();
    return f->installed;
}

void install_enabled_filters() {
    int n = 0;
    filter* t = table(&n);
    for (int i = 0; i < n; i++)
        if (mode_of(t[i].id) != mode::off)
            ensure_installed(t[i].id);
}
}
