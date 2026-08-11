#pragma once

// PS-button / ShellUI overlay detection via sceSystemServiceGetStatus.
//
// While the system UI is overlaid (user pressed the PS button) the game is
// "constrained": issuing script draw/scaleform natives from our per-frame hook
// in that state crashes the title. menu::tick() polls overlaid() first thing
// each frame and skips ALL native work while it returns true.
//
// The symbol is resolved at runtime out of the already-loaded
// libSceSystemService module (sceKernelGetModuleList + sceKernelDlsym), so the
// plugin gains no new load-time import. If resolution fails, overlaid() is
// permanently false (fail-open: menu keeps working, PS-button guard inactive)
// and one Err line is logged.
namespace platform::system_ui {
    // True while the ShellUI overlay or background execution is active.
    // Logs one line on every state transition (enter/leave).
    bool overlaid();
}
