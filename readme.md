
# Allolib C/C++ Libraries
Developed by:

AlloSphere Research Group

University of California, Santa Barbara

[![Build Status](https://travis-ci.org/AlloSphere-Research-Group/allolib.svg?branch=master)](https://travis-ci.org/AlloSphere-Research-Group/allolib)
[![Build status](https://ci.appveyor.com/api/projects/status/c311nw14jmwq9lv1?svg=true)](https://ci.appveyor.com/project/mantaraya36/allolib)

# Documentation

The API documentation can be found at: https://allosphere-research-group.github.io/allolib-doc/

# Installing Allolib

## Dependencies

Allolib only depends on Cmake version 3.8 (as the build tool), OpenGL and glew.

## Building library and running examples
On a bash shell on Windows, Linx and OS X do:

    git clone https://github.com/AlloSphere-Research-Group/allolib
    cd allolib
    git submodule init
    git submodule update
    ./run.sh examples/graphics/2d.cpp

# Running allolib

## On Bash shell

The simplest way to compile and run a single file in allolib is by using the run.sh script:

    ./run.sh path/to/file.cpp

This will build allolib, and create an executable for the file.cpp called 'file' inside the '''path/to/bin''' directory. It will then run the application.

You can add a file called '''flags.cmake''' in the '''path/to/''' directory which will be added to the build scripts. Here you can add dependencies, include directories, linking and anything else that cmake could be used for. See the example in '''examples/user_flags'''.

For more complex projects follow the example provided in the empty/project directory. This requires writing a CMakeLists.txt to specify sources, dependencies and linkage.

#TODO

- fullscreen with specific monitor

# Optional Dependencies

- freeimage (optional)
- freetype (optional)
- assimp (optional)

If these are avaialble, the classes that use them will be built. A simple alternative is to use the image/font/asset loading provided in the modeules directory. These classes depend on header only libraries that are included in these sources.

