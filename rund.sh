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
  POSTFIX=_debug
  shift # consume option
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
APP_NAME="$1" # first argument (assumming we consumed all the options above)

if [ ${APP_NAME} == "." ]; then
  # if '.' was given for the app directory,
  # it means script was run from app path
  # so initial dir has the app path
  # and it's basename will be app name
  APP_NAME=${INITIALDIR}
  APP_PATH=${APP_NAME}
else
  APP_PATH=${INITIALDIR}/${APP_NAME}
fi

# discard '/'' in the end of the input directory (if it has)
LASTCHAR=${APP_NAME:(-1)}
if [ ${LASTCHAR} == "/" ]; then
    # ${string%substring}
    # Strips shortest match of $substring from back of $string.
    APP_NAME=${APP_NAME%/}
fi

# get only last foldername
APP_NAME=$(basename ${APP_NAME})

echo " "
echo "___ building ${APP_NAME} __________"
echo " "

echo "app path: ${APP_PATH}"
cd ${APP_PATH}
mkdir -p build
cd build
# if app is run with this script, al_path is set here
# if this script was not used, cmake will set it to value user provided
cmake ${APP_PATH}/ -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -Dal_path=${AL_LIB_PATH}
make
APP_BUILD_RESULT=$?
# if app failed to build, exit
if [ ${APP_BUILD_RESULT} != 0 ]; then
	exit 1
fi

# run app ######################################################################
# go to where excutable is so we have cwd there
# (app's cmake is set to put binary in 'bin')
cd ${APP_PATH}/bin
echo " "
echo "___ running ${APP_NAME} __________"
echo " "
./"${APP_NAME}${POSTFIX}"