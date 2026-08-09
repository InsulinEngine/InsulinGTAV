#pragma once
#include "option.h"
#include "menu/base/base.h"
#include "rage/types/base_types.h"

class break_option : public base_option {
public:
    break_option(stl::string name)
        : base_option(name) {}

    // add_option takes T& (lvalue); ref() yields an lvalue for a bare break.
    break_option& ref() { return *this; }
    break_option& add_requirement(stl::function<bool()> function) { m_requirement = function; return *this; }
    break_option& add_update(stl::function<void(break_option*)> function) { m_on_update = function; return *this; }
    break_option& add_translate() { m_name.set_translate(true); m_tooltip.set_translate(true); return *this; }

    void render(int position);
    void render_selected(int position, stl::stack<stl::string> submenu_name_stack);
    void invoke_hotkey() {}
private:
    stl::function<void(break_option*)> m_on_update = [](break_option*) {};
};
