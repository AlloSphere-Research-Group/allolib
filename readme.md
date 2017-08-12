TODO

- stereo window
- fullscreen with specific monitor
- connect omnirender
- al::File

- use std::thread (?)
- use std::random
- img, font, mesh loading
- Parameter
- al::Socket
- alloAudio
- build n deploy

DEPENDENCIES

- GLFW
- GLEW

NOTE

For visual studio, cmake with version at least 3.8 is recommended. Version 3.8 has a feature for setting the working directory of Visual Studio project within cmake script. Otherwise, user will have to set it manually at

`project -> properties -> debugging -> working directory`

setting it to $(OutDir) will do it correctly (where compiler outputs executable)