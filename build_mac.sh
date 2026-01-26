#! /bin/sh
cd src/libwebm
cmake -B build -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" -DENABLE_WEBM_PARSER=ON
cmake --build build

cd ../../
scons compiledb=yes
