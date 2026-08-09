#pragma once

// std::initializer_list is compiler-backed (the compiler emits it for braced
// lists regardless of the standard library), so this header is safe even
// without libc++. Ozark's color_rgba::as_initializer_list() relies on it.
#include <initializer_list>
