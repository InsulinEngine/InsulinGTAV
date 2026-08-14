#include "submenu_handler.h"
#include "base.h"
#include "renderer.h"
#include "rage/invoker/natives.h"
#include "global/ui_vars.h"
#include "submenus/main.h"
#include "platform/log.h"

namespace menu::submenu::handler {
    void submenu_handler::load() {
        m_main = m_current = main_menu::get();
    }

    void submenu_handler::update() {
        if (m_current) {
            m_current->update_menu();

            if (m_next) {
                menu::base::set_current_option(m_next_current_option);
                menu::base::set_scroll_offset(m_next_scroll_offset);

                m_current = m_next;
                m_next = nullptr;
                m_current->update_once();
            }
        }
    }

    void submenu_handler::feature_update() {
        // Boot diagnostics: the first few fan-out passes are the dangerous ones.
        // menu::tick gates this on game::player_valid(), which only means the
        // local player ped exists - Ozark's gate (GameStatePlaying) is stricter,
        // so these passes can land while the game is still on its loading screen.
        // Trace the first few passes by name and then go quiet; if the game dies
        // in there, the last name on the wire is the submenu that did it.
        static int s_boot_passes = 0;
        const bool trace = s_boot_passes < 5;
        if (trace)
            platform::klogf("fu: pass %d start (%d submenus)",
                            s_boot_passes, (int)m_submenus.size());

        // Single-player base: no network gate. Feature submenus are out of scope,
        // so this just fans out update to whatever submenus are registered.
        for (submenu* submenu : m_submenus) {
            if (trace)
                platform::klogf("fu: %s", submenu->get_name().get_original().c_str());
            submenu->feature_update();
        }

        if (trace) {
            platform::klogf("fu: pass %d done", s_boot_passes);
            s_boot_passes++;
        }
    }

    void submenu_handler::cleanup() {
        for (submenu* submenu : m_submenus) {
            delete submenu;
        }
    }

    void submenu_handler::add_submenu(submenu* submenu) {
        m_submenus.push_back(submenu);
    }

    void submenu_handler::set_submenu(submenu* submenu) {
        menu::renderer::set_smooth_scroll(global::ui::g_position.y);

        m_current->set_old_current_option(menu::base::get_current_option());
        m_current->set_old_scroll_offset(menu::base::get_scroll_offset());

        m_next_current_option = 0;
        m_next_scroll_offset = 0;

        m_previous = m_current;
        m_next = submenu;
    }

    void submenu_handler::set_submenu_previous(bool reset) {
        m_previous = m_current;
        if (m_current->get_parent() == nullptr) {
            m_next = m_main;
            m_next_current_option = 0;
            m_next_scroll_offset = 0;
            menu::base::set_open(false);
        } else {
            m_next = m_current->get_parent();

            if (reset) {
                m_next_current_option = 0;
                m_next_scroll_offset = 0;
                menu::renderer::set_smooth_scroll(global::ui::g_position.y);
            } else {
                m_next_current_option = m_next->get_old_current_option();
                m_next_scroll_offset = m_next->get_old_scroll_offset();

                int max_options = base::get_max_options();
                int scroller_position = math::clamp(m_next_current_option - m_next_scroll_offset > max_options ? max_options : m_next_current_option - m_next_scroll_offset, 0, max_options);
                menu::renderer::set_smooth_scroll(global::ui::g_position.y + (scroller_position * global::ui::g_option_scale));
            }
        }

        global::ui::g_rendering_tooltip.clear();
    }

    submenu_handler* get_submenu_handler() {
        static submenu_handler instance;
        return &instance;
    }
}
