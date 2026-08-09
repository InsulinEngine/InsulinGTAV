#pragma once
#include "platform/stdafx.h"

// Hotkey binding is a feature, out of scope for the base. Stubbed so the option
// classes that call into it compile and behave as "no hotkey".
class base_option;

namespace menu::hotkey {
    inline void read_hotkey(base_option* /*option*/) {}
    inline void register_hotkey(int /*key*/, base_option* /*option*/) {}
    inline void unregister_hotkey(base_option* /*option*/) {}
}
