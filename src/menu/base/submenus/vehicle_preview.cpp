#include "menu/base/submenus/vehicle_preview.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/invoker.h"
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
        "lgm_dlc_tuner","sssa_dlc_tuner"
    };
    const int g_dict_count = (int)(sizeof(g_dicts) / sizeof(g_dicts[0]));

    // ---- g_TxdStore resolver (RE catalog + CScriptHud::GetSpriteTexture 0x9BBB30)
    // g_TxdStore @ RVA 0x3E4B218. vtable+0x48 = FindSlotFromHashKey(hash) -> slot.
    // pool: base *(store+0x38), flags *(store+0x40), stride *(u32*)(store+0x4C).
    // slot invalid if flags[slot] & 0x80. pgDictionary at *(base + stride*slot):
    //   +0x10 parent, +0x20 u32* hashes (sorted), +0x28 u16 count, +0x30 entries.
    const uint64_t RVA_TXD_STORE = 0x3E4B218;

    uint64_t find_txd(uint32_t dict_hash) {
        const uint64_t base_img = (uint64_t)rage::invoker::g_eboot_base;
        if (!base_img) return 0;
        const uint64_t store = base_img + RVA_TXD_STORE;

        const uint64_t vtable = *(const volatile uint64_t*)store;
        if (!vtable) return 0;
        typedef int (*find_slot_fn)(uint64_t, uint32_t);
        const find_slot_fn find_slot = *(find_slot_fn*)(vtable + 0x48);
        const int slot = find_slot(store, dict_hash);
        if (slot < 0) return 0;

        const uint64_t flags = *(const volatile uint64_t*)(store + 0x40);
        if (!flags || (*(const volatile uint8_t*)(flags + (uint32_t)slot) & 0x80)) return 0;

        const uint64_t pool  = *(const volatile uint64_t*)(store + 0x38);
        const uint32_t stride = *(const volatile uint32_t*)(store + 0x4C);
        if (!pool) return 0;
        return *(const volatile uint64_t*)(pool + (uint64_t)stride * (uint32_t)slot);
    }

    // ---- model-hash -> dict-index map (open-addressed, power-of-two) -----------
    const int MAP_BITS = 12;
    const int MAP_SIZE = 1 << MAP_BITS;          // 4096 slots
    const int MAP_MASK = MAP_SIZE - 1;
    struct map_slot { uint32_t hash; uint8_t dict; };
    map_slot g_map[MAP_SIZE] = {};               // hash 0 == empty

    void map_insert(uint32_t h, uint8_t d) {
        if (!h) return;                          // 0 is the empty marker; skip
        uint32_t i = h & MAP_MASK;
        for (int p = 0; p < MAP_SIZE; p++, i = (i + 1) & MAP_MASK) {
            if (g_map[i].hash == 0) { g_map[i].hash = h; g_map[i].dict = d; return; }
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
    uint8_t g_done[128] = {};     // per-dict: read or permanently skipped
    uint8_t g_req[128]  = {};     // per-dict: we issued the streaming request
    int  g_remaining    = 0;
    int  g_pass         = 0;
    const int kScanPasses = 300;  // frame budget; absent dicts never load
    uint8_t g_stable[128] = {};   // per-dict: consecutive passes seen loaded
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
    }

    // Read one loaded dict's texture-name hashes into the map. Returns false if the
    // dict's store slot is not resolvable yet (loaded flag flipped but pointer not
    // published), so the caller retries next pass.
    bool scan_one(int i) {
        const uint64_t dict = find_txd(native::get_hash_key(g_dicts[i]));
        if (!dict) return false;
        // Crash-markers: the last one printed before a hang/crash names exactly
        // which read faulted (find_txd ptr, the +0x28/+0x20 reads, or the loop).
        platform::klogf("vprev scan dict=%d(%s) ptr=%llx", i, g_dicts[i], (unsigned long long)dict);
        uint32_t* hashes = *(uint32_t* const volatile*)(dict + 0x20);
        uint32_t  count  = *(const volatile uint16_t*)(dict + 0x28);
        platform::klogf("vprev scan dict=%d hashes=%llx count=%u", i, (unsigned long long)hashes, count);
        if (hashes && count) {
            if (count > 1024) count = 1024;       // sanity cap
            for (uint32_t j = 0; j < count; j++)
                map_insert(hashes[j], (uint8_t)i);
        }
        platform::klogf("vprev scan dict=%d done", i);
        return true;
    }

    void scan_pump() {
        if (g_scan_state != SCAN_RUNNING) return;
        if (g_pass++ >= kScanPasses || g_remaining <= 0) {
            for (int i = 0; i < g_dict_count; i++)     // release anything unfinished
                if (!g_done[i] && g_req[i])
                    native::set_streamed_texture_dict_as_no_longer_needed(g_dicts[i]);
            g_scan_state = SCAN_DONE;
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

    // Resolve the highlighted model to (dict index, texture name). Tries the model
    // name, then model+"2" (some images use the "2" name), mirroring the 360 code.
    bool resolve_model(const char* model) {
        if (!model || !model[0]) return false;
        int d = map_lookup(native::get_hash_key(model));
        if (d >= 0) {
            strncpy(g_tex, model, sizeof(g_tex) - 1);
            g_tex[sizeof(g_tex) - 1] = 0;
            g_model_dict = d;
            return true;
        }
        char alt[80];
        strncpy(alt, model, sizeof(alt) - 3);
        alt[sizeof(alt) - 3] = 0;
        size_t n = strlen(alt);
        alt[n] = '2'; alt[n + 1] = 0;
        d = map_lookup(native::get_hash_key(alt));
        if (d >= 0) {
            strncpy(g_tex, alt, sizeof(g_tex) - 1);
            g_tex[sizeof(g_tex) - 1] = 0;
            g_model_dict = d;
            return true;
        }
        return false;
    }

    void draw_box() {
        // Marker fires once per drawn model, right before the first DRAW_SPRITE,
        // so a draw-time crash is localised the same way the scan is.
        static int s_logged = -2;
        if (s_logged != g_pinned) { platform::klogf("vprev draw dict=%d tex=%s", g_pinned, g_tex); s_logged = g_pinned; }
        // Bottom-right, ~16:9, clear of the left-hand menu and the tooltip.
        const float w = 0.26f, h = 0.146f, cx = 0.845f, cy = 0.795f;
        native::draw_sprite(g_dicts[g_pinned], g_tex, cx, cy, w, h, 0.f, 255, 255, 255, 255, 0);
    }
}

// ---------------------------------------------------------------------------
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
