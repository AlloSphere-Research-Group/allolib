if(MACOS OR LINUX)
# on MacOS
# pkg-config gives wrong path for glfw from homebrew. let's manually add dir
# on LINUX
# make install from source-compiled glfw puts lib in /usr/local/lib
    link_directories(/usr/local/lib)
endif (MACOS OR LINUX)

set(APP_OUTPUT_PATH ${app_path}/bin)
if (WINDOWS)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${APP_OUTPUT_PATH})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${APP_OUTPUT_PATH})
else ()
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${APP_OUTPUT_PATH})
endif (WINDOWS)