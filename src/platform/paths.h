#pragma once

// Every path the plugin owns, in one place.
//
// There used to be three: /data/insulin for textures, themes, languages and
// animations, /data/InsulinGTAV for config.json, and /data/insulingtav.log
// loose in the root. One folder now, and one definition - a path spelled out at
// a callsite is a path that drifts from the one the loader creates.
//
// Renaming this constant renames the folder. Anything already written under an
// older name stays where it is; nothing migrates on its own.
#define OZARK_DIR       "/data/Ozark"

#define OZARK_CONFIG    OZARK_DIR "/config.json"
#define OZARK_LOG       OZARK_DIR "/insulingtav.log"
#define OZARK_THEMES    OZARK_DIR "/themes"
#define OZARK_LANG      OZARK_DIR "/lang"
#define OZARK_ANIM      OZARK_DIR "/anim"
#define OZARK_BANNER    OZARK_ANIM "/banner"
#define OZARK_LOGO      OZARK_DIR "/logo.dds"
#define OZARK_IMAGES    OZARK_DIR "/images"
#define OZARK_IMGCACHE  OZARK_IMAGES "/.cache"
#define OZARK_WEB       OZARK_DIR "/web"

namespace platform {
    // Create OZARK_DIR if it is not there. Safe to call repeatedly and safe
    // before anything else exists - it takes no locks and touches no game state.
    //
    // Call this BEFORE the first log write. The log is the tool that survives a
    // crash, and it silently writes nothing if its directory is missing, which
    // would cost exactly the diagnostic you need at the worst moment.
    void ensure_data_dir();
}
