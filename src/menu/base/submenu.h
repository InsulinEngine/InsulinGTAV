#pragma once
#include "platform/stdafx.h"
#include "options/option.h"
#include "util/hotkeys.h"
#include "rage/invoker/natives.h"

// Note: Ozark's submenu.h pulls in every option-type header for convenience.
// We include only option.h (the base class the option vector holds); each
// submenu translation unit includes the concrete option types it uses. This
// decouples the base cluster so it compiles without all option types present.

namespace menu::submenu {
    class submenu {
    public:
        void update_menu();
        void clear_options(int offset);

        virtual void load();
        virtual void update_once();
        virtual void update();
        virtual void feature_update();

        void set_parent(submenu* submenu) { m_parent = submenu; }
        void set_can_be_searched(bool searchable) { m_can_be_searched = searchable; }
        void set_default_instructionals(bool def) { m_default_instructionals = def; }
        void set_old_current_option(int val) { m_old_current_option = val; }
        void set_old_scroll_offset(int val) { m_old_scroll_offset = val; }
        void set_name(stl::string str, bool translation = true, bool searchable = true);

        submenu* get_parent() { return m_parent; }
        bool can_be_searched() { return m_can_be_searched; }
        bool get_default_instructionals() { return m_default_instructionals; }
        int get_old_current_option() { return m_old_current_option; }
        int get_old_scroll_offset() { return m_old_scroll_offset; }
        stl::vector<stl::shared_ptr<base_option>>& get_options() { return m_options; }
        stl::vector<localization>& get_strings() { return m_strings; }
        stl::stack<stl::string>& get_submenu_name_stack() { return m_submenu_name_stack; }
        stl::string get_string(stl::string str);

        void add_string(stl::string str) { m_strings.push_back(localization(str, true)); }
        localization& get_name() { return m_name; }

        template<typename T>
        stl::shared_ptr<T> add_option(T& option) {
            stl::shared_ptr<T> ptr = stl::make_shared<T>(option);
            m_options.push_back(ptr);

            ptr.get()->set_submenu_name_stack(&m_submenu_name_stack);

            if (ptr.get()->has_hotkey()) {
                menu::hotkey::read_hotkey(ptr.get());
            }

            return ptr;
        }

        template<typename T>
        stl::shared_ptr<T>& add_option(stl::shared_ptr<T>& option) {
            m_options.push_back(option);

            option.get()->set_submenu_name_stack(&m_submenu_name_stack);

            if (option.get()->has_hotkey()) {
                menu::hotkey::read_hotkey(option.get());
            }

            return option;
        }

        template<typename T>
        void set_parent() {
            m_parent = T::get();
            m_submenu_name_stack = {};

            m_submenu_name_stack.push(m_name.get_original());

            submenu* submenu = m_parent;
            while (submenu) {
                m_submenu_name_stack.push(submenu->m_name.get_original());
                submenu = submenu->get_parent();
            }
        }
    private:
        localization m_name;
        submenu* m_parent = nullptr;

        bool m_can_be_searched = true;
        bool m_default_instructionals = true;
        int m_old_current_option = 0;
        int m_old_scroll_offset = 0;

        stl::vector<stl::shared_ptr<base_option>> m_options = {};
        stl::vector<localization> m_strings = {};
        stl::stack<stl::string> m_submenu_name_stack = {};
    };
}
