#! /bin/sh
cd src/libwebm
cmake -B build -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" -DENABLE_WEBM_PARSER=ON
cmake --build build

cd ../../
cd src/libopus

# Turn off NEON support because of x86_64, if this causes issues or can be resolved
# then make a PR! I just couldn't figure it out another way myself.
cmake -B build -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" -DCOMPILER_SUPPORT_NEON=OFF
cmake --build build

cd ../../
scons compiledb=yes
