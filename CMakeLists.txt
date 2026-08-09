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
)

target_include_directories(InsulinGTAV PRIVATE src)

# GHPLUGIN links SceLibcInternal, not libc++: use the project-local mini-STL.
target_compile_definitions(InsulinGTAV PRIVATE STL_FREESTANDING)
target_compile_options(InsulinGTAV PRIVATE
    -std=c++17
    -fno-exceptions
    -fno-rtti
    -Wno-invalid-offsetof
)
