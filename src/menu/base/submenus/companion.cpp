#include "menu/base/submenus/companion.h"
#include "menu/base/submenus/misc.h"
// submenu.h deliberately pulls in only options/option.h, so each submenu
// includes the concrete option types it actually uses.
#include "menu/base/options/toggle.h"
#include "menu/base/options/break.h"
#include "net/server.h"
#include "net/jobs.h"
#include "game/player_valid.h"
#include "game/teleport_fsm.h"
#include "game/teleport_actions.h"
#include "platform/log.h"
#include "platform/paths.h"

#include <stdio.h>
#include <string.h>

namespace {

    bool     g_enabled = false;      // what the toggle says
    bool     g_started = false;      // what the server actually is
    char     g_pin[5]  = "0000";
    unsigned g_port    = 8080;

    // The snapshot the HTTP thread reads. Written by the game thread once per
    // frame, read by the HTTP thread at any moment. Torn reads are acceptable
    // here - the worst case is a marker one frame stale - and the alternative
    // (a lock on the render path) is not worth it.
    struct snapshot {
        volatile bool  valid;
        volatile float x, y, z, heading;
    };
    snapshot g_snap = { false, 0.0f, 0.0f, 0.0f, 0.0f };

    game::tp::machine g_teleporter;

    void make_pin() {
        // No RNG dependency: the address is randomised per boot and the frame
        // count at enable time is unpredictable enough for the job, which is
        // stopping accidents, not attackers.
        unsigned seed = (unsigned)(unsigned long long)&g_snap;
        for (int i = 0; i < 4; i++) {
            seed = seed * 1103515245u + 12345u;
            g_pin[i] = (char)('0' + (seed >> 16) % 10);
        }
        g_pin[4] = 0;
    }
}

unsigned companion_state_json(char* out, unsigned cap) {
    if (!g_snap.valid) return 0;
    int n = snprintf(out, cap,
                     "{\"x\":%.2f,\"y\":%.2f,\"z\":%.2f,\"heading\":%.1f}",
                     g_snap.x, g_snap.y, g_snap.z, g_snap.heading);
    // snprintf reports what it would have written; state_fn must report what
    // fit. Anything that did not fit is a failure, not a truncated reply.
    return (n < 0 || (unsigned)n >= cap) ? 0u : (unsigned)n;
}

void companion_menu::load() {
    set_name("Companion Server");
    set_parent<misc_menu>();

    // State only. Starting the server here would run during build(), which can
    // happen while the game is still loading.
    add_option(toggle_option("Enable Server")
        .add_toggle(g_enabled)
        .add_tooltip("Serves a page on this console over the local network. "
                     "Open http://<console ip>:8080 and enter the PIN below.")
        .add_savable(get_submenu_name_stack()));

    // The status line. There is no set_status on the submenu base; a break
    // option that rewrites its own name from add_update is how this codebase
    // shows live text (break.cpp calls m_on_update(this) from render). The
    // lambda captures nothing, which keeps it far inside stl::function's
    // 64-byte cap.
    add_option(break_option("Stopped")
        .add_update([](break_option* o) {
            char line[96];
            if (net::server_running()) {
                snprintf(line, sizeof(line), "Running on port %u - PIN %s",
                         (unsigned)net::server_port(), g_pin);
            } else {
                snprintf(line, sizeof(line), "Stopped");
            }
            o->set_name(line);
        }));
}

void companion_menu::feature_update() {
    // Start and stop follow the toggle, but only from here - never from load().
    if (g_enabled && !g_started) {
        make_pin();
        if (net::server_start((unsigned short)g_port,
                              OZARK_WEB,
                              g_pin, companion_state_json)) {
            g_started = true;
            char msg[96];
            snprintf(msg, sizeof(msg), "Companion server on :%u, PIN %s", g_port, g_pin);
            platform::notify(msg);
        } else {
            g_enabled = false;         // do not retry every frame
            platform::notify("Companion server failed to start");
        }
    } else if (!g_enabled && g_started) {
        net::server_stop();
        g_started = false;
    }

    // Deliberately above the `!g_started` gate below: if the server is turned
    // off mid-teleport, the player can already be faded to black at 1000m.
    // The machine's own frame counters are what get them back down and faded
    // in (streaming/resolving timeouts, then fade_in in release()); stopping
    // this tick when the server stops would freeze those counters and strand
    // the player on a black screen until the server is re-enabled. Finishing
    // an in-flight teleport is intended even with the server off - do not
    // move this back into the block below.
    g_teleporter.tick(game::live_actions());

    if (!g_started) return;

    // Publish the snapshot the HTTP thread serves. Pure memory reads through
    // the same chain player_valid() uses; no native is involved.
    if (game::player_valid()) {
        const float* m = game::local_player_matrix();
        if (m) {
            g_snap.x = m[12];
            g_snap.y = m[13];
            g_snap.z = m[14];
            g_snap.heading = game::local_player_heading();
            g_snap.valid = true;
        }
    }

    // Take at most one new job per frame, and only while nothing is in
    // flight already - the tick above already advanced whatever was pending.
    if (!g_teleporter.busy()) {
        net::job j;
        if (net::jobs().pop(&j) && j.kind == net::job_kind::teleport) {
            game::tp::request r = { j.x, j.y, j.z, j.ground };
            g_teleporter.submit(r);
            platform::logf("net", "teleport to %.1f %.1f ground=%d",
                           j.x, j.y, (int)j.ground);
        }
    }
}

companion_menu* companion_menu::get() {
    static companion_menu instance;
    return &instance;
}
