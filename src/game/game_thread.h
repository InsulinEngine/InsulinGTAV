#pragma once

// Runs work on GTA's script thread. Native calls that touch engine state are
// only safe there; calling them from an arbitrary thread crashes. We get onto
// the script thread by detouring a native impl the base game invokes every
// frame, and draining a queue from inside that detour.
//
// Callbacks are plain function pointers, not std::function: the GoldHEN plugin
// links SceLibcInternal without libc++, so the STL is unavailable here.

namespace game {

	// Queue a callback to run on the script thread on the next frame. Safe to
	// call from any thread. The callback may itself invoke natives.
	void run_on_game_thread(void (*fn)());

	// Install the per-frame detour. Requires rage::invoker::g_eboot_base to be
	// resolved first. Returns false if the hook could not be installed.
	bool install_frame_hook();

	// A persistent callback invoked once per NEW frame (deduped via
	// native::get_frame_count()) from inside the frame-hook detour, after the
	// queued work has drained. Set once at load; call again to replace it.
	typedef void (*frame_fn)();
	void set_frame_callback(frame_fn fn);
}
