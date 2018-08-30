
# Allolib C/C++ Libraries
Developed by:

AlloSphere Research Group

University of California, Santa Barbara

[![Build Status](https://travis-ci.org/AlloSphere-Research-Group/allolib.svg?branch=master)](https://travis-ci.org/AlloSphere-Research-Group/allolib)
[![Build status](https://ci.appveyor.com/api/projects/status/c311nw14jmwq9lv1?svg=true)](https://ci.appveyor.com/project/mantaraya36/allolib)

# Documentation

The API documentation can be found at: https://allosphere-research-group.github.io/allolib-doc/

# Installing Allolib

    git clone https://github.com/AlloSphere-Research-Group/allolib
    cd allolib
    git submodule init
    git submodule update
    ./run.sh examples/graphics/2d.cpp

# Running allolib

## On *nix systems (Linux and Mac OS)

The simplest way to compile and run a single file in allolib is by using the run.sh script:

    ./run.sh path/to/file.cpp

This will build allolib, and create an executable for the file.cpp called 'file' inside the '''path/to/bin''' directory. It will then run the application.

You can add a file called '''flags.cmake''' in the '''path/to/''' directory which will be added to the build scripts. Here you can add dependencies, include directories, linking and anything else that cmake could be used for. See the example in '''examples/user_flags'''.

For more complex projects follow the example provided in the empty/project directory. This requires writing a CMakeLists.txt to specify sources, dependencies and linkage.

## On Windows systems

Open cmake-gui and create a Visual Studio project for Allolib. Build it. This will create the library you can then use in your project, but you will have to add libraries and directories manually.

The run.sh script will not work for Windows.

An alternative is to use the 'empty_project/' template to start your project by copying it to the directory where allolib/ resides (i.e. move it outside the source tree). Then import the CMakeLists.txt project to visual studio. You will need to install full cmake support in VS. The generated project should build allolib and your project.

#TODO

- fullscreen with specific monitor

#DEPENDENCIES

- GLFW
- GLEW
- pkg-config (ubuntu and macos, for glfw)
- freeimage (optional)
- freetype (optional)
- assimp (optional)

NOTE

For visual studio, cmake with version at least 3.8 is required. Version 3.8 has a feature for setting the working directory of Visual Studio project within cmake script. Otherwise, user will have to set it manually at

`project -> properties -> debugging -> working directory`

setting it to $(OutDir) will do it correctly (where compiler outputs executable)
