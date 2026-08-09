#!/bin/bash
set -e
cd "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [ -z "$OO_PS4_TOOLCHAIN" ]; then
    echo "OO_PS4_TOOLCHAIN environment variable is not set. See the OpenOrbis PS4 Toolchain README." >&2
    exit 1
fi

if [ "$1" = "clean" ]; then
    rm -rf build
    echo "Cleaned."
    exit 0
fi

cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=cmake/oo-ps4-toolchain.cmake
cmake --build build