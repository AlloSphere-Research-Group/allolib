add_subdirectory(external/cpptoml)

add_library(alconfig ${AL_LIBRARY_TYPE})
add_library(al::config ALIAS alconfig)

target_sources(alconfig
  PRIVATE
    src/io/al_PersistentConfig.cpp
    src/io/al_Toml.cpp
  PUBLIC
    FILE_SET HEADERS
      BASE_DIRS include
      FILES
        include/al/io/al_PersistentConfig.hpp
        include/al/io/al_Toml.hpp
)

al_configure_module(alconfig EXPORT_NAME config)

if(AL_WINDOWS)
  target_compile_definitions(alconfig PUBLIC NOMINMAX)
endif()

target_link_libraries(alconfig PUBLIC
  al::io
  al::ui
  cpptoml
)

al_install_module(alconfig)
