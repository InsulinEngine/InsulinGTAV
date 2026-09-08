#include "menu/base/submenus/vehicle_preview.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/invoker.h"
#include "rage/txd_store.h"
#include "platform/log.h"

#include <stdint.h>
#include <string.h>

namespace menu { namespace vehicle_preview {
namespace {

    // Website texture dictionaries, ported from the Xbox 360 build (which took
    // them from Ozark). Dicts absent on this build simply never load and are
    // dropped by the scan, so keeping the full list is harmless.
    const char* const g_dicts[] = {
        "candc_apartments","candc_assault","candc_battle","candc_casinoheist",
        "candc_chopper","candc_default","candc_executive1","candc_gunrunning",
        "candc_hacker","candc_importexport","candc_smuggler","candc_truck",
        "candc_xmas2017","casino_suites","dock_default","dock_dlc_banner",
        "dock_dlc_color","dock_dlc_executive1","dock_dlc_fittings","dock_dlc_flag",
        "dock_dlc_lights","dock_dlc_model","dock_dlc_slides","dock_yacht_backgrounds",
        "elt_default","elt_dlc_apartments","elt_dlc_assault","elt_dlc_battle",
        "elt_dlc_business","elt_dlc_executive1","elt_dlc_luxe","elt_dlc_pilot",
        "elt_dlc_smuggler","lgm_default","lgm_dlc_apartments","lgm_dlc_arena",
        "lgm_dlc_assault","lgm_dlc_battle","lgm_dlc_biker","lgm_dlc_business",
        "lgm_dlc_business2","lgm_dlc_casinoheist","lgm_dlc_executive1",
        "lgm_dlc_gunrunning","lgm_dlc_heist","lgm_dlc_importexport",
        "lgm_dlc_lts_creator","lgm_dlc_luxe","lgm_dlc_pilot","lgm_dlc_smuggler",
        "lgm_dlc_specialraces","lgm_dlc_stunt","lgm_dlc_summer2020",
        "lgm_dlc_valentines","lgm_dlc_valentines2","lgm_dlc_vinewood",
        "lgm_dlc_xmas2017","lsc_default","lsc_dlc_import_export","lsc_dlc_summer2020",
        "lsc_jan2016","lsc_lowrider2","mba_vehicles","pandm_default","sssa_default",
        "sssa_dlc_arena","sssa_dlc_assault","sssa_dlc_battle","sssa_dlc_biker",
        "sssa_dlc_business","sssa_dlc_business2","sssa_dlc_casinoheist",
        "sssa_dlc_christmas_2","sssa_dlc_christmas_3","sssa_dlc_executive_1",
        "sssa_dlc_halloween","sssa_dlc_heist","sssa_dlc_hipster",
        "sssa_dlc_independence","sssa_dlc_lts_creator","sssa_dlc_mp_to_sp",
        "sssa_dlc_smuggler","sssa_dlc_stunt","sssa_dlc_summer2020",
        "sssa_dlc_valentines","sssa_dlc_vinewood","sssa_dlc_xmas2017",
        "lgm_dlc_tuner","sssa_dlc_tuner",

        // Everything above came from the 360 port and stops at Los Santos
        // Tuners. The dicts below are the rest of what the shipped game keeps in
        // update.rpf/x64/patch/data/cdimages/scaleform_web.rpf, read out of the
        // archives rather than guessed - that directory IS the website dict set.
        // Anything this build predates simply never loads (see scan_pump) -
        // though on a current console none of these are absent: the scan
        // reported remaining=0 for all 127, up to and including the 2026 dicts.
        // Adding them is what fixed the Issi8, which resolves out of
        // sssa_dlc_xmas2022 (measured - the model ships in a later pack than
        // the dict its showroom image ended up in).
        "sssa_dlc_heist4","sssa_dlc_security","sssa_dlc_sum2","sssa_dlc_xmas2022",
        "sssa_dlc_2023_01","sssa_dlc_2023_2","sssa_dlc_2024_1","sssa_dlc_2024_2",
        "sssa_dlc_2025_1","sssa_dlc_2025_2","sssa_dlc_2026_1",
        "lgm_dlc_heist4","lgm_dlc_security","lgm_dlc_sum2","lgm_dlc_xmas2022",
        "lgm_dlc_2023_01","lgm_dlc_2023_2","lgm_dlc_2024_1","lgm_dlc_2024_2",
        "lgm_dlc_2025_1","lgm_dlc_2025_2","lgm_dlc_2026_1",
        "candc_heist4","candc_sub","candc_xmas2022","candc_2023_01","candc_2023_2",
        "candc_dlc_2024_1","candc_dlc_2024_2","candc_dlc_2025_1","candc_dlc_2025_2",
        "candc_dlc_2026_1",
        "elt_dlc_sum2","elt_dlc_2024_2","dock_dlc_heist4","lsc_dlc_sum2",
        "pandm_dlc_2023_01","mba_garage"
    };
    const int g_dict_count = (int)(sizeof(g_dicts) / sizeof(g_dicts[0]));

    // The map below packs a dict index into a uint8_t, and the per-dict arrays
    // are sized from this count rather than a hardcoded 128 - which the list had
    // already grown to within one entry of.
    static_assert(g_dict_count <= 255, "dict index is stored as uint8_t in map_slot");

    // ---- g_TxdStore resolver ---------------------------------------------------
    // g_TxdStore @ RVA 0x3E4B218. The lookup itself lives in rage/txd_store.cpp
    // so it can be host-tested; see that header for the table layout and for why
    // this does NOT go through vtable+0x48. Short version: it used to, and
    // vtable+0x48 takes a name, not a hash - it hashed our hash and the console
    // died in atStringHash+0xB with rdi = 0xB63C4BA0 = hash("candc_apartments").
    // pgDictionary, once resolved: +0x10 parent, +0x20 u32* hashes (sorted),
    // +0x28 u16 count, +0x30 entries.
    const uint64_t RVA_TXD_STORE = 0x3E4B218;

    uint64_t find_txd(uint32_t dict_hash) {
        const uint64_t base_img = (uint64_t)rage::invoker::g_eboot_base;
        if (!base_img) return 0;
        const uint64_t store = base_img + RVA_TXD_STORE;
        return rage::txd::dict_from_slot(store, rage::txd::slot_from_hash(store, dict_hash));
    }

    // ---- model-hash -> dict-index map (open-addressed, power-of-two) -----------
    // 8192 slots, not the 4096 this started with. Open addressing degrades
    // sharply as it fills, and a full map silently drops entries - which would
    // show up as exactly the symptom this list was extended to fix, a car with
    // no image, from an unrelated cause. g_map_fill is logged at scan end so the
    // real load factor is a measured number rather than an assumption.
    const int MAP_BITS = 13;
    const int MAP_SIZE = 1 << MAP_BITS;
    const int MAP_MASK = MAP_SIZE - 1;
    struct map_slot { uint32_t hash; uint8_t dict; };
    map_slot g_map[MAP_SIZE] = {};               // hash 0 == empty
    int      g_map_fill = 0;

    void map_insert(uint32_t h, uint8_t d) {
        if (!h) return;                          // 0 is the empty marker; skip
        uint32_t i = h & MAP_MASK;
        for (int p = 0; p < MAP_SIZE; p++, i = (i + 1) & MAP_MASK) {
            if (g_map[i].hash == 0) { g_map[i].hash = h; g_map[i].dict = d; g_map_fill++; return; }
            if (g_map[i].hash == h) return;      // first dict wins
        }
    }
    int map_lookup(uint32_t h) {
        if (!h) return -1;
        uint32_t i = h & MAP_MASK;
        for (int p = 0; p < MAP_SIZE; p++, i = (i + 1) & MAP_MASK) {
            if (g_map[i].hash == 0) return -1;
            if (g_map[i].hash == h) return g_map[i].dict;
        }
        return -1;
    }

    // ---- background scan -------------------------------------------------------
    enum { SCAN_IDLE, SCAN_RUNNING, SCAN_DONE };
    int  g_scan_state = SCAN_IDLE;
    uint8_t g_done[g_dict_count] = {};     // per-dict: read or permanently skipped
    uint8_t g_req[g_dict_count]  = {};     // per-dict: we issued the streaming request
    int  g_remaining    = 0;
    int  g_pass         = 0;
    const int kScanPasses = 300;  // frame budget; absent dicts never load
    uint8_t g_stable[g_dict_count] = {};   // per-dict: consecutive passes seen loaded
    const int kStable = 4;        // settle this many passes before reading a dict's
                                  // pgDictionary; a just-flipped "loaded" flag can
                                  // precede the dictionary being fully constructed

    void scan_start() {
        if (g_scan_state != SCAN_IDLE) return;
        g_remaining = g_dict_count;
        g_pass = 0;
        for (int i = 0; i < g_dict_count; i++) {
            if (native::has_streamed_texture_dict_loaded(g_dicts[i])) {
                g_req[i] = 0;                     // already resident; don't release it
            } else {
                native::request_streamed_texture_dict(g_dicts[i], false);
                g_req[i] = 1;
            }
        }
        g_scan_state = SCAN_RUNNING;
        platform::logf("VPrev", "scan start: %d dicts requested", g_dict_count);
    }

    // Read one loaded dict's texture-name hashes into the map. Returns false if the
    // dict's store slot is not resolvable yet (loaded flag flipped but pointer not
    // published), so the caller retries next pass.
    bool scan_one(int i) {
        const uint64_t dict = find_txd(native::get_hash_key(g_dicts[i]));
        if (!dict) return false;
        uint32_t* hashes = *(uint32_t* const volatile*)(dict + 0x20);
        uint32_t  count  = *(const volatile uint16_t*)(dict + 0x28);
        if (hashes && count) {
            if (count > 1024) count = 1024;       // sanity cap
            for (uint32_t j = 0; j < count; j++)
                map_insert(hashes[j], (uint8_t)i);
        }
        return true;
    }

    void scan_pump() {
        if (g_scan_state != SCAN_RUNNING) return;
        if (g_pass++ >= kScanPasses || g_remaining <= 0) {
            for (int i = 0; i < g_dict_count; i++)     // release anything unfinished
                if (!g_done[i] && g_req[i])
                    native::set_streamed_texture_dict_as_no_longer_needed(g_dicts[i]);
            g_scan_state = SCAN_DONE;
            platform::logf("VPrev", "scan done: pass=%d remaining=%d map=%d/%d",
                           g_pass, g_remaining, g_map_fill, MAP_SIZE);

            // Name the dicts that never resolved. A dict this build predates is
            // the expected case, and the names are what say which list entries
            // are worth keeping - "remaining=38" on its own says nothing. Built
            // in batches because logf's buffer is 512 bytes.
            char line[400];
            int  n = 0;
            for (int i = 0; i < g_dict_count; i++) {
                if (g_done[i]) continue;
                const int len = (int)strlen(g_dicts[i]);
                if (n + len + 2 >= (int)sizeof(line)) {
                    line[n] = 0;
                    platform::logf("VPrev", "absent: %s", line);
                    n = 0;
                }
                if (n) line[n++] = ' ';
                memcpy(line + n, g_dicts[i], (size_t)len);
                n += len;
            }
            if (n) { line[n] = 0; platform::logf("VPrev", "absent: %s", line); }
            return;
        }
        for (int i = 0; i < g_dict_count; i++) {
            if (g_done[i]) continue;
            if (!native::has_streamed_texture_dict_loaded(g_dicts[i])) { g_stable[i] = 0; continue; }
            if (g_stable[i] < kStable) { g_stable[i]++; continue; }   // let the dict settle
            if (!scan_one(i)) continue;                // slot not ready; retry next pass
            if (g_req[i]) native::set_streamed_texture_dict_as_no_longer_needed(g_dicts[i]);
            g_done[i] = 1;
            g_remaining--;
        }
    }

    // ---- pinned preview dict + crash-safety gates ------------------------------
    // Only one preview dict is pinned at a time. A dict is not drawn until it has
    // been reported loaded for kReady consecutive frames SINCE our request (a just-
    // flipped "loaded" flag can precede a valid texture and DRAW_SPRITE on it hits
    // vtable garbage). Releases are deferred kRelease frames so a release never
    // frees a texture whose draw is still queued in the deferred render buffer.
    int g_pinned = -1;            // index into g_dicts, -1 = none
    int g_ready  = 0;
    const int kReady = 20;

    struct pending { int dict; int frames; };
    pending g_pending[16] = {};
    int g_pending_count = 0;
    const int kRelease = 6;

    void defer_release(int dict) {
        if (dict < 0) return;
        for (int i = 0; i < g_pending_count; i++)
            if (g_pending[i].dict == dict) { g_pending[i].frames = kRelease; return; }
        if (g_pending_count < 16) { g_pending[g_pending_count].dict = dict; g_pending[g_pending_count].frames = kRelease; g_pending_count++; }
    }
    void cancel_pending(int dict) {
        for (int i = 0; i < g_pending_count; i++)
            if (g_pending[i].dict == dict) {
                g_pending[i] = g_pending[g_pending_count - 1];
                g_pending_count--;
                return;
            }
    }

    void pin(int dict) {
        if (dict == g_pinned) return;
        if (g_pinned >= 0) defer_release(g_pinned);    // old draw may still be in flight
        cancel_pending(dict);                          // re-pinning: keep resident
        native::request_streamed_texture_dict(g_dicts[dict], false);
        g_pinned = dict;
        g_ready  = 0;                                  // restart the settle window
    }
    void unpin() {
        if (g_pinned < 0) return;
        defer_release(g_pinned);
        g_pinned = -1;
        g_ready  = 0;
    }

    // ---- idle detection: release the pin after browse() stops being called -----
    int g_idle = 999;

    // ---- the model whose image to draw this frame ------------------------------
    int         g_model_dict = -1;
    // texture name to draw (model, or model+"2"); backed by a static buffer since
    // the +"2" case needs a place to live across the frame.
    char        g_tex[80] = {};

    // Arena War themed conversions. Their images live in the Maze Bank Arena
    // dict as <stem>_c_<1..3>, and the stem is NOT the model name: issi4 is
    // stored under "issi3_c_1", dominator4 under "dominato_c_1" - truncated, in
    // the game's own data. No string rule produces those, so they are a table.
    //
    // The stems and suffixes were read out of mba_vehicles.ytd in the archives;
    // what is INFERRED is which model gets which of the three, taken to follow
    // the shop order Apocalypse, Future Shock, Nightmare. A wrong guess there
    // shows the right car in the wrong theme, which the eye catches at once.
    struct alias { const char* model; const char* tex; };
    const alias g_aliases[] = {
        { "issi4",      "issi3_c_1"     }, { "issi5",      "issi3_c_2"     }, { "issi6",      "issi3_c_3"     },
        { "impaler2",   "impaler_c_1"   }, { "impaler3",   "impaler_c_2"   }, { "impaler4",   "impaler_c_3"   },
        { "slamvan4",   "slamvan_c_1"   }, { "slamvan5",   "slamvan_c_2"   }, { "slamvan6",   "slamvan_c_3"   },
        { "dominator4", "dominato_c_1"  }, { "dominator5", "dominato_c_2"  }, { "dominator6", "dominato_c_3"  },
        { "bruiser",    "bruiser_c_1"   }, { "bruiser2",   "bruiser_c_2"   }, { "bruiser3",   "bruiser_c_3"   },
        { "deathbike",  "deathbike_c_1" }, { "deathbike2", "deathbike_c_2" }, { "deathbike3", "deathbike_c_3" },
        { "monster3",   "monster_c_1"   }, { "monster4",   "monster_c_2"   }, { "monster5",   "monster_c_3"   },

        // The two survivors of the old suffix rule. Both were found by the
        // audit, and both are kept for the same reason the other 28 were
        // dropped: their target is not itself a model, so the image can only
        // belong to this car.
        { "hardy",      "hardy1"        }, { "btype",      "btype2"        },
    };
    const int g_alias_count = (int)(sizeof(g_aliases) / sizeof(g_aliases[0]));

    // Try one texture name against the scan's map. Pure lookup: it writes the
    // out-parameters only on a hit and touches no selection state, which is what
    // lets audit() call the resolver as freely as the preview does.
    bool try_tex(const char* name, char* tex, unsigned cap, int* dict) {
        const int d = map_lookup(native::get_hash_key(name));
        if (d < 0) return false;
        strncpy(tex, name, cap - 1);
        tex[cap - 1] = 0;
        *dict = d;
        return true;
    }

    enum { path_direct = 0, path_alias, path_count };

    // Resolve a model to (texture name, dict index), or -1 for no image.
    //
    // There is deliberately no "try the model name with a digit stuck on the
    // end" rule here. The 360 port had one (model -> model+"2") and it was
    // wrong: the audit logged all 30 of its hits on this build, and 28 of the
    // targets are THEMSELVES models in the spawner list, so the preview was
    // quietly showing a different car - baller wearing the Baller II's photo,
    // voltic the Rocket Voltic's, sultan the Sultan Classic's. A wrong image is
    // worse than none, because it looks like the feature worked. The two hits
    // whose target is not a model of its own survived, as table rows.
    int resolve_tex(const char* model, char* tex, unsigned cap, int* dict) {
        if (!model || !model[0]) return -1;
        if (try_tex(model, tex, cap, dict)) return path_direct;

        for (int i = 0; i < g_alias_count; i++)
            if (strcmp(g_aliases[i].model, model) == 0)
                return try_tex(g_aliases[i].tex, tex, cap, dict) ? path_alias : -1;
        return -1;
    }

    // The preview's own use: resolve and remember for this frame's draw.
    bool resolve_model(const char* model) {
        char tex[80];
        int  dict = -1;
        if (resolve_tex(model, tex, sizeof(tex), &dict) < 0) return false;
        strncpy(g_tex, tex, sizeof(g_tex) - 1);
        g_tex[sizeof(g_tex) - 1] = 0;
        g_model_dict = dict;
        return true;
    }

    // ---- one-shot audit --------------------------------------------------------
    // Checking previews by hand means highlighting hundreds of cars one at a
    // time. This does it instead: every model the spawner offers goes through the
    // resolver above, and the ones with no image are logged by name. Chunked so
    // it never costs a visible frame, and it runs once per boot.
    const int kAuditChunk = 64;
    int  g_audit_at = 0;
    int  g_audit_hits[path_count] = {};
    int  g_audit_miss = 0;

    // Many names per line: logf's buffer is 512 bytes, and one write per model
    // would be hundreds of file opens. Holds no pointers, so it needs no
    // constructor - this plugin has no .init_array.
    struct linebuf { char buf[400]; int len; };
    linebuf g_miss_buf;
    linebuf g_pair_buf;

    void buf_flush(linebuf* b, const char* tag) {
        if (!b->len) return;
        b->buf[b->len] = 0;
        platform::logf("VPrev", "%s: %s", tag, b->buf);
        b->len = 0;
    }
    void buf_add(linebuf* b, const char* tag, const char* text) {
        const int len = (int)strlen(text);
        if (len + 2 >= (int)sizeof(b->buf)) return;          // never truncate mid-name
        if (b->len + len + 2 >= (int)sizeof(b->buf)) buf_flush(b, tag);
        if (b->len) b->buf[b->len++] = ' ';
        memcpy(b->buf + b->len, text, (size_t)len);
        b->len += len;
    }

    void draw_box() {
        // Reached only through browse()'s gates: the pin matches the resolved
        // model, the dict reports loaded, and it has done so for kReady frames
        // since our own request. The index is still re-checked here, because this
        // is the one place that hands a pointer to the renderer.
        if (g_pinned < 0 || g_pinned >= g_dict_count) return;
        const char* dname = g_dicts[g_pinned];

        // One line per pin change, not per frame. Kept from the diagnostic build:
        // this is the log that proved the scan maps models correctly (pin=39,
        // lgm_dlc_business, for 'alpha'), and it costs one write per selection.
        static int s_logged = -2;
        if (s_logged != g_pinned) {
            platform::logf("VPrev", "draw pin=%d name=%s tex='%s'", g_pinned, dname, g_tex);
            s_logged = g_pinned;
        }

        // Bottom-right, ~16:9, clear of the left-hand menu and the tooltip.
        const float w = 0.26f, h = 0.146f, cx = 0.845f, cy = 0.795f;
        native::draw_sprite(dname, g_tex, cx, cy, w, h, 0.f, 255, 255, 255, 255, 0);
    }
}

// ---------------------------------------------------------------------------
void audit(model_at_fn at, int count) {
    if (g_scan_state != SCAN_DONE || !at || g_audit_at >= count) return;

    const int end = (g_audit_at + kAuditChunk < count) ? g_audit_at + kAuditChunk : count;
    for (; g_audit_at < end; g_audit_at++) {
        const char* m = at(g_audit_at);
        if (!m) continue;
        char tex[80];
        int  dict = -1;
        const int p = resolve_tex(m, tex, sizeof(tex), &dict);
        if (p < 0) {
            g_audit_miss++;
            buf_add(&g_miss_buf, "no image", m);
            continue;
        }
        g_audit_hits[p]++;

        // Every non-direct hit is logged as model=texture. A rule that renames
        // a model is exactly where a wrong-but-plausible image comes from -
        // "scarab" finding "scarab2" looks like success and is not - so these
        // pairs get read rather than trusted.
        if (p != path_direct) {
            char pair[176];
            const int a = (int)strlen(m), b = (int)strlen(tex);
            if (a + b + 2 < (int)sizeof(pair)) {
                memcpy(pair, m, (size_t)a);
                pair[a] = '=';
                memcpy(pair + a + 1, tex, (size_t)b);
                pair[a + 1 + b] = 0;
                buf_add(&g_pair_buf, "mapped", pair);
            }
        }
    }

    if (g_audit_at >= count) {
        buf_flush(&g_pair_buf, "mapped");
        buf_flush(&g_miss_buf, "no image");
        platform::logf("VPrev",
                       "audit: %d models, %d with image (direct=%d alias=%d), %d without",
                       count, count - g_audit_miss,
                       g_audit_hits[path_direct], g_audit_hits[path_alias],
                       g_audit_miss);
    }
}

void browse(const char* model) {
    g_idle = 0;                      // seen this frame -> not idle
    scan_start();                    // idempotent
    if (g_scan_state != SCAN_DONE)   // preview only once the map is built
        return;

    if (!resolve_model(model)) {     // no image for this model -> unpin, draw nothing
        unpin();
        return;
    }
    pin(g_model_dict);
    if (g_pinned == g_model_dict
        && native::has_streamed_texture_dict_loaded(g_dicts[g_pinned])
        && g_ready >= kReady) {
        draw_box();
    }
}

void tick() {
    scan_pump();

    // Settle gate: count uninterrupted loaded frames since our request.
    if (g_pinned >= 0 && native::has_streamed_texture_dict_loaded(g_dicts[g_pinned])) {
        if (g_ready < kReady) g_ready++;
    } else {
        g_ready = 0;
    }

    // Left the list a few frames ago -> release the pin (deferred).
    if (g_idle < 1000) g_idle++;
    if (g_idle > 3 && g_pinned >= 0) unpin();

    // Deferred release queue.
    for (int i = 0; i < g_pending_count; ) {
        if (--g_pending[i].frames <= 0) {
            native::set_streamed_texture_dict_as_no_longer_needed(g_dicts[g_pending[i].dict]);
            g_pending[i] = g_pending[g_pending_count - 1];
            g_pending_count--;
        } else {
            i++;
        }
    }
}

}}
