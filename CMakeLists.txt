cmake_minimum_required(VERSION 3.20)
project(InsulinGTAV CXX)

if(NOT DEFINED ENV{OO_PS4_TOOLCHAIN})
    message(FATAL_ERROR "OO_PS4_TOOLCHAIN environment variable is not set")
endif()

add_orbis_target(InsulinGTAV
    TYPE GHPLUGIN
    SOURCES src/InsulinGTAV.cpp
)
