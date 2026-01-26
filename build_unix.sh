#! /bin/sh
cd src/libwebm
cmake -B build -DENABLE_WEBM_PARSER=ON
cmake --build build

cd ../../
scons compiledb=yes
