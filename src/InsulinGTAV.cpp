#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>

#include <GoldHEN/Common.h>
#include <orbis/libkernel.h>

#include "rage/invoker/invoker.h"
#include "rage/invoker/natives.h"
#include "game/game_thread.h"
#include "menu/menu.h"
#include "stl_smoke.h"

#define PLUGIN_NAME    "InsulinGTAV"
#define PLUGIN_VERSION "1.00"

// GoldHEN reads these to list the plugin.
#define attr_public __attribute__((visibility("default")))

attr_public const char *g_pluginName    = PLUGIN_NAME;
attr_public const char *g_pluginDesc    = "Ozark menu-base port";
attr_public const char *g_pluginAuth    = "unknown";
attr_public const char *g_pluginVersion = PLUGIN_VERSION;

// joaat("insulin"), computed offline. GET_HASH_KEY must reproduce it.
#define SMOKE_STRING   "insulin"
#define SMOKE_EXPECTED 0x0669D57Fu

#define LOG_PATH "/data/insulingtav.log"
#define ORBIS_O_WRONLY 0x0001
#define ORBIS_O_APPEND 0x0008
#define ORBIS_O_CREAT  0x0200

static void log_line(const char *msg)
{
    int fd = sceKernelOpen(LOG_PATH, ORBIS_O_WRONLY | ORBIS_O_APPEND | ORBIS_O_CREAT, 0666);
    if (fd < 0)
        return;
    sceKernelWrite(fd, msg, strlen(msg));
    sceKernelWrite(fd, "\n", 1);
    sceKernelClose(fd);
}

static void notify(const char *msg)
{
    log_line(msg);

    OrbisNotificationRequest req;
    memset(&req, 0, sizeof(req));
    req.type            = NotificationRequest;
    req.targetId        = -1;
    req.useIconImageUri = 1;
    strncpy(req.iconUri, "cxml://psnotification/tex_icon_system", sizeof(req.iconUri) - 1);
    strncpy(req.message, msg, sizeof(req.message) - 1);
    sceKernelSendNotificationRequest(0, &req, sizeof(req), 0);
}

// M0: base resolution + native ABI (GET_HASH_KEY) + mini-STL, all at once.
static void run_smoke_test()
{
    Hash h = native::get_hash_key(SMOKE_STRING);
    bool stl_ok = stl_smoke::run();
    char msg[192];
    snprintf(msg, sizeof(msg),
             "base=0x%llx HASH(" SMOKE_STRING ")=0x%08X exp=0x%08X %s | STL=%s",
             (unsigned long long)rage::invoker::g_eboot_base,
             (unsigned)h, SMOKE_EXPECTED,
             h == SMOKE_EXPECTED ? "OK" : "MISMATCH",
             stl_ok ? "OK" : "FAIL");
    notify(msg);
}

static void *worker(void *arg)
{
    (void)arg;
    sceKernelSleep(10);          // let the title settle before touching it
    run_smoke_test();
    return NULL;
}

// Entry points need C linkage: crtprx.o's _init resolves module_start/
// module_stop by their unmangled names.
extern "C" {

int module_start(size_t argc, const void *argp)
{
    (void)argc;
    (void)argp;

    notify(PLUGIN_NAME " v" PLUGIN_VERSION " loaded");

    if (!rage::invoker::resolve_base())
    {
        notify("base resolve FAILED - invoker inert");
        return 0;
    }

    bool hook_ok = game::install_frame_hook();
    notify(hook_ok ? "frame hook installed"
                   : "frame hook FAILED (smoke test still runs)");

    if (hook_ok)
    {
        menu::build();
        game::set_frame_callback(menu::tick);
        notify("menu built");
    }

    OrbisPthread thr;
    if (scePthreadCreate(&thr, NULL, worker, NULL, "insulingtav_test") == 0)
        scePthreadDetach(thr);

    return 0;
}

int module_stop(size_t argc, const void *argp)
{
    (void)argc;
    (void)argp;

    notify(PLUGIN_NAME " unloaded");
    return 0;
}

}
