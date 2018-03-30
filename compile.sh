#!/bin/bash

INITIALDIR=${PWD}
# echo "Script executed from: ${INITIALDIR}"

# BASH_SOURCE has the script's path
# could be absolute, could be relative
SCRIPT_PATH=$(dirname ${BASH_SOURCE})

FIRSTCHAR=${SCRIPT_PATH:0:1}
if [ ${FIRSTCHAR} == "/" ]; then
  # it's asolute path
  AL_LIB_PATH=${SCRIPT_PATH}
else
  # SCRIPT_PATH was relative
  AL_LIB_PATH=${INITIALDIR}/${SCRIPT_PATH}
fi

# check if we want debug build
BUILD_TYPE=Release
while getopts ":d" opt; do
  case $opt in
  d)
  BUILD_TYPE=Debug
  shift # consume option
    ;;
  esac
done

# Get the number of processors on OS X; Linux; or MSYS2, or take a best guess.
NPROC=$(grep --count ^processor /proc/cpuinfo 2>/dev/null || sysctl -n hw.ncpu || nproc || echo 2)
# Save one core for the gui.
PROC_FLAG=$((NPROC - 1))

cd ${AL_LIB_PATH}
git submodule init
git submodule update
mkdir -p build
cd build
mkdir -p "${BUILD_TYPE}"
cd "${BUILD_TYPE}"
cmake ../.. -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
make -j$PROC_FLAG