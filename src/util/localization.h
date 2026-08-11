#pragma once
#include "platform/stdafx.h"
#include "util/translation.h"

// Trimmed localization: the base only needs it to carry an original string and
// hand it back. The PC build registered every instance in a global translation
// table (global::vars::g_localization_table) for a runtime language swap; that
// feature is out of scope, so register_translation() is a no-op and get()
// simply returns the (mapped==original) text.
class localization {
public:
    localization() {}
    localization(stl::string original, bool translate = false, bool global_register = false) {
        (void)global_register;
        m_translate = translate;
        set(original);
    }

    void reset() { set_mapped(m_original); }
    void set(stl::string str) { m_original = str; set_mapped(m_original); }
    void set_mapped(stl::string str) { m_mapped = str; }
    void set_translate(bool translate) { m_translate = translate; }
    void register_translation() {}   // no-op: no translation table on PS4

    stl::string get_original() { return m_original; }
    stl::string get() { return (m_translate && util::i18n::active()) ? util::i18n::translate(m_original) : m_original; }
    bool has_translation() { return m_translate; }

private:
    bool m_translate = false;
    stl::string m_original;
    stl::string m_mapped;
};
