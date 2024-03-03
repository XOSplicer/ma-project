#! /usr/bin/env bash

set -ex
mkdir -p build
cd build
CC=clang CXX=clang++ cmake .. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_COMPILER=$CXX \
    -DCMAKE_C_COMPILER=$CC \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DPHASAR_BUILD_UNITTESTS=ON \
    -DPHASAR_BUILD_DOC=OFF \
    -DPHASAR_BUILD_OPENSSL_TS_UNITTESTS=OFF \
    -G Ninja
cmake --build .

# I don't know how to get those file to be copied in CMake
cp ./cargo/build/x86_64-unknown-linux-gnu/debug/deps/*.ll ./analysis-targets/
