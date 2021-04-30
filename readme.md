
# Allolib C/C++ Libraries  {#mainpage}
Developed by:

AlloSphere Research Group

allolib is a fork of [AlloSystem](https://github.com/AlloSphere-Research-Group/AlloSystem), maintained in separate repository.

University of California, Santa Barbara

[![Build Status](https://travis-ci.org/AlloSphere-Research-Group/allolib.svg?branch=master)](https://travis-ci.org/AlloSphere-Research-Group/allolib)
[![Build status](https://ci.appveyor.com/api/projects/status/c311nw14jmwq9lv1?svg=true)](https://ci.appveyor.com/project/mantaraya36/allolib)

Find us at #allolib on libera.chat. Try the [libera.chat web interface](https://web.libera.chat/) interface if you don't have an IRC client

# Documentation

The API documentation can be found at: https://allosphere-research-group.github.io/allolib-doc/

# Installing Allolib

## Dependencies

Allolib depends on Cmake version 3.8 (as the build tool), OpenGL and glew. See platform specific instructions below.

### Windows

There are two paths for Windows installation. One through Visual Studio and one through the Chocolatey package manager.

For installation through Visual Studio:

 1. Install Visual Studio 2017 or 2019 Community Edition from https://visualstudio.microsoft.com/downloads/
 2. During installation options:

    a. Install "Desktop development with C++" workload

    b. Install Individual components: C++/CLI support (make sure the version matches your compiler version), Git for Windows, Visual C++ Tools for Cmake

For installation through Chocolatey (not currently recommended, but let us know if it works):

 * Install Chocolatey: Aim your browser at https://chocolatey.org/install. Follow the directions there to install Chocolatey. Wait for this to finish before moving on.
 * Use the choco command to install a C++ compiler. Open cmd.exe (Command Prompt) as administrator and run this command: choco install -y visualstudio2017buildtools visualstudio2017-workload-vctools
 * Use the choco command to install some software. Open cmd.exe (Command Prompt) as administrator and run this command: choco install -y git git-lfs cmake graphviz doxygen atom vscode
 * Install libsndfile: Aim your browser at http://www.mega-nerd.com/libsndfile/#Download. Download and install the 64-bit version: libsndfile-1.0.xx-w64-setup.exe.

### macOS

 * Install Xcode: Open the App Store app. Search for "xcode". Install the first result. Wait for this to finish before moving on.
 * Install Homebrew: Open the Terminal app. Aim your browser at https://brew.sh. Copy and paste the text of the install command into the Terminal app.
 * Use the brew command to install some software. In the Terminal app, run this command:

       brew install git git-lfs cmake libsndfile

### Ubuntu/Debian

    sudo apt install build-essential git git-lfs cmake libsndfile1-dev libassimp-dev libasound-dev libxrandr-dev libopengl-dev libxinerama-dev libxcursor-dev libxi-dev
    
allolib requires gcc >= 7. If you need to build on older versions, you can use a ppa to get gcc:

    sudo -E apt-add-repository -y "ppa:ubuntu-toolchain-r/test"
    sudo apt update
    sudo -E apt-get install gcc-8 g++-8

## Building library and running examples
On a bash shell on Windows, Linux and OS X do:

    git clone https://github.com/AlloSphere-Research-Group/allolib
    cd allolib
    git submodule update --recursive --init
    mkdir build
    cd build
    cmake .. -DALLOLIB_BUILD_EXAMPLES=1
    cmake --build .

The library will be built in build/lib.

# Building aplications with allolib

There are two options provided to build allolib applications. The first is [allolib_playground](https://github.com/AlloSphere-Research-Group/allolib_playground) that is great for prototyping single file applications and for exploring the examples. For more complex projects with multiple source files and dependencies, use [allotemplate](https://github.com/AlloSphere-Research-Group/allotemplate)

# Extensions

Allolib provides an extension mechanism for libraries that have large or
platform dependent dependencies. The stable set of extensions can be found in
the [al_ext repo](https://github.com/AlloSphere-Research-Group/al_ext).
The allotemplate repo linked above shows how to integrate them. al_ext
is a separate repo that depends on the allolib library and provides cmake
facilities for easy integration with it.
