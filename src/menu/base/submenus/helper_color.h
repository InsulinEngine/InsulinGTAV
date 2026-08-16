#pragma once
#include "menu/base/submenu.h"

// A shared colour editor, not a feature. A caller points it at a registry
// entry by index and opens it - the entry index rather than a bare
// color_rgba* so the editor can title itself and reach that colour's default
// for a revert - or, for a colour that has no registry entry, points it
// directly at a color_rgba* and supplies the title itself.
class helper_color_menu : public menu::submenu::submenu {
public:
    void load() override;
    void update() override;
    void update_once() override;

    static void        target(int registry_index);
    // Point the editor at a colour that is not in the theme registry. The
    // index form exists so the editor can title itself and find a default to
    // revert to; supplying the title by argument gives the same thing
    // without forcing every caller's colour into a registry that describes
    // menu chrome and is saved by themes. `title` must outlive the editor -
    // a literal, not a buffer.
    static void        target(color_rgba* colour, const char* title);
    static int         current_target();
    // May return nullptr: current_target() only changes via target(), which
    // range-checks against menu::theme::color_count(), but a caller that
    // hasn't gone through target() yet (or a registry that shrinks under it)
    // gets nullptr back from menu::theme::color_ptr() rather than a crash.
    static color_rgba* target_color();

    static helper_color_menu* get();
};
