#pragma once

#include "menu/base/submenu.h"

// The companion server's switch and status, plus the per-frame drain that turns
// queued HTTP work into engine work.
//
// Boot rules this file obeys, both of them load-bearing:
//   - load() only sets state. It must never call a native or start the server,
//     because build() can run while the game is still on its loading screen.
//   - feature_update() is where everything happens; menu::tick already gates it
//     on game::player_valid().

class companion_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;

    static companion_menu* get();
};

// No update() override: the status line is a break option that rewrites its own
// name through add_update, so there is nothing left for the submenu to do while
// it is open. Declaring update() here without defining it would be a link error.

// Handed to the server as its state provider. Runs on the HTTP thread and reads
// only a snapshot the game thread published - it never touches the engine.
unsigned companion_state_json(char* out, unsigned cap);
