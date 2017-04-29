TODO

- stereo window
- fullscreen with specific monitor
- connect omnirender

BIG TODO

- remove apr dependency, implement socket, make osc work with new socket and stl thread
- replace portaudio with rtaudio

DEPENDENCIES

- GLFW3
- GLEW
- PORTAUDIO
- APR

NOTE

- for visual studio cmake with version 3.8 (latest) is recommended for setting the working directory via cmake script. otherwise, user will have to set it manually at `project -> properties -> debugging -> working directory`. setting it to $(OutDir) will do it correctly.