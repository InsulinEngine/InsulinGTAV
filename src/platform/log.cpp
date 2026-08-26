#include "platform/log.h"
#include "platform/paths.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <GoldHEN/Common.h>
#include <orbis/libkernel.h>

// FreeBSD open(2) flags - the OpenOrbis headers don't provide them.
#define ORBIS_O_WRONLY 0x0001
#define ORBIS_O_APPEND 0x0008
#define ORBIS_O_CREAT  0x0200
#define ORBIS_O_TRUNC  0x0400

#define LOG_PATH OZARK_LOG

namespace platform {

    // Truncate the file log to empty. Called once at module_start so each boot
    // starts a fresh log instead of appending to every previous session's - the
    // kernel-log channel (klogf) is a live stream and needs no clearing.
    void log_reset() {
        int fd = sceKernelOpen(LOG_PATH, ORBIS_O_WRONLY | ORBIS_O_CREAT | ORBIS_O_TRUNC, 0666);
        if (fd >= 0) sceKernelClose(fd);
    }

    void log_line(const char* tag, const char* msg) {
        int fd = sceKernelOpen(LOG_PATH, ORBIS_O_WRONLY | ORBIS_O_APPEND | ORBIS_O_CREAT, 0666);
        if (fd < 0) return;
        if (tag && tag[0]) {
            sceKernelWrite(fd, "[", 1);
            sceKernelWrite(fd, tag, strlen(tag));
            sceKernelWrite(fd, "] ", 2);
        }
        sceKernelWrite(fd, msg, strlen(msg));
        sceKernelWrite(fd, "\n", 1);
        sceKernelClose(fd);
    }

    void logf(const char* tag, const char* fmt, ...) {
        char buf[512];
        va_list ap; va_start(ap, fmt);
        vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        log_line(tag, buf);
    }

    void klogf(const char* fmt, ...) {
        char buf[256];
        int n = 0;
        buf[n++] = 'I'; buf[n++] = 'G'; buf[n++] = 'V'; buf[n++] = ' ';

        va_list ap; va_start(ap, fmt);
        vsnprintf(buf + n, sizeof(buf) - n - 1, fmt, ap);
        va_end(ap);

        size_t len = strlen(buf);
        if (len == 0 || buf[len - 1] != '\n') {
            if (len < sizeof(buf) - 1) { buf[len] = '\n'; buf[len + 1] = 0; }
        }
        sceKernelDebugOutText(0, buf);
    }

    void notify(const char* msg) {
        log_line("Notify", msg);

        OrbisNotificationRequest req;
        memset(&req, 0, sizeof(req));
        req.type            = NotificationRequest;
        req.targetId        = -1;
        req.useIconImageUri = 1;
        strncpy(req.iconUri, "cxml://psnotification/tex_icon_system", sizeof(req.iconUri) - 1);
        strncpy(req.message, msg, sizeof(req.message) - 1);
        sceKernelSendNotificationRequest(0, &req, sizeof(req), 0);
    }
}
