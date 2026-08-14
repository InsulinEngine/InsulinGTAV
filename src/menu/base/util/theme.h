#pragma once
#include "platform/stdafx.h"
#include "stl/vector.h"
#include "stl/string.h"
#include "global/ui_vars.h"

// Theme system: named registries of the ui_vars colours / fonts / positions,
// serialised to /data/insulin/themes/<name>.json via the tj mini-JSON and applied
// back at runtime. Mirrors Ozark's settings/themes but adapted to PS4 (tj JSON +
// sceKernelGetdents dir scan). Texture-per-slot theming is a follow-up that needs
// renderer slot-draw changes; colours/fonts/positions are covered here.
namespace menu::theme {
    const char* dir();                       // "/data/insulin/themes"

    void save(const char* name);             // write current ui_vars -> <name>.json
    bool load_file(const char* path);        // apply a theme .json (absolute path)
    bool load_by_name(const char* name);     // apply <dir>/<name>.json
    void reset_to_default();                 // restore the built-in default palette

    stl::vector<stl::string> list();         // theme names (stem, no .json) in the dir

    // The colour registry, exposed for editing surfaces. theme.cpp already owns
    // this table for save/load; a second copy elsewhere would drift against it
    // the first time a colour is added to ui_vars.
    int         color_count();
    const char* color_name(int index);   // stable key, e.g. "option_selected"
    color_rgba* color_ptr(int index);    // nullptr for an invalid index
}
