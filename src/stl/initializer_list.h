#pragma once

// std::initializer_list is compiler-backed (the compiler emits it for braced
// lists regardless of the standard library), so this header is safe even
// without libc++. stl::vector's braced-init constructor needs the type, and
// stdafx pulls this in for everything that constructs one.
#include <initializer_list>
