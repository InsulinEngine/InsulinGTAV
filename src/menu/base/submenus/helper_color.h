#pragma once
#include "menu/base/submenu.h"

// A shared colour editor, not a feature. Any caller points it at a registry
// entry and opens it; the entry index rather than a bare color_rgba* so the
// editor can title itself and reach that colour's default for a revert.
class helper_color_menu : public menu::submenu::submenu {
public:
    void load() override;
    void update() override;
    void update_once() override;

    static void        target(int registry_index);
    static int         current_target();
    // May return nullptr: current_target() only changes via target(), which
    // range-checks against menu::theme::color_count(), but a caller that
    // hasn't gone through target() yet (or a registry that shrinks under it)
    // gets nullptr back from menu::theme::color_ptr() rather than a crash.
    static color_rgba* target_color();

    static helper_color_menu* get();
};
