### Imgui ###
if(ALLOLIB_BUILD_SHARED)
  set(ALLOLIB_EXTERNAL_LIB_TYPE "SHARED")
else()
  set(ALLOLIB_EXTERNAL_LIB_TYPE "STATIC")
endif()

add_library(imgui ${ALLOLIB_EXTERNAL_LIB_TYPE}
  external/imgui/imgui.cpp
  external/imgui/imgui_demo.cpp
  external/imgui/imgui_draw.cpp
  external/imgui/imgui_widgets.cpp
)

target_include_directories(imgui PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/imgui>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/examples>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties(imgui PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  DEBUG_POSTFIX d
)

### alimgui ###

add_library(alimgui ${AL_LIBRARY_TYPE})
add_library(al::imgui ALIAS alimgui)

target_sources(alimgui
  PRIVATE
    src/io/al_Imgui.cpp
    src/io/al_imgui_impl.cpp
  PUBLIC
    FILE_SET HEADERS
      BASE_DIRS include
      FILES
        include/al/io/al_Imgui.hpp
)

al_configure_module(alimgui EXPORT_NAME imgui)

target_link_libraries(alimgui PUBLIC
  imgui
  al::graphics
)

al_install_module(alimgui)
