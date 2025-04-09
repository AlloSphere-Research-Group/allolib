# add_subdirectory(external/cpptoml)

# find_package(cpptoml REQUIRED)

set(headers

  # include/al/io/al_Arduino.hpp
  # include/al/io/al_AudioIO.hpp
  # include/al/io/al_AudioIOData.hpp
  # include/al/io/al_ControlNav.hpp
  include/al/io/al_CSVReader.hpp
  include/al/io/al_File.hpp
  include/al/io/al_FileWatcher.hpp

  # include/al/io/al_Imgui.hpp
  # include/al/io/al_MIDI.hpp
  # include/al/io/al_PersistentConfig.hpp
  include/al/io/al_Socket.hpp

  # include/al/io/al_Toml.hpp
  # include/al/io/al_Window.hpp
)

set(sources

  # src/io/al_Arduino.cpp
  # src/io/al_AudioIO.cpp
  # src/io/al_AudioIOData.cpp
  # src/io/al_ControlNav.cpp
  src/io/al_CSVReader.cpp
  src/io/al_File.cpp
  src/io/al_FileWatcher.cpp

  # src/io/al_Imgui.cpp
  # src/io/al_MIDI.cpp
  # src/io/al_PersistentConfig.cpp
  src/io/al_Socket.cpp

  # src/io/al_Toml.cpp
  # src/io/al_Window.cpp

  # src/io/al_WindowGLFW.cpp
  # src/io/al_imgui_impl.cpp
)

add_library(alio ${AL_LIBRARY_TYPE} ${headers} ${sources})

set_target_properties(alio PROPERTIES
  DEBUG_POSTFIX d
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
)

target_include_directories(alio PUBLIC
  include
)

target_link_libraries(alio PUBLIC
  alsystem
  altypes
)