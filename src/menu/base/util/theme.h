#pragma once
#include "platform/stdafx.h"
#include "stl/vector.h"
#include "stl/string.h"

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
}
