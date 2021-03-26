#!/bin/sh
# 
# Cross compilation for Windows
# This works on Arch Linux out of the box, just install mingw-w64-cmake from
# AUR. (e.g. with yaourt -ySa mingw-w64-cmake)
#
# For other distros, having a look at Arch's package might be useful:
# https://aur.archlinux.org/cgit/aur.git/tree/?h=mingw-w64-cmake

set -x -e

rm -rf ${BUILD_DIR}
mkdir ${BUILD_DIR}
cd ${BUILD_DIR}

${TARGET}-cmake -DCMAKE_EXE_LINKER_FLAGS="-static" -DCMAKE_BUILD_TYPE="Release" ..
make
