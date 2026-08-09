// C++ runtime pieces the GHPLUGIN lacks (it links SceLibcInternal, not libc++).
// Providing them locally keeps the module from importing operator new/delete and
// the guard/pure-virtual helpers, so nothing here has to resolve inside the game
// process at load time. The menu and all function-local static singletons are
// constructed on the module-start / game thread before the frame callback runs,
// and the menu is single-threaded thereafter, so the guards never race (and with
// -fno-threadsafe-statics they aren't even emitted -- these are belt-and-braces).
#include <stddef.h>
#include <stdlib.h>

void* operator new(size_t n) { return malloc(n); }
void* operator new[](size_t n) { return malloc(n); }
void  operator delete(void* p) noexcept { free(p); }
void  operator delete[](void* p) noexcept { free(p); }
void  operator delete(void* p, size_t) noexcept { free(p); }
void  operator delete[](void* p, size_t) noexcept { free(p); }

extern "C" {
    // Itanium ABI guard variable is 8 bytes; byte 0 = "initialized".
    int  __cxa_guard_acquire(long long* g) { return *((char*)g) == 0; }
    void __cxa_guard_release(long long* g) { *((char*)g) = 1; }
    void __cxa_guard_abort(long long*) {}
    void __cxa_pure_virtual() { for (;;) {} }   // never call a pure virtual
}
