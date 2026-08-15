#include "menu/base/util/menu_images.h"
#include "menu/base/util/animated_texture.h"
#include "menu/base/util/textures.h"  // apply() refreshes the registry on success; not in the header so the two modules stay one-way dependent
#include "global/ui_vars.h"          // apply() writes m_header / m_background
#include "platform/paths.h"
#include "platform/log.h"
#include "rage/gfx.h"
#include "util/image/decode.h"
#include "util/image/dds_write.h"
#include "util/image/scale.h"
#include "util/json.h"

// stb_image_resize's implementation lives here (exactly one TU). Same trap as
// util/image/decode.cpp's STBI_ASSERT: SceLibcInternal exports no
// __assert_fail, so the default STBIR_ASSERT(x) -> assert(x) fails to link.
// Override before the include, in the same arrangement decode.cpp uses.
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STBIR_ASSERT(x) ((void)0)
#include <stb/stb_image_resize.h>

#include <orbis/libkernel.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace menu::images {

    namespace {
        struct box { int w, h; };

        box box_for(slot s) {
            // From the spec: the menu background covers roughly 420x700 at 1080p
            // and the header 420x86. These sit above that with room for other
            // resolutions, and bound what a 4K source can cost.
            return s == slot::header ? box{ 512, 128 } : box{ 512, 1024 };
        }

        // "header" / "background" - the cache subdirectory per slot. Deliberately
        // distinct from anim_name_for()'s "slot_header"/"slot_background" below:
        // that one names an animation-registry/dictionary entry, this one names
        // a filesystem directory, and nothing requires the two spellings match.
        const char* slot_dir_name(slot s) {
            return s == slot::header ? "header" : "background";
        }

        // <OZARK_IMGCACHE>/<slot>/<name>. The slot is part of the path because
        // the same source picture caches at a different box per slot - keying
        // by name alone would let applying a picture as the header make
        // is_cached() answer "yes" for the background too, handing back a
        // frame sized for the wrong box (stretched, visibly blurry, no log).
        void cache_dir_for(slot s, const char* name, char* out, int len) {
            snprintf(out, len, "%s/%s/%s", OZARK_IMGCACHE, slot_dir_name(s), name);
        }

        // texture_dictionary::copy_name (rage/gfx.cpp) truncates any texture
        // name at 63 characters. A stem longer than that would register under
        // one string and be looked up under a shorter, silently truncated one -
        // a checkerboard with no error anywhere. list_sources() refuses such a
        // stem outright rather than letting it reach add()/add_texture() and
        // fail in a way nothing explains.
        const int k_max_registrable_stem = 63;

        // The animation registry key for each slot - one per slot, never
        // derived from the picture's own name. load_from_dir names its
        // textures "<name>_000" onward, so two pictures both starting at 000
        // would collide in the shared dictionary when a second slot's frames
        // commit.
        const char* anim_name_for(slot s) {
            return s == slot::header ? "slot_header" : "slot_background";
        }

        // Source extensions, in the one place both the scanner and convert()'s
        // reverse lookup read from - four separate copies of this list is how
        // one of them drifts.
        const char* k_exts[] = { ".png", ".jpg", ".jpeg", ".gif", ".bmp" };
        const int k_ext_count = (int)(sizeof(k_exts) / sizeof(k_exts[0]));

        bool ext_is(const char* name, const char* dotext) {
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
    }

    // ---- list_sources ---------------------------------------------------------
    // Same sceKernelOpen + sceKernelGetdents loop shape as menu::theme::list()
    // (src/menu/base/util/theme.cpp) - this project's established idiom for a
    // directory scan, not reinvented here.
    stl::vector<stl::string> list_sources() {
        stl::vector<stl::string> out;
        int fd = sceKernelOpen(OZARK_IMAGES, 0 /* O_RDONLY */, 0);
        if (fd < 0) return out;

        char buf[4096];
        int n;
        while ((n = sceKernelGetdents(fd, buf, sizeof(buf))) > 0) {
            int pos = 0;
            while (pos < n) {
                struct dirent* de = (struct dirent*)(buf + pos);
                if (de->d_reclen == 0) break;

                int L = 0; while (de->d_name[L]) L++;
                // ".cache" (and any other entry without one of these extensions,
                // including plain directories) never matches, so it is skipped
                // here without a separate name check.
                for (int e = 0; e < k_ext_count; e++) {
                    size_t elen = strlen(k_exts[e]);
                    if ((size_t)L > elen && ext_is(de->d_name, k_exts[e])) {
                        int stem_len = L - (int)elen;
                        if (stem_len > k_max_registrable_stem) {
                            LOG_WARN("images: \"%s\" is too long (%d chars, max %d) to register as a "
                                     "texture, skipping", de->d_name, stem_len, k_max_registrable_stem);
                            break;
                        }
                        // Stem kept exactly as it appears on disk. /data is
                        // case-sensitive and convert() rebuilds the source path
                        // from this same stem, so lowercasing here would make a
                        // capitalised source file (e.g. "MyPic.PNG") unopenable.
                        char stem[256]; int i = 0;
                        for (; i < stem_len && i < (int)sizeof(stem) - 1; i++)
                            stem[i] = de->d_name[i];
                        stem[i] = 0;
                        out.push_back(stl::string(stem));
                        break;
                    }
                }
                pos += de->d_reclen;
            }
        }
        sceKernelClose(fd);
        return out;
    }

    // ---- is_cached --------------------------------------------------------
    // True only when <OZARK_IMGCACHE>/<slot>/<name>/frames.json exists and is
    // not older than the source file. A user who edits foo.png and re-picks it
    // must see the new picture, not a stale cache with no way to force a
    // refresh. If either stat fails, this reports "not cached" - reconverting
    // costs seconds, showing the wrong picture costs trust.
    bool is_cached(const char* name, slot s) {
        if (!name || !name[0]) return false;

        char dir[320];
        cache_dir_for(s, name, dir, sizeof(dir));
        char manifest[384];
        snprintf(manifest, sizeof(manifest), "%s/frames.json", dir);

        OrbisKernelStat cache_st;
        if (sceKernelStat(manifest, &cache_st) != 0) return false;

        // Find whatever source extension is actually on disk - convert() does
        // the same reverse lookup, so if it can't find the source either, that
        // attempt fails loudly and says why.
        bool found_src = false;
        OrbisKernelStat src_st;
        for (int i = 0; i < k_ext_count && !found_src; i++) {
            char src[320];
            snprintf(src, sizeof(src), "%s/%s%s", OZARK_IMAGES, name, k_exts[i]);
            if (sceKernelStat(src, &src_st) == 0) found_src = true;
        }
        if (!found_src) return false;

        return cache_st.st_mtim.tv_sec >= src_st.st_mtim.tv_sec;
    }

    // ---- convert ------------------------------------------------------------
    bool convert(const char* name, slot s) {
        if (!name || !name[0]) return false;

        char src[320];
        bool found = false;
        for (int i = 0; i < k_ext_count && !found; i++) {
            snprintf(src, sizeof(src), "%s/%s%s", OZARK_IMAGES, name, k_exts[i]);
            int fd = sceKernelOpen(src, 0 /* O_RDONLY */, 0);
            if (fd >= 0) { sceKernelClose(fd); found = true; }
        }
        if (!found) {
            LOG_ERROR("images: no source for \"%s\" in %s", name, OZARK_IMAGES);
            return false;
        }

        util::image::decoded d;
        if (!util::image::decode_file(src, &d)) {
            LOG_ERROR("images: could not decode \"%s\"", src);
            return false;
        }

        box b = box_for(s);
        int dw = 0, dh = 0;
        util::image::fit_within(d.w, d.h, b.w, b.h, &dw, &dh);

        // The animation system refuses more than k_max_frames and says so rather
        // than truncating quietly. Cap here too, and report it, so the limit is
        // visible at the place the user chose the picture.
        int frames = d.frames;
        if (frames > menu::animation::k_max_frames) {
            frames = menu::animation::k_max_frames;
            char cap_msg[96];
            snprintf(cap_msg, sizeof(cap_msg),
                     "Image has more frames than the menu plays; using the first %d",
                     menu::animation::k_max_frames);
            platform::notify(cap_msg);
        }

        sceKernelMkdir(OZARK_IMAGES, 0777);
        sceKernelMkdir(OZARK_IMGCACHE, 0777);

        char slot_dir[320];
        snprintf(slot_dir, sizeof(slot_dir), "%s/%s", OZARK_IMGCACHE, slot_dir_name(s));
        sceKernelMkdir(slot_dir, 0777);

        char dir[320];
        cache_dir_for(s, name, dir, sizeof(dir));
        sceKernelMkdir(dir, 0777);

        unsigned char* scaled = nullptr;
        if (dw != d.w || dh != d.h) {
            scaled = (unsigned char*)malloc((size_t)dw * dh * 4);
            if (!scaled) {
                LOG_ERROR("images: out of memory scaling \"%s\" to %dx%d", name, dw, dh);
                util::image::free_decoded(&d);
                return false;
            }
        }

        tj::json manifest;
        tj::json list;
        bool ok = true;

        for (int i = 0; i < frames && ok; i++) {
            const unsigned char* srcpx = d.rgba + (size_t)i * d.w * d.h * 4;
            const unsigned char* use = srcpx;

            if (scaled) {
                if (!stbir_resize_uint8(srcpx, d.w, d.h, 0, scaled, dw, dh, 0, 4)) {
                    LOG_ERROR("images: resize failed for \"%s\" frame %d", name, i);
                    ok = false;
                    break;
                }
                use = scaled;
            }

            char frame_path[384];
            snprintf(frame_path, sizeof(frame_path), "%s/frame_%03d.dds", dir, i);
            ok = util::image::write_dds(frame_path, use, dw, dh);
            if (!ok) { LOG_ERROR("images: could not write %s", frame_path); break; }

            char file_name[64];
            snprintf(file_name, sizeof(file_name), "frame_%03d.dds", i);
            tj::json entry;
            entry["file"] = tj::json(file_name);
            entry["delay"] = tj::json((long long)d.delays_ms[i]);
            list.push_back(entry);
        }

        if (ok) {
            manifest["loop"] = tj::json(true);
            manifest["frames"] = list;
            char man[384];
            snprintf(man, sizeof(man), "%s/frames.json", dir);
            // Frames are already on disk at this point - if the manifest fails
            // to write, is_cached() can never find it and every future apply()
            // reconverts the whole picture from scratch. Report it as the
            // failure it is rather than returning true with half the cache
            // missing.
            if (!manifest.save_to_file(man, 2)) {
                LOG_ERROR("images: could not write %s", man);
                ok = false;
            } else {
                platform::logf("images", "\"%s\": %d frame(s) at %dx%d", name, frames, dw, dh);
            }
        }

        if (scaled) free(scaled);
        util::image::free_decoded(&d);
        return ok;
    }

    // ---- apply ----------------------------------------------------------------
    bool apply(const char* name, slot s) {
        menu_texture& mt = (s == slot::header) ? global::ui::m_header
                                               : global::ui::m_background;
        const char* anim_name = anim_name_for(s);

        if (!name || !name[0]) {           // "None"
            // The renderer's per-slot animation branch reads the animation
            // registry directly and never looks at m_enabled, so clearing the
            // flags alone would leave the picture drawing with no way to take
            // it off.
            if (menu::animated_texture* a = menu::animation::get(anim_name)) a->clear();
            mt.m_enabled = false;
            mt.m_texture.set("");
            // The still frame registered below (under the picture's own stem)
            // is left in the dictionary on purpose: texture_dictionary has no
            // remove, only replace-by-re-adding, and with m_enabled false
            // nothing looks it up any more. One unused entry, not a leak.
            return true;
        }

        if (!is_cached(name, s) && !convert(name, s))
            return false;

        char dir[320];
        cache_dir_for(s, name, dir, sizeof(dir));

        // Registered under the slot's own name so header and background cannot
        // collide in the dictionary - load_from_dir names textures
        // "<name>_000", and two pictures both starting at 000 would drop each
        // other on commit().
        if (!menu::animation::load_from_dir(anim_name, dir)) {
            LOG_ERROR("images: nothing loadable in %s", dir);
            return false;
        }

        char frame0[384];
        snprintf(frame0, sizeof(frame0), "%s/frame_000.dds", dir);
        // The still under the plain stem is what get_texture() resolves to, and
        // it is the fallback the animation branch falls through to. Without it
        // m_enabled = true points the slot at an unregistered name, which draws
        // the checkerboard over the gradient/game header this is meant to
        // replace - so both calls below are checked, and the slot is left
        // untouched (mt is not written) on either failure.
        //
        // The animation frames re-register under the fixed "slot_header_000".."
        // slot_background_000".. names every time load_from_dir runs above, so
        // they stay capped at 32 dictionary entries total regardless of how
        // many pictures are applied in a session. This still does not: it is
        // added under the picture's own stem, a new entry for every distinct
        // picture ever applied this session. texture_dictionary caps at 64
        // entries and has no remove, only replace - so there is a real ceiling
        // of roughly 32 distinct pictures per session. Not solved here; this
        // just turns hitting it into a named error instead of a checkerboard.
        if (!rage::gfx::menu_textures().add(name, frame0)) {
            LOG_ERROR("images: could not register still texture \"%s\" - %s failed to load, "
                      "or the shared \"insulin\" dictionary is already at its 64-texture cap",
                      name, frame0);
            return false;
        }
        if (!rage::gfx::menu_textures().commit()) {
            LOG_ERROR("images: dictionary commit failed applying \"%s\"", name);
            return false;
        }

        mt.m_texture.set(name);
        mt.m_enabled = true;

        // A picture applied mid-session (e.g. dropped in over FTP after boot)
        // needs its name in the registry too, or renderer::get_texture's
        // find_if never finds it and the still fallback silently fails while
        // the animation keeps drawing - a half-working feature. load() clears
        // and refills, so this is a refresh, not a doubling.
        menu::textures::load();
        return true;
    }

    // ---- request / update (deferred apply) -------------------------------
    //
    // theme::load_file() runs inside menu::build() (by way of
    // apply_last_theme()), before the frame hook exists and possibly before
    // the game has finished loading. apply() above decodes an image, resizes
    // it, writes DDS frames and injects a texture dictionary - none of that
    // is legal there. request() records what the caller wants without doing
    // any of it; update() performs it later, from menu::tick().
    //
    // State is file-scope POD - plain bool/char, zero-initialised in .bss by
    // the loader - because this plugin runs no .init_array, so nothing with a
    // constructor (an stl::string included) may live at file scope.
    namespace {
        bool g_header_pending = false;
        char g_header_pending_name[128];
        bool g_background_pending = false;
        char g_background_pending_name[128];
    }

    void request(const char* name, slot s) {
        size_t n = name ? strlen(name) : 0;

        // Each branch bounds its copy by its own destination's sizeof, so the
        // two buffers stay independently safe even if one of them is ever
        // resized on its own - a shared bound (e.g. sizeof(g_header_pending_name)
        // reused for the background copy) would silently stop matching the
        // moment the sizes diverged.
        if (s == slot::header) {
            if (n > sizeof(g_header_pending_name) - 1) n = sizeof(g_header_pending_name) - 1;
            if (n) memcpy(g_header_pending_name, name, n);
            g_header_pending_name[n] = 0;
            g_header_pending = true;
        } else {
            if (n > sizeof(g_background_pending_name) - 1) n = sizeof(g_background_pending_name) - 1;
            if (n) memcpy(g_background_pending_name, name, n);
            g_background_pending_name[n] = 0;
            g_background_pending = true;
        }
    }

    void update() {
        // At most one pending request per call, so a theme that names both a
        // header and a background picture spreads their (potentially costly,
        // first-time) conversion across two frames instead of stalling one.
        if (g_header_pending) {
            g_header_pending = false;   // cleared first: a failing convert must not retry forever
            const char* n = g_header_pending_name[0] ? g_header_pending_name : nullptr;
            if (!apply(n, slot::header))
                LOG_WARN("theme: header image \"%s\" not available", g_header_pending_name);
            return;
        }
        if (g_background_pending) {
            g_background_pending = false;
            const char* n = g_background_pending_name[0] ? g_background_pending_name : nullptr;
            if (!apply(n, slot::background))
                LOG_WARN("theme: background image \"%s\" not available", g_background_pending_name);
        }
    }
}
