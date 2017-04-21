if(MACOS OR LINUX)
# on MacOS
# pkg-config gives wrong path for glfw from homebrew. let's manually add dir
# on LINUX
# make install from source-compiled glfw puts lib in /usr/local/lib
    link_directories(/usr/local/lib)
endif (MACOS)