#!/bin/bash

rm -r build/ || true
mkdir build || true

cmake -DCMAKE_C_COMPILER:FILEPATH=/usr/bin/clang-13 -DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/clang++-13 . -Bbuild/ -G "Ninja"
cmake --build build/ --config Release --target all -j 8
