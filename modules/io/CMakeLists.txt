add_library(alio ${AL_LIBRARY_TYPE})
add_library(al::io ALIAS alio)

target_sources(alio
  PRIVATE
    src/io/al_CSVReader.cpp
    src/io/al_File.cpp
    src/io/al_FileWatcher.cpp
    src/io/al_Socket.cpp
  PUBLIC
    FILE_SET HEADERS
      BASE_DIRS include
      FILES
        include/al/io/al_CSVReader.hpp
        include/al/io/al_File.hpp
        include/al/io/al_FileWatcher.hpp
        include/al/io/al_Socket.hpp
)

al_configure_module(alio EXPORT_NAME io)

if(AL_WINDOWS)
  target_compile_definitions(alio PUBLIC NOMINMAX)
endif()

target_link_libraries(alio PUBLIC
  al::system
  al::types
)

al_install_module(alio)
