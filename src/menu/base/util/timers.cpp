#include "menu/base/util/timers.h"
#include "rage/invoker/natives.h"

namespace menu::timers {
    void run_timed(int* timer, int ms, stl::function<void()> callback) {
        if (*timer < native::get_game_timer()) {
            *timer = native::get_game_timer() + ms;
            callback();
        }
    }

    void timer::start(unsigned long long ticks) {
        if (m_tick) {
            m_ready_at = platform::now_ms64() + ticks;
            m_tick = false;
        }
    }

    bool timer::is_ready() {
        return platform::now_ms64() > m_ready_at;
    }

    void timer::reset() {
        m_tick = true;
    }
}
