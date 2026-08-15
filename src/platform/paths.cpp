#include "platform/paths.h"
#include "platform/log.h"
#include <orbis/libkernel.h>

namespace platform {

    void ensure_data_dir() {
        static bool s_reported = false;

        int r = sceKernelMkdir(OZARK_DIR, 0777);
        // Nested under OZARK_DIR, created right after it so the parent already
        // exists. On a fresh install nothing has ever put this folder there, so
        // without it list_sources() (menu_images.cpp) finds nothing to scan and
        // Settings -> Themes -> Menu Images shows two lone "None" entries with
        // no hint that a picture needs to go somewhere first.
        int ri = sceKernelMkdir(OZARK_IMAGES, 0777);

        // Report once, and report the failure too, because the call this replaces
        // did not: it was an unchecked sceKernelMkdir, so a failure and a success
        // looked identical. When neither of the old folders turned up on this
        // console we had no way to tell whether the directory was never created
        // or had simply been deleted. It creates fine - measured, 0x00000000 -
        // so the absence had some other cause. The line stays so the question is
        // answerable next time instead of guessable.
        //
        // 0 is success; 0x80020011 is "already exists", which is the normal case
        // from the second boot onward and not a problem.
        if (!s_reported) {
            s_reported = true;
            klogf("paths: mkdir(\"%s\") -> 0x%08X%s", OZARK_DIR, (unsigned)r,
                  (r == 0 || (unsigned)r == 0x80020011u) ? " (ok)" : " *** FAILED ***");
            klogf("paths: mkdir(\"%s\") -> 0x%08X%s", OZARK_IMAGES, (unsigned)ri,
                  (ri == 0 || (unsigned)ri == 0x80020011u) ? " (ok)" : " *** FAILED ***");
        }
    }
}
