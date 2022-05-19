#! /bin/bash

rm -rf ./build

cmake . -B build -DCMAKE_TOOLCHAIN_FILE=ts7200.cmake

cd ./build

cmake --build . --target kernel