TODO

- stereo window
- fullscreen with specific monitor
- connect omnirender
- remove apr dependency (using oscpack's minial socket?)
- stl thread
- stl random

DEPENDENCIES

- GLFW3
- GLEW
- APR

NOTE

For visual studio, cmake with version 3.8 (latest) is recommended. It has a feature for setting the working directory within cmake script. Otherwise, user will have to set it manually at

`project -> properties -> debugging -> working directory`

setting it to $(OutDir) will do it correctly.