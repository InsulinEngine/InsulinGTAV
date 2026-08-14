#include "menu/panels/builtin_panels.h"
#include "menu/base/util/panels.h"

namespace menu::panels {
    // Panels arrive in Tasks 9 and 10; this file owns their registration so
    // menu.cpp - already 300 lines of it - does not absorb four more render
    // callbacks.
    void register_builtin_panels() {
        panel_parent* parent = new panel_parent();
        parent->m_render = true;
        parent->m_id   = "insulin";
        parent->m_name = "Insulin";
        get_panels().push_back(parent);
    }
}
