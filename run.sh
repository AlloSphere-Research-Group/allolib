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

# resolve flags ###############################################################

# check if we want debug build
BUILD_TYPE=Release
while getopts ":dn" opt; do
  case $opt in
  d)
  BUILD_TYPE=Debug
  POSTFIX=_debug
  shift # consume option
    ;;
  n)
  EXIT_AFTER_BUILD=1
  shift
    ;;
  esac
done
echo "BUILD TYPE: ${BUILD_TYPE}"

# first build allolib ###########################################################
echo " "
echo "___ building allolib __________"
echo " "

cd ${AL_LIB_PATH}
git submodule init
git submodule update
mkdir -p build
cd build
mkdir -p "${BUILD_TYPE}"
cd "${BUILD_TYPE}"
cmake ../.. -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
make
LIB_BUILD_RESULT=$?
# if lib failed to build, exit
if [ ${LIB_BUILD_RESULT} != 0 ]; then
  exit 1
fi

# then build the app ###########################################################

APP_FILE_INPUT="$1" # first argument (assumming we consumed all the options above)
APP_PATH=$(dirname ${APP_FILE_INPUT})
APP_FILE=$(basename ${APP_FILE_INPUT})
APP_NAME=${APP_FILE%.*} # remove extension (once, assuming .cpp)

echo " "
echo "___ building ${APP_NAME} __________"
echo " "

# echo "app path: ${APP_PATH}"
# echo "app file: ${APP_FILE}"
# echo "app name: ${APP_NAME}"

cd ${INITIALDIR}
cd ${APP_PATH}
mkdir -p build
cd build
mkdir -p ${APP_NAME}
cd ${APP_NAME}

cmake ${AL_LIB_PATH}/cmake/single_file -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -Dal_path=${AL_LIB_PATH} -DAL_APP_FILE=../../${APP_FILE}
make
APP_BUILD_RESULT=$?
# if app failed to build, exit
if [ ${APP_BUILD_RESULT} != 0 ]; then
  exit 1
fi

if [ ${EXIT_AFTER_BUILD} ]; then
  exit 0
fi

# run app ######################################################################
# go to where the binary is so we have cwd there
# (app's cmake is set to put binary in 'bin')
cd ${INITIALDIR}
cd ${APP_PATH}/bin
echo " "
echo "___ running ${APP_NAME} __________"
echo " "
./"${APP_NAME}${POSTFIX}"
