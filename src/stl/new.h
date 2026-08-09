#pragma once
#include <stddef.h>

// The plugin build (STL_FREESTANDING) has no <new>; provide placement new.
// The PC test build uses the host's <new>.
#if defined(STL_FREESTANDING)
inline void* operator new(size_t, void* p) noexcept { return p; }
inline void* operator new[](size_t, void* p) noexcept { return p; }
inline void  operator delete(void*, void*) noexcept {}
inline void  operator delete[](void*, void*) noexcept {}
#else
#include <new>
#endif
