#pragma once
#include "menu/base/submenu.h"
#include "menu/base/util/esp.h"

// The shared ESP editor, not a feature. A consumer points it at its context and
// opens it; every level below edits whatever current() returns.
class helper_esp_menu : public menu::submenu::submenu {
public:
    void load() override;
    void update_once() override;
    void update() override;

    // Call before opening, from the click handler of the option that opens it.
    // `T` is the submenu the user is on, and becomes this menu's parent for
    // this opening: a shared editor cannot set its parent once at load, because
    // "back" has to return to whichever of its several openers was used. With
    // no parent at all, set_submenu_previous() treats it as top-level - it
    // routes to main AND closes the whole menu (submenu_handler.cpp).
    // `title` must outlive the menu - a literal.
    template<typename T>
    static void open_for(menu::esp::esp_context* ctx, const char* title) {
        get()->set_parent<T>();
        set_target(ctx, title);
    }

    static menu::esp::esp_context* current();

    static helper_esp_menu* get();

private:
    // The half of open_for() that does not depend on the parent type, kept out
    // of the header. Not public: an opener that set the target without setting
    // the parent is exactly the bug open_for() exists to prevent.
    static void set_target(menu::esp::esp_context* ctx, const char* title);
};
