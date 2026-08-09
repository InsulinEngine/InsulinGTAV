#include "game_thread.h"
#include "../rage/invoker/invoker.h"
#include "rage/invoker/natives.h"

#include <GoldHEN/Detour.h>
#include <stdint.h>

// No STL: the plugin links SceLibcInternal without libc++. Synchronisation uses
// clang's __atomic builtins (word-sized, lock-free on x86-64, no runtime lib).

namespace game {

	// GET_PLAYER_PED impl (confidence: verified). The base game's scripts call
	// it many times per frame on the script thread, so detouring it gives a
	// reliable per-frame, correct-thread callback. If on-console testing shows
	// the queue never drains, swap this for another verified per-frame native.
	static const uint64_t HOOK_RVA = 0xAA9310;

	typedef int64_t (*native_fn)(rage::invoker::native_context*);
	typedef void (*task_fn)();

	static const int QUEUE_CAP = 64;

	static Detour g_detour;
	static bool g_installed = false;

	static task_fn g_queue[QUEUE_CAP];
	static int g_queue_count = 0;
	static volatile int g_lock = 0;      // spinlock guarding the queue
	static volatile int g_draining = 0;  // re-entrancy guard for the detour

	static frame_fn g_frame_cb = nullptr;
	static int g_last_frame = -1;

	static inline void lock() {
		while (__atomic_exchange_n(&g_lock, 1, __ATOMIC_ACQUIRE)) { }
	}
	static inline void unlock() {
		__atomic_store_n(&g_lock, 0, __ATOMIC_RELEASE);
	}

	static void drain_queue() {
		// One drainer at a time: queued work may call the hooked native and
		// re-enter this detour on the same thread.
		if (__atomic_exchange_n(&g_draining, 1, __ATOMIC_ACQ_REL))
			return;

		task_fn local[QUEUE_CAP];
		int n;
		lock();
		n = g_queue_count;
		for (int i = 0; i < n; ++i)
			local[i] = g_queue[i];
		g_queue_count = 0;
		unlock();

		for (int i = 0; i < n; ++i)
			local[i]();

		__atomic_store_n(&g_draining, 0, __ATOMIC_RELEASE);
	}

	static int64_t player_ped_hook(rage::invoker::native_context* ctx) {
		drain_queue();

		// g_last_frame is updated BEFORE g_frame_cb() runs, so if the callback
		// itself calls the hooked native (get_player_ped) the resulting
		// re-entrant call to this detour sees frame == g_last_frame and skips
		// calling g_frame_cb() again - one bounded extra recursion, not a loop.
		if (g_frame_cb) {
			int frame = native::get_frame_count();
			if (frame != g_last_frame) {
				g_last_frame = frame;
				g_frame_cb();
			}
		}

		return Detour_Stub(&g_detour, native_fn, ctx);
	}

	void run_on_game_thread(task_fn fn) {
		lock();
		if (g_queue_count < QUEUE_CAP)
			g_queue[g_queue_count++] = fn;
		unlock();
	}

	void set_frame_callback(frame_fn fn) { g_frame_cb = fn; }

	bool install_frame_hook() {
		if (g_installed)
			return true;
		if (!rage::invoker::g_eboot_base)
			return false;

		Detour_Construct(&g_detour, DetourMode_x64);
		void* stub = Detour_DetourFunction(
			&g_detour,
			rage::invoker::g_eboot_base + HOOK_RVA,
			(void*)&player_ped_hook);

		g_installed = (stub != nullptr);
		return g_installed;
	}
}
