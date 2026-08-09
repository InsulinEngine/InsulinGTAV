#include "util/json.h"
#include <orbis/libkernel.h>

// FreeBSD open(2) flags (the OpenOrbis headers don't provide them).
#define ORBIS_O_RDONLY 0x0000
#define ORBIS_O_WRONLY 0x0001
#define ORBIS_O_CREAT  0x0200
#define ORBIS_O_TRUNC  0x0400

namespace tj {
    bool json::save_to_file(const char* path, int indent) const {
        stl::dstring text = dump(indent);
        int fd = sceKernelOpen(path, ORBIS_O_WRONLY | ORBIS_O_CREAT | ORBIS_O_TRUNC, 0666);
        if (fd < 0) return false;
        size_t len = text.length();
        size_t written = 0;
        while (written < len) {
            long n = sceKernelWrite(fd, text.c_str() + written, len - written);
            if (n <= 0) break;
            written += (size_t)n;
        }
        sceKernelClose(fd);
        return written == len;
    }

    json json::load_from_file(const char* path) {
        int fd = sceKernelOpen(path, ORBIS_O_RDONLY, 0);
        if (fd < 0) return json();

        stl::dstring buf;
        char chunk[4096];
        for (;;) {
            long n = sceKernelRead(fd, chunk, sizeof(chunk) - 1);
            if (n <= 0) break;
            chunk[n] = 0;
            buf.append(chunk);
        }
        sceKernelClose(fd);

        return parse(buf.c_str());
    }
}
