add_library(alsystem ${AL_LIBRARY_TYPE})
add_library(al::system ALIAS alsystem)

target_sources(alsystem
  PRIVATE
    src/system/al_PeriodicThread.cpp
    src/system/al_Printing.cpp
    src/system/al_ThreadNative.cpp
    src/system/al_Time.cpp
  PUBLIC
    FILE_SET HEADERS
      BASE_DIRS include
      FILES
        include/al/system/al_NodeConfiguration.hpp
        include/al/system/al_PeriodicThread.hpp
        include/al/system/al_Printing.hpp
        include/al/system/al_Thread.hpp
        include/al/system/al_Time.hpp
)

al_configure_module(alsystem EXPORT_NAME system)

if(AL_WINDOWS)
  target_compile_definitions(alsystem PUBLIC NOMINMAX)
endif()

target_compile_definitions(alsystem PUBLIC ${PLATFORM_DEFINITION})
target_link_libraries(alsystem PUBLIC al::math)

al_install_module(alsystem)
