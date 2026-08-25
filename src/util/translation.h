#pragma once
#include "stl/string.h"
#include "stl/vector.h"

// Runtime localization. A language file /data/Ozark/lang/<name>.json is a flat
// object of { "original string": "translated string", ... }. load() builds a
// hash->translation table and localization::get() looks its original text up
// there (only when a language is active), so no per-string registration table is
// needed (avoids the dangling-pointer problem of Ozark's localization* list when
// options are copied into shared_ptrs). User supplies the language files.
namespace util::i18n {
    const char* dir();                                    // "/data/Ozark/lang"

    void load_file(const char* path);                     // load a language .json, activate
    void load_by_name(const char* name);                  // <dir>/<name>.json
    void reset();                                         // clear + back to original (English)
    bool active();

    stl::string translate(const stl::string& original);   // mapped translation, or original
    stl::vector<stl::string> list();                      // language names (stem) in the dir
}
