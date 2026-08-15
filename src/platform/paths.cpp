#include "platform/paths.h"
#include "platform/log.h"
#include <orbis/libkernel.h>

namespace platform {

    void ensure_data_dir() {
        static bool s_reported = false;

        int r = sceKernelMkdir(OZARK_DIR, 0777);

        // Report once, and report the failure too. The folder this replaces was
        // created by an unchecked sceKernelMkdir, and on this console it never
        // appeared at all - which means every "savable" option silently wrote
        // nothing and no theme ever survived a restart. Nobody could see that,
        // because a failing mkdir looked exactly like a succeeding one.
        //
        // 0 is success; 0x80020011 is "already exists", which is the normal case
        // from the second boot onward and not a problem.
        if (!s_reported) {
            s_reported = true;
            klogf("paths: mkdir(\"%s\") -> 0x%08X%s", OZARK_DIR, (unsigned)r,
                  (r == 0 || (unsigned)r == 0x80020011u) ? " (ok)" : " *** FAILED ***");
        }
    }
}
