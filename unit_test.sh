#! /bin/bash

rm -rf ./build

# build for target host
cmake . -B build -DCMAKE_TOOLCHAIN_FILE=linux.cmake

cd ./build

cmake --build . --target sample
ctest



