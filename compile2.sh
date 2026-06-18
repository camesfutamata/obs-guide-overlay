#!/bin/bash
export PATH="/mingw64/bin:/usr/bin:$PATH"
gcc -O2 -shared -fPIC \
  -I /c/Users/Denis/Desktop/obs_nrs/obs_include/libobs \
  -I /c/Users/Denis/Desktop/obs_nrs/obs_include \
  /c/Users/Denis/Desktop/obs_nrs/src/guide-overlay.c \
  "/c/Program Files/obs-studio/bin/64bit/obs.dll" \
  "/c/Program Files/obs-studio/bin/64bit/obs-frontend-api.dll" \
  -o /c/Users/Denis/Desktop/obs_nrs/releases/v1.0.1/obs-guide-overlay.dll 2>&1
