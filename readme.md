TODO

- fullscreen with specific monitor
- stereo & omni
- font loading
- Parameter class

DEPENDENCIES

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