0. Using terminal?

   It is recommended to use `cmder` (http://cmder.net/) for git terminal commands. Download "Full" version that says "(With Git for Windows)". Cmder is a terminal emulator for windows coming with git and several unix commands included. However, none of the processes is dependent on `cmder`. So any other method (using git gui program such as Sourcetree) should also work.




1. Clone allolib repo

   ```shell
   $ git clone git@github.com:AlloSphere-Research-Group/allolib.git
   ```

   ​

2. Initialize submodules

   ```shell
   $ cd allolib
   $ git submodule init
   $ git submodule update
   ```

   ​

3. Download dependencies and place them in `dependencies` folder

   - GLEW http://glew.sourceforge.net/ "Binaries Windows 32-bit and 64-bit"
   - GLFW http://www.glfw.org/ "Download GLFW 3.X.X"
   - Name the folders `glew` and `glfw` when moving them to `dependencies` folder

   ```
   resulting folder structure will be
   allolib/dependencies/
                      |- glew/
                      |     |- bin/, doc/, include/, lib/
                      |     
                      |- glfw/
                            |- docs/, include/, ...
   ```

   ​

4. Run CMake to generate visual studio solution

   - CMake installer can be download from https://cmake.org/

   - To use CMake in terminal, option `Add CMake to the system PATH for all users` or `Add CMake to the system PATH for the current user` should be chosen when installing.  This option can be skipped if CMake will be used via GUI application. Here running CMake with terminal commands is explained.

   - Make a directory to place build files, go into the directory and run CMake with allolib folder. When generating solution, Visual Studio version and bit should be specified. If using Visual Studio 2015, the generator part will be `"Visual Studio 14 2015 Win64"`

     ```shell
     $ mkdir build
     $ cd build
     $ cmake .. -G "Visual Studio 15 2017 Win64"
     ```

     ​

5. Start Visual Studio and build allolib

   - Open allolib.sln in build folder with Visual Studio
   - Select build type (Debug? Release?).
   - Right click `ALL_BUILD` target under solution explorer and click `build`.
   - `al_debug.lib` or `al.lib` library file will be output at `allolib/lib` directory. Now allolib is ready to get linked to user project.