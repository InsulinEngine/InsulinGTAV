#pragma once
#include "platform/stdafx.h"
#include "menu/base/options/option.h"
#include "global/ui_vars.h"

// Minimal menu_input for M1: the deferred-action queue + key-name table. The
// colour-modal and hotkey-capture paths (which need color_option / the hotkey
// feature) are stubbed here and filled in with the option types in M2.
namespace menu::input {
    class menu_input {
    public:
        void update();
        void push(stl::function<void()> function);
        void hotkey(stl::string name, base_option* option);
        void color(color_rgba* option);
        int get_key(stl::string name, int default_key);
    private:
        stl::vector<stl::function<void()>> m_queue;
    };

    menu_input* get_menu_input();

    inline void mi_update() { get_menu_input()->update(); }
    inline void push(stl::function<void()> function) { get_menu_input()->push(function); }
    inline void hotkey(stl::string name, base_option* option) { get_menu_input()->hotkey(name, option); }
    inline void color(color_rgba* color) { get_menu_input()->color(color); }
    inline int get_key(stl::string name, int default_key) { return get_menu_input()->get_key(name, default_key); }

    // Key-name table (indexed by open key). Only index 0 is used on PS4 (the
    // keyboard open key is unused; the open bind is L1+O), so a single empty
    // entry is enough; the rest are nullptr and never dereferenced.
    static const char* g_key_names[254] = { "" };
}
