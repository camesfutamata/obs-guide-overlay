#!/bin/bash
export PATH="/mingw64/bin:/usr/bin:$PATH"
cd /c/Users/Denis/Desktop/obs_nrs

OBS_DIR="/c/Program Files/obs-studio"

# Try CMake build
cmake -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DLIBOBS_INCLUDE_DIR="/c/Users/Denis/Desktop/obs_nrs/obs_include" \
  -DLIBOBS_LIB_DIR="/c/Users/Denis/Desktop/obs_nrs/obs_include" \
  -S . -B build2 2>&1

echo "--- cmake exit: $? ---"

if [ -f build2/build.ninja ]; then
  ninja -C build2 2>&1
  echo "--- ninja exit: $? ---"
  ls -la obs-guide-overlay*.dll 2>/dev/null || echo "No DLL found"
fi
