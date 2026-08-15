#include "util/translation.h"
#include "platform/paths.h"
#include "util/json.h"
#include "stl/pair.h"

#include <orbis/libkernel.h>
#include <dirent.h>
#include <stdint.h>
#include <stdio.h>

namespace util::i18n {
    // hash -> translated string (linear-searched; language files are small).
    static stl::vector<stl::pair<uint32_t, stl::string>> s_map;
    static bool s_active = false;

    const char* dir() { return OZARK_LANG; }

    static uint32_t hash(const char* s) {
        uint32_t h = 0;
        for (; s && *s; ++s) { h += (uint8_t)*s; h += h << 10; h ^= h >> 6; }
        h += h << 3; h ^= h >> 11; h += h << 15;
        return h;
    }

    void load_file(const char* path) {
        tj::json root = tj::json::load_from_file(path);
        s_map.clear();
        for (size_t i = 0; i < root.member_count(); i++) {
            const char* orig = root.key_at(i);
            const tj::json* v = root.try_get(orig);
            if (v && v->is_string()) s_map.push_back(stl::make_pair(hash(orig), stl::string(v->get_string())));
        }
        s_active = s_map.size() > 0;
    }

    void load_by_name(const char* name) {
        char p[256];
        snprintf(p, sizeof(p), "%s/%s.json", dir(), name);
        load_file(p);
    }

    void reset() { s_map.clear(); s_active = false; }
    bool active() { return s_active; }

    stl::string translate(const stl::string& original) {
        if (!s_active) return original;
        uint32_t h = hash(original.c_str());
        for (size_t i = 0; i < s_map.size(); i++)
            if (s_map[i].first == h) return s_map[i].second;
        return original;
    }

    stl::vector<stl::string> list() {
        stl::vector<stl::string> out;
        int fd = sceKernelOpen(dir(), 0 /* O_RDONLY */, 0);
        if (fd < 0) return out;

        char buf[4096];
        int n;
        while ((n = sceKernelGetdents(fd, buf, sizeof(buf))) > 0) {
            int pos = 0;
            while (pos < n) {
                struct dirent* de = (struct dirent*)(buf + pos);
                if (de->d_reclen == 0) break;
                int L = 0; while (de->d_name[L]) L++;
                if (L > 5) {
                    const char* e = de->d_name + L - 5;
                    if (e[0] == '.' && e[1] == 'j' && e[2] == 's' && e[3] == 'o' && e[4] == 'n') {
                        char stem[64]; int i = 0;
                        for (; i < L - 5 && i < 63; i++) stem[i] = de->d_name[i];
                        stem[i] = 0;
                        out.push_back(stl::string(stem));
                    }
                }
                pos += de->d_reclen;
            }
        }
        sceKernelClose(fd);
        return out;
    }
}
