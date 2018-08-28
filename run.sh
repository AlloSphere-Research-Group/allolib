#!/bin/bash
usage="$(basename "$0") [-h] [-d] [-n] [-c] [-v] source.cpp
-- build and run single-file allolib applications.

where:
    -h  show this help text
    -d build debug version and run in debugger
    -n build only. Don't run.
    -c clean build
    -v verbose build. Prints full cmake log and verbose build.
"

if [ $(uname -s) == "Darwin" ]; then
  CURRENT_OS="MACOS"
  # echo "running on macOS"
fi

if [ $(uname -s) == "Linux" ]; then
  CURRENT_OS="LINUX"
fi

INITIALDIR=${PWD} # gives absolute path
# echo "Script executed from: ${INITIALDIR}"

# BASH_SOURCE has the script's path
# could be absolute, could be relative
SCRIPT_PATH=$(dirname ${BASH_SOURCE[0]})

FIRSTCHAR=${SCRIPT_PATH:0:1}
if [ ${FIRSTCHAR} == "/" ]; then
  # it's asolute path
  AL_LIB_PATH=${SCRIPT_PATH}
else
  # SCRIPT_PATH was relative
  AL_LIB_PATH=${INITIALDIR}/${SCRIPT_PATH}
fi

# Get the number of processors on OS X; Linux; or MSYS2, or take a best guess.
NPROC=$(grep --count ^processor /proc/cpuinfo 2>/dev/null || sysctl -n hw.ncpu || nproc || echo 2)
# Save one core for the gui.
PROC_FLAG=$((NPROC - 1))

# resolve flags
BUILD_TYPE=Release
DO_CLEAN=0
IS_VERBOSE=0
VERBOSE_FLAG=OFF
RUN_MPI=0
RUN_APP=1

if [ $# == 0 ]; then
  echo "$usage"
  exit 1
fi

while getopts "adncvh" opt; do
  case "${opt}" in
  a)
    RUN_MPI=1
    RUN_APP=0
    ;;
  d)
    BUILD_TYPE=Debug
    POSTFIX=d # if release, there's no postfix
    ;;
  n)
    RUN_APP=0
    ;;
  c)
    DO_CLEAN=1
    ;;
  v)
    IS_VERBOSE=1
    VERBOSE_FLAG=ON
    ;;
  h) echo "$usage"
  exit
  ;;
  \?) echo "$usage" >&2
    exit 1
    ;;
  esac
done
# consume options that were parsed
shift $(expr $OPTIND - 1 )

if [ ${IS_VERBOSE} == 1 ]; then
  echo "BUILD TYPE: ${BUILD_TYPE}"
fi

APP_FILE_INPUT="$1" # first argument (assumming we consumed all the options above)
APP_PATH=$(dirname ${APP_FILE_INPUT})
APP_FILE=$(basename ${APP_FILE_INPUT})
APP_NAME=${APP_FILE%.*} # remove extension (once, assuming .cpp)

# echo "app path: ${APP_PATH}"
# echo "app file: ${APP_FILE}"
# echo "app name: ${APP_NAME}"

(
  cd ${APP_PATH}
  if [ ${DO_CLEAN} == 1 ]; then
    if [ ${IS_VERBOSE} == 1 ]; then
      echo "cleaning build"
    fi
    rm -r build
  fi
  mkdir -p build
  cd build
  mkdir -p ${APP_NAME}
  cd ${APP_NAME}
  mkdir -p ${BUILD_TYPE}
  cd ${BUILD_TYPE}

  cmake -Wno-deprecated -DBUILD_EXAMPLES=0 -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DAL_APP_FILE=../../../${APP_FILE} -DAL_VERBOSE_OUTPUT=${VERBOSE_FLAG} -DAL_RUN_APP=${RUN_APP} ${AL_LIB_PATH}/cmake/single_file > cmake_log.txt
  cmake --build .
)

APP_BUILD_RESULT=$?
# if app failed to build, exit
if [ ${APP_BUILD_RESULT} != 0 ]; then
  echo "app ${APP_NAME} failed to build"
  exit 1
fi

# run app
# go to where the binary is so we have cwd there
# (app's cmake is set to put binary in 'bin')
cd ${INITIALDIR}
cd ${APP_PATH}/bin

if [ "${CURRENT_OS}" = "MACOS" ]; then
  DEBUGGER="lldb -o run -ex "
else
  DEBUGGER="gdb -ex run "
fi

if [ ${RUN_MPI}  != 0 ]; then
  echo Running MPI
  mpirun --mca btl_tcp_if_include enp1s0 --hostfile ../../mpi_hosts.txt /usr/bin/env DISPLAY=:0 ./"${APP_NAME}${POSTFIX}"
fi
# elif [ ${BUILD_TYPE} == "Release" ]; then
#   ./"${APP_NAME}${POSTFIX}"
# else
#   ${DEBUGGER} ./"${APP_NAME}${POSTFIX}"
# fi
