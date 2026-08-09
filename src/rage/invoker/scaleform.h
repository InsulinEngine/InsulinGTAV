#pragma once
#include "invoker.h"

// Scaleform natives for the instructional-buttons bar. These are ABSENT from the
// generated natives.h (or unreliable there), so they are hand-wrapped from RVAs
// verified against the eboot on 2026-08-01. See
// docs/superpowers/notes/2026-08-01-scaleform-natives-validation.md.
//
// Void-returning natives are called through invoke<int> and the result ignored
// (harmless: the return slot is either written-and-discarded or reads 0).
namespace sf {
	using rage::invoker::invoke;

	inline int  request_scaleform_movie(const char* name) { return invoke<int>(0x9D0CC0, name); }
	inline bool has_scaleform_movie_loaded(int handle)    { return invoke<bool>(0x9D0D80, handle); }
	inline bool begin_method(int handle, const char* method) { return invoke<bool>(0x9D16D0, handle, method); }
	inline void add_param_int(int v)                      { invoke<int>(0x9D1990, v); }
	inline void add_param_float(float v)                  { invoke<int>(0x9D19A0, v); }
	inline void begin_text(const char* type)              { invoke<int>(0x9D19E0, type); }
	inline void add_text_string(const char* s)            { invoke<int>(0x9E08F0, s); }
	inline void end_text()                                { invoke<int>(0x9D19F0); }
	inline void end_method()                              { invoke<int>(0x9D1870); }  // canonical void END (crossmap)
	inline void draw_fullscreen(int handle, int r, int g, int b, int a) {
		invoke<int>(0x9D11F0, handle, r, g, b, a, 0);
	}
}
