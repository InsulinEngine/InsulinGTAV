#include "rage/gfx.h"
#include "rage/invoker/invoker.h"
#include "platform/log.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <orbis/libkernel.h>
#include <dirent.h>

// See rage/gfx.h + memory/insulingtav-custom-textures.md for the derivation. All
// addresses are RVAs into the CUSA00411 v1.57 eboot (imagebase 0), made absolute
// at call time via rage::invoker::g_eboot_base.
namespace rage::gfx {
    // ---- reverse-engineered RVAs (v1.57) -------------------------------------
    static constexpr uint64_t RVA_FACTORY_SINGLETON = 0x3B11CC8; // grcTextureFactory::sm_Instance (holds grcTextureFactoryGNM*)
    static constexpr uint64_t RVA_CREATE_FROM_FILE  = 0x1A0FB10; // Create(const char* filename, params) -> new grcTextureGNM(filename)
    static constexpr uint64_t RVA_TXDSTORE          = 0x3E4B218; // fwTxdStore OBJECT (embedded; its address IS `this`)
    static constexpr uint64_t RVA_TXD_FINDSLOT      = 0x25D2FC0; // int(this, const char* name) -> idx | -1
    static constexpr uint64_t RVA_TXD_ADDSLOT       = 0x25D3400; // int(this, u32* pHash)        -> idx
    static constexpr uint64_t RVA_TXD_GETPTR        = 0x1ECF0C0; // void*(this, int idx)         -> dict* (redirect-aware)
    static constexpr uint64_t RVA_TXD_ADDREF        = 0x25D31C0; // void(this, int idx)          pin (won't stream out)
    static constexpr uint64_t RVA_HASH_TXD          = 0x18AB110; // u32(u32 seed=0, const char* name) -- FindSlot/AddSlot key
    static constexpr uint64_t RVA_HASH_TEX          = 0x1942AC0; // u32(const char* name, int 0)      -- pgDictionary code
    static constexpr uint64_t RVA_GAME_ALLOCATOR    = 0x3F231D8; // qword: game general allocator*; vtbl[80]=Allocate(size,align), vtbl[104]=Free

    // txd store field offsets: +0x38 slot-array base, +0x40 flag byte-array
    // (bit7 = redirect), +0x4C slot stride. Slot: +0x00 object ptr (pgDictionary).
    static constexpr uint64_t STORE_SLOTBASE_OFF = 0x38;
    static constexpr uint64_t STORE_FLAGARR_OFF  = 0x40;
    static constexpr uint64_t STORE_STRIDE_OFF   = 0x4C;
    static constexpr uint64_t STORE_COUNT_OFF    = 0x88; // slot count (donor-vtable scan bound)

    template<typename Fn> static inline Fn as_fn(uint64_t rva) { return (Fn)(rage::invoker::g_eboot_base + rva); }
    static inline void* at(uint64_t rva) { return (void*)(rage::invoker::g_eboot_base + rva); }

    // Allocate from the game's general RAGE heap (not libc). A pgDictionary and
    // its code/entry arrays injected into the txd store MUST be blocks the game's
    // allocator recognises: on app suspend GTA runs a resource-accounting pass
    // that calls allocator->GetSize(ptr) on them, and that translates ptr into a
    // heap chunk index (ptr - heap_base). A libc-malloc'd pointer is outside the
    // RAGE heap -> garbage index -> wild read -> SIGSEGV (the PS-button crash).
    // Returns nullptr if the allocator singleton isn't up yet; caller falls back.
    static void* rage_alloc(uint64_t size, uint64_t align) {
        if (!rage::invoker::g_eboot_base) return nullptr;
        void* alloc = *(void**)at(RVA_GAME_ALLOCATOR);
        if (!alloc) return nullptr;
        void** vt = *(void***)alloc;
        typedef void* (*allocate_fn)(void* self, uint64_t size, uint64_t align);
        void* p = ((allocate_fn)vt[10])(alloc, size, align);   // vtbl+0x50 = Allocate(size, align)
        if (p) memset(p, 0, size);
        return p;
    }

    static inline char lc(char c) { return (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c; }

    static uint32_t hash_tex(const char* name) {
        return as_fn<uint32_t (*)(const char*, int)>(RVA_HASH_TEX)(name, 0);
    }

    // ---- managed-dict registry (renderer bypass) -----------------------------
    static char g_managed[8][64];
    static int  g_managed_count = 0;

    bool is_custom_dict(const char* dict_name) {
        if (!dict_name) return false;
        for (int i = 0; i < g_managed_count; i++)
            if (!strcmp(g_managed[i], dict_name)) return true;
        return false;
    }
    static void register_managed(const char* dict_name) {
        if (is_custom_dict(dict_name)) return;
        if (g_managed_count < 8) {
            strncpy(g_managed[g_managed_count], dict_name, 63);
            g_managed[g_managed_count][63] = 0;
            g_managed_count++;
        }
    }

    // --------------------------------------------------------------------------
    // The engine's image loader (grcImage::Load, RVA 0x19CF830) reads four bytes
    // and requires the 'DDS ' magic -- it parses nothing else, no PNG, no JPEG.
    // What makes that dangerous is the failure path: its caller (0x19C3680)
    // substitutes a built-in 32x32 magenta/green checkerboard, the constructor
    // builds a genuine texture around it, and the factory returns a valid pointer.
    // Success and failure are therefore indistinguishable downstream, and a wrong
    // file shows up as stripes on screen with a clean log. Check the magic here so
    // it is an error instead.
    static bool is_dds_file(const char* path) {
        int fd = sceKernelOpen(path, 0 /* O_RDONLY */, 0);
        if (fd < 0) { LOG_ERROR("gfx: \"%s\": open failed (%d)", path, fd); return false; }
        char magic[4] = { 0, 0, 0, 0 };
        long n = sceKernelRead(fd, magic, sizeof(magic));
        sceKernelClose(fd);
        if (n != 4 || magic[0] != 'D' || magic[1] != 'D' || magic[2] != 'S' || magic[3] != ' ') {
            LOG_ERROR("gfx: \"%s\": not a DDS file (%02X %02X %02X %02X) -- the engine loads DDS only",
                      path, (unsigned char)magic[0], (unsigned char)magic[1],
                      (unsigned char)magic[2], (unsigned char)magic[3]);
            return false;
        }
        return true;
    }

    void* create_texture_from_file(const char* path) {
        if (!rage::invoker::g_eboot_base) return nullptr;
        if (!is_dds_file(path)) return nullptr;
        void* factory = *(void**)at(RVA_FACTORY_SINGLETON);
        if (!factory) { LOG_ERROR("gfx: texture factory singleton is null (render device not up?)"); return nullptr; }

        // grcTextureGNM(filename) tolerates params == NULL (builds VIDEO/TILED
        // defaults) and does grcImage::Load + Create + Copy internally.
        typedef void* (*create_fn)(void* self, const char* filename, void* params);
        void* tex = as_fn<create_fn>(RVA_CREATE_FROM_FILE)(factory, path, nullptr);
        platform::logf("gfx", "Create(\"%s\") -> %p", path, tex);
        return tex;
    }

    // ---- texture_dictionary --------------------------------------------------
    texture_dictionary::texture_dictionary(const char* dict_name) {
        strncpy(m_dict, dict_name ? dict_name : "custom", 63);
        m_dict[63] = 0;
        m_slot = -1;
        m_committed = false;
        m_injected = false;
    }

    int  texture_dictionary::count() const { return (int)m_entries.size(); }
    bool texture_dictionary::ready() const { return m_committed; }
    const char* texture_dictionary::name() const { return m_dict; }

    static void copy_name(char* out, const char* in) {
        int i = 0;
        for (; in[i] && i < 63; i++) out[i] = lc(in[i]);
        out[i] = 0;
    }

    bool texture_dictionary::has(const char* tex_name) const {
        char q[64]; copy_name(q, tex_name);
        for (size_t i = 0; i < m_entries.size(); i++)
            if (!strcmp(m_entries[i].name, q)) return true;
        return false;
    }

    bool texture_dictionary::add_texture(const char* tex_name, void* tex) {
        if (!tex || !tex_name || !tex_name[0]) return false;
        char nm[64]; copy_name(nm, tex_name);

        // Replace an existing entry with the same name (supports reload). This
        // drops the displaced grcTexture* on the floor: nothing in this codebase
        // reverses the engine's texture destructor (rage_alloc above has no
        // matching free), so a proper release would mean reimplementing that
        // destructor - real work with real crash risk, and not undertaken here.
        // Every replace strands the old texture's video memory; the one caller
        // that replaces routinely (menu::images::apply) logs when it happens.
        for (size_t i = 0; i < m_entries.size(); i++) {
            if (!strcmp(m_entries[i].name, nm)) { m_entries[i].tex = tex; return true; }
        }
        if (m_entries.size() >= 64) { LOG_WARN("gfx: dictionary \"%s\" full, dropping \"%s\"", m_dict, nm); return false; }

        entry e;
        e.tex = tex;
        strncpy(e.name, nm, 63); e.name[63] = 0;
        e.code = hash_tex(nm);
        m_entries.push_back(e);
        return true;
    }

    // Derive a lowercased stem: last path component minus its extension.
    static void stem_of(const char* path, char* out) {
        const char* base = path;
        for (const char* p = path; *p; ++p) if (*p == '/' || *p == '\\') base = p + 1;
        int i = 0;
        for (; base[i] && base[i] != '.' && i < 63; i++) out[i] = lc(base[i]);
        out[i] = 0;
    }

    bool texture_dictionary::add(const char* tex_name, const char* path) {
        void* tex = create_texture_from_file(path);
        if (!tex) return false;
        char stem[64];
        if (!tex_name || !tex_name[0]) { stem_of(path, stem); tex_name = stem; }
        return add_texture(tex_name, tex);
    }

    static bool ext_is(const char* name, const char* dotext /* ".dds" */) {
        size_t ln = strlen(name), le = strlen(dotext);
        if (ln < le) return false;
        const char* e = name + ln - le;
        for (size_t i = 0; i < le; i++) if (lc(e[i]) != lc(dotext[i])) return false;
        return true;
    }

    int texture_dictionary::add_directory(const char* dir) {
        int fd = sceKernelOpen(dir, 0 /* O_RDONLY */, 0);
        if (fd < 0) { platform::logf("gfx", "add_directory(\"%s\"): open failed (%d)", dir, fd); return 0; }

        int added = 0;
        char buf[4096];
        int n;
        while ((n = sceKernelGetdents(fd, buf, sizeof(buf))) > 0) {
            int pos = 0;
            while (pos < n) {
                struct dirent* de = (struct dirent*)(buf + pos);
                if (de->d_reclen == 0) break;                 // guard against a bad record
                if (de->d_namlen > 0 && (ext_is(de->d_name, ".dds") || ext_is(de->d_name, ".png"))) {
                    char full[320];
                    snprintf(full, sizeof(full), "%s/%s", dir, de->d_name);
                    char stem[64]; stem_of(de->d_name, stem);
                    if (add(stem, full)) added++;
                }
                pos += de->d_reclen;
            }
        }
        sceKernelClose(fd);
        platform::logf("gfx", "add_directory(\"%s\"): %d texture(s)", dir, added);
        return added;
    }

    bool texture_dictionary::commit() {
        int n = (int)m_entries.size();
        if (n <= 0 || !rage::invoker::g_eboot_base) return false;
        void* store = at(RVA_TXDSTORE);

        // Sort entries by code ascending (the draw lookup binary-searches codes[]).
        for (int i = 1; i < n; i++) {
            entry key = m_entries[i];
            int j = i - 1;
            while (j >= 0 && m_entries[j].code > key.code) { m_entries[j + 1] = m_entries[j]; j--; }
            m_entries[j + 1] = key;
        }

        // Build the two parallel atArrays, dropping any duplicate codes. These
        // MUST come from the RAGE heap (see rage_alloc): the game accounts them
        // on suspend and would fault on a libc pointer. Dict is over-allocated
        // to 0x80 (real pgDictionary<grcTexture> is ~0x40) and fully zeroed so
        // any field the accounting reads past our layout is a clean NULL, not
        // heap garbage. Falls back to malloc only if the game allocator isn't up.
        uint32_t* codes = (uint32_t*)rage_alloc((uint64_t)n * sizeof(uint32_t), 16);
        void**    ents  = (void**)rage_alloc((uint64_t)n * sizeof(void*), 16);
        uint8_t*  dict  = (uint8_t*)rage_alloc(0x80, 16);
        bool rage_backed = (codes && ents && dict);
        if (!rage_backed) {
            LOG_WARN("gfx: rage_alloc unavailable, falling back to malloc (suspend may crash)");
            if (!codes) codes = (uint32_t*)malloc((size_t)n * sizeof(uint32_t));
            if (!ents)  ents  = (void**)malloc((size_t)n * sizeof(void*));
            if (!dict)  dict  = (uint8_t*)malloc(0x80);
        }
        if (!codes || !ents || !dict) { LOG_ERROR("gfx: commit alloc failed"); return false; }
        int m = 0;
        for (int i = 0; i < n; i++) {
            if (m > 0 && codes[m - 1] == m_entries[i].code) {
                LOG_WARN("gfx: \"%s\": code collision on \"%s\" (0x%08X), skipping", m_dict, m_entries[i].name, m_entries[i].code);
                continue;
            }
            codes[m] = m_entries[i].code;
            ents[m]  = m_entries[i].tex;
            m++;
        }
        memset(dict, 0, 0x80);                           // full zero (covers the malloc fallback path too)
        *(void**)   (dict + 0x00) = nullptr;             // pgBase vtable: adopted from a donor dict below
        *(void**)   (dict + 0x10) = nullptr;             // parent dictionary
        *(void**)   (dict + 0x20) = codes;               // codes atArray: elements
        *(uint16_t*)(dict + 0x28) = (uint16_t)m;         //   count
        *(uint16_t*)(dict + 0x2A) = (uint16_t)m;         //   capacity
        *(void**)   (dict + 0x30) = ents;                // entries atArray: elements
        *(uint16_t*)(dict + 0x38) = (uint16_t)m;         //   count
        *(uint16_t*)(dict + 0x3A) = (uint16_t)m;         //   capacity

        // Find/create the store slot for our dictionary name.
        if (m_slot < 0) {
            m_slot = as_fn<int (*)(void*, const char*)>(RVA_TXD_FINDSLOT)(store, m_dict);
            if (m_slot < 0) {
                uint32_t h = as_fn<uint32_t (*)(uint32_t, const char*)>(RVA_HASH_TXD)(0, m_dict);
                m_slot = as_fn<int (*)(void*, uint32_t*)>(RVA_TXD_ADDSLOT)(store, &h);
                platform::logf("gfx", "AddSlot(\"%s\") -> idx %d (hash 0x%08X)", m_dict, m_slot, h);
            }
            if (m_slot < 0) { LOG_ERROR("gfx: could not obtain a txd slot for \"%s\"", m_dict); return false; }
        }

        // Point the slot at our fabricated dictionary.
        char*    slotbase = *(char**)((uint8_t*)store + STORE_SLOTBASE_OFF);
        uint8_t* flagarr  = *(uint8_t**)((uint8_t*)store + STORE_FLAGARR_OFF);
        uint32_t stride   = *(uint32_t*)((uint8_t*)store + STORE_STRIDE_OFF);
        if (!slotbase || !stride) { LOG_ERROR("gfx: bad store layout (base=%p stride=%u)", (void*)slotbase, stride); return false; }
        if (flagarr && (flagarr[m_slot] & 0x80)) { LOG_ERROR("gfx: slot %d is a redirect; refusing", m_slot); return false; }

        // Adopt a real pgDictionary<grcTexture> vtable from a live neighbour
        // dict. A NULL vtable is fine for the draw lookup (verified), but any
        // engine pass that walks store objects and makes a virtual call (e.g.
        // constrain-time cleanup when the PS-button overlay opens) would crash
        // on it. PC menus never have this problem because they construct the
        // real class; stealing a resident dict's vtable replicates that. The
        // donor's first qword must point into the eboot image to be accepted.
        {
            int total = *(int*)((uint8_t*)store + STORE_COUNT_OFF);
            if (total < 0 || total > 65535) total = 0;
            void* vt = nullptr;
            int donor = -1;
            for (int i = 0; i < total && !vt; i++) {
                if (i == m_slot) continue;
                if (flagarr && (flagarr[i] & 0x80)) continue;      // redirect slot
                void* d = *(void**)(slotbase + (uint64_t)i * stride);
                if (!d) continue;                                   // not resident
                uint64_t cand = *(uint64_t*)d;
                uint64_t base = rage::invoker::g_eboot_base;
                if (cand > base && cand < base + 0x4000000 && (cand & 7) == 0) {
                    vt = (void*)cand;
                    donor = i;
                }
            }
            *(void**)(dict + 0x00) = vt;
            if (vt) platform::logf("gfx", "vtable %p adopted from slot %d", vt, donor);
            else    LOG_WARN("gfx: no donor dict found; vtable stays NULL");
        }

        void** slot = (void**)(slotbase + (uint64_t)m_slot * stride);
        *slot = dict;

        if (!m_injected) {
            as_fn<void (*)(void*, int)>(RVA_TXD_ADDREF)(store, m_slot);   // pin so it won't stream out
            register_managed(m_dict);
            m_injected = true;
        }

        void* got = as_fn<void* (*)(void*, int)>(RVA_TXD_GETPTR)(store, m_slot);
        platform::logf("gfx", "commit \"%s\": %d tex, slot %d, dict %p, GetPtr %p, rage=%d", m_dict, m, m_slot, (void*)dict, got, (int)rage_backed);
        platform::klogf("gfx commit \"%s\": %d tex, slot %d, dict %p, rage=%d", m_dict, m, m_slot, (void*)dict, (int)rage_backed);
        m_committed = (got == (void*)dict);
        if (m_committed) platform::notify("Custom textures loaded");
        return m_committed;
    }

    // ---- menu-facing helpers -------------------------------------------------
    texture_dictionary& menu_textures() {
        static texture_dictionary instance("insulin");
        return instance;
    }

    bool banner_ready() {
        return menu_textures().ready() && menu_textures().has("logo");
    }
}
