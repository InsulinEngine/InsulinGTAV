#include "menu/base/util/animated_texture.h"
#include "platform/paths.h"
#include "menu/base/util/frame_clock.h"
#include "platform/log.h"
#include "rage/gfx.h"
#include "util/json.h"
#include <orbis/libkernel.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

namespace menu {
    void animated_texture::add_frame(const char* texture_name, int delay_ms) {
        if (!texture_name || !texture_name[0]) return;
        if ((int)m_names.size() >= animation::k_max_frames) {
            LOG_WARN("anim: \"%s\" already holds %d frames, refusing \"%s\"",
                     m_dict.c_str(), animation::k_max_frames, texture_name);
            return;
        }
        if (delay_ms <= 0) delay_ms = animation::k_default_delay_ms;
        if (delay_ms > 65535) delay_ms = 65535;          // uint16_t storage
        m_names.push_back(stl::string(texture_name));
        m_delays.push_back((uint16_t)delay_ms);
    }

    void animated_texture::clear() {
        m_names.clear();
        m_delays.clear();
        m_elapsed_ms = 0;
    }

    void animated_texture::advance(float dt_seconds) {
        if (m_names.size() == 0 || dt_seconds <= 0.f) return;
        m_elapsed_ms += (int)(dt_seconds * 1000.f + 0.5f);

        // Bound the accumulator either way, so it cannot overflow across a long
        // session (signed overflow is UB, and int milliseconds run out after ~24
        // days of uptime). Looping wraps; one-shot saturates at the end, which
        // frame_clock already reads as "stay on the last frame".
        const int total = menu::frame_clock::total_ms(&m_delays[0], (int)m_delays.size());
        if (total > 0) {
            if (m_loop)                     m_elapsed_ms %= total;
            else if (m_elapsed_ms > total)  m_elapsed_ms = total;
        }
    }

    int animated_texture::current_index() const {
        if (m_names.size() == 0) return 0;
        return menu::frame_clock::frame_at(&m_delays[0], (int)m_delays.size(), m_elapsed_ms, m_loop);
    }

    stl::pair<stl::string, stl::string> animated_texture::current() const {
        if (m_names.size() == 0) return stl::make_pair(stl::string(""), stl::string(""));
        // Named rather than make_pair'd: stl::decay strips references but not cv,
        // so deducing from const members here would yield pair<const string,
        // const string>, which does not convert to the return type.
        return stl::pair<stl::string, stl::string>(m_dict, m_names[current_index()]);
    }

    namespace animation {
        struct slot { stl::string name; animated_texture anim; };

        static stl::vector<slot>& registry() {
            static stl::vector<slot> instance;   // function-local: no .init_array
            return instance;
        }

        animated_texture* get(const char* name) {
            if (!name) return nullptr;
            stl::vector<slot>& r = registry();
            for (size_t i = 0; i < r.size(); i++)
                if (r[i].name == name) return &r[i].anim;
            return nullptr;
        }

        animated_texture* create(const char* name) {
            if (!name || !name[0]) return nullptr;
            animated_texture* existing = get(name);
            if (existing) { existing->clear(); return existing; }

            slot s;
            s.name = name;
            registry().push_back(s);
            return &registry()[registry().size() - 1].anim;
        }

        static bool ext_is(const char* name, const char* dotext) {
            size_t ln = strlen(name), le = strlen(dotext);
            if (ln < le) return false;
            const char* e = name + ln - le;
            for (size_t i = 0; i < le; i++) {
                char a = e[i], b = dotext[i];
                if (a >= 'A' && a <= 'Z') a = (char)(a + 32);
                if (b >= 'A' && b <= 'Z') b = (char)(b + 32);
                if (a != b) return false;
            }
            return true;
        }

        // Insertion-sorted directory listing of image files, so 000.png..015.png
        // play in order regardless of what the filesystem hands back.
        static stl::vector<stl::string> image_files(const char* dir) {
            stl::vector<stl::string> out;
            int fd = sceKernelOpen(dir, 0 /* O_RDONLY */, 0);
            if (fd < 0) return out;

            char buf[4096];
            int n;
            while ((n = sceKernelGetdents(fd, buf, sizeof(buf))) > 0) {
                int pos = 0;
                while (pos < n) {
                    struct dirent* de = (struct dirent*)(buf + pos);
                    if (de->d_reclen == 0) break;
                    if (de->d_namlen > 0 && (ext_is(de->d_name, ".png") || ext_is(de->d_name, ".dds"))) {
                        stl::string name(de->d_name);
                        size_t at = out.size();
                        out.push_back(name);
                        while (at > 0 && strcmp(out[at - 1].c_str(), out[at].c_str()) > 0) {
                            stl::string tmp = out[at - 1];
                            out[at - 1] = out[at];
                            out[at] = tmp;
                            at--;
                        }
                    }
                    pos += de->d_reclen;
                }
            }
            sceKernelClose(fd);
            return out;
        }

        animated_texture* load_from_dir(const char* name, const char* dir) {
            if (!name || !name[0] || !dir || !dir[0]) return nullptr;

            // Frame list: the manifest if there is one, else the sorted directory.
            struct pending { stl::string file; int delay; };
            stl::vector<pending> frames;
            bool loop = true;

            char manifest_path[320];
            snprintf(manifest_path, sizeof(manifest_path), "%s/frames.json", dir);
            tj::json root = tj::json::load_from_file(manifest_path);

            const tj::json* list = root.try_get("frames");
            if (list && list->is_array() && list->size() > 0) {
                loop = root.value_bool("loop", true);
                int fallback = (int)root.value_int("default_delay", k_default_delay_ms);
                if (fallback <= 0) fallback = k_default_delay_ms;

                for (size_t i = 0; i < list->size(); i++) {
                    const tj::json* item = &(*(tj::json*)list)[i];
                    const tj::json* file = item->try_get("file");
                    if (!file || !file->is_string() || !file->get_string()[0]) continue;
                    int delay = (int)item->value_int("delay", fallback);
                    pending p;
                    p.file  = file->get_string();
                    p.delay = delay > 0 ? delay : fallback;
                    frames.push_back(p);
                }
            } else {
                stl::vector<stl::string> found = image_files(dir);
                for (size_t i = 0; i < found.size(); i++) {
                    pending p;
                    p.file  = found[i];
                    p.delay = k_default_delay_ms;
                    frames.push_back(p);
                }
                if (frames.size() > 0)
                    platform::logf("anim", "\"%s\": no frames.json, scanned %d file(s) at %d ms",
                                   name, (int)frames.size(), k_default_delay_ms);
            }

            if (frames.size() == 0) {
                LOG_WARN("anim: \"%s\": nothing loadable in %s", name, dir);
                return nullptr;
            }
            if ((int)frames.size() > k_max_frames)
                LOG_WARN("anim: \"%s\": %d frames found, only the first %d are loaded",
                         name, (int)frames.size(), k_max_frames);

            // Load the images into the shared dictionary under explicit names.
            rage::gfx::texture_dictionary& dict = rage::gfx::menu_textures();
            animated_texture* anim = create(name);
            if (!anim) return nullptr;
            anim->set_dictionary(dict.name());
            anim->set_loop(loop);

            int loaded = 0;
            for (size_t i = 0; i < frames.size() && loaded < k_max_frames; i++) {
                char tex_name[64];
                snprintf(tex_name, sizeof(tex_name), "%s_%03d", name, loaded);
                char full[320];
                snprintf(full, sizeof(full), "%s/%s", dir, frames[i].file.c_str());

                if (!dict.add(tex_name, full)) {
                    LOG_WARN("anim: \"%s\": frame %s failed to load, skipping", name, full);
                    continue;   // a bad frame drops out; the rest still plays
                }
                anim->add_frame(tex_name, frames[i].delay);
                loaded++;
            }

            if (loaded == 0) {
                LOG_ERROR("anim: \"%s\": no frame loaded from %s", name, dir);
                return nullptr;
            }

            dict.commit();   // one commit for the whole sequence
            platform::logf("anim", "\"%s\": %d frame(s) from %s, loop=%d", name, loaded, dir, (int)loop);
            return anim;
        }

        animated_texture* load_banner() {
            return load_from_dir("banner", OZARK_BANNER);
        }

        void update(float dt_seconds) {
            stl::vector<slot>& r = registry();
            for (size_t i = 0; i < r.size(); i++) r[i].anim.advance(dt_seconds);
        }

        stl::pair<stl::string, stl::string> header_asset() {
            animated_texture* banner = get("banner");
            if (banner && banner->ready()) return banner->current();
            return stl::make_pair(stl::string("insulin"), stl::string("logo"));
        }
    }
}
