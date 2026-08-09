#pragma once

// Menu entrypoints called from module_start / the per-frame hook.
//   build() - one-time setup on the game thread (init ui vars, register submenus)
//   tick()  - once per frame on the script thread (input, update, render)
namespace menu {
    void build();
    void tick();
}
