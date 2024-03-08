#!/bin/bash -e

cmake -B build -D CMAKE_BUILD_TYPE=Debug || exit 1
cmake --build build --config Debug || exit 1

if [ "$1" == "--notrun" ]; then
    echo "Compiled executable will not be launched."
    exit 0
fi
./build/DDPG-Oligopolies-Simulator


