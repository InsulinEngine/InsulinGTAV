cmake_minimum_required(VERSION 3.20)
project(InsulinGTAV CXX)

if(NOT DEFINED ENV{OO_PS4_TOOLCHAIN})
    message(FATAL_ERROR "OO_PS4_TOOLCHAIN environment variable is not set")
endif()

# Compile the whole source tree. GLOB_RECURSE + CONFIGURE_DEPENDS re-globs when
# files are added/removed so new menu files are picked up without editing this.
file(GLOB_RECURSE INSULIN_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_orbis_target(InsulinGTAV
    TYPE GHPLUGIN
    SOURCES ${INSULIN_SOURCES}
    # The GHPLUGIN defaults plus SceNet, for the companion server's socket.
    # LIBS replaces the default list rather than extending it, and setting
    # ORBIS_LIBS from out here does nothing: add_orbis_target runs
    # cmake_parse_arguments with LIBS as a keyword, which clears the
    # caller's ORBIS_LIBS inside the function before the default kicks in.
    LIBS SceLibcInternal kernel SceSysmodule GoldHEN_Hook SceNet
)

target_include_directories(InsulinGTAV PRIVATE src)

# Newer ld.lld (LLVM 18) defaults to -z separate-code, which splits headers into
# their own vaddr-0 segment and pushes .text to 0x4000. create-fself then drops
# the empty header segment, leaving the module based at 0x4000 -- and GoldHEN's
# loader only accepts modules based at 0, so a 0x4000 module silently fails to
# load and module_start never runs (no notify, no log). Force the pre-18
# single-segment layout so the module base is 0. (Root cause + fix proven in
# ../OrbisLibServer; we build with clang/lld 18.1.8.)
target_link_options(InsulinGTAV PRIVATE "LINKER:--no-rosegment")

# GHPLUGIN links SceLibcInternal, not libc++: use the project-local mini-STL.
target_compile_definitions(InsulinGTAV PRIVATE STL_FREESTANDING)
target_compile_options(InsulinGTAV PRIVATE
    -std=c++17
    -fno-exceptions
    -fno-rtti
    -Wno-invalid-offsetof
    # The menu runs single-threaded on the game script thread, so guarded
    # function-local statics are unnecessary -- and dropping them removes the
    # __cxa_guard_acquire/release import dependency (one less symbol that must
    # resolve inside the game process at load time).
    -fno-threadsafe-statics
)
