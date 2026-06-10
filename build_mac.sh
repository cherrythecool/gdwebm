#! /bin/sh
cd src/libwebm
cmake -B build -DENABLE_WEBM_PARSER=ON
cmake --build build --config Release
cd ../../

cd src/libopus
cmake -B build
cmake --build build --config Release
cd ../../

cd src/libdav1d
mkdir -p build && cd build
meson setup .. --default-library=static --buildtype=release
ninja
cd ../../../

scons compiledb=yes arch=arm64
