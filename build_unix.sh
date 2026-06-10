#! /bin/sh
cd src/libwebm
cmake -B build -DENABLE_WEBM_PARSER=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build --config Release
cd ../../

cd src/libopus
cmake -B build -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build --config Release
cd ../../

cd src/libdav1d
mkdir -p build && cd build
meson setup .. --default-library=static --buildtype=release
ninja
cd ../../../

scons compiledb=yes
