
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
    external/imgui
    external/imgui/examples
)

set_target_properties(imgui PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    DEBUG_POSTFIX d
)



### alimgui ###
set(headers
  include/al/io/al_Imgui.hpp
)

set(sources
  src/io/al_Imgui.cpp
  src/io/al_imgui_impl.cpp
)

add_library(alimgui ${AL_LIBRARY_TYPE} ${headers} ${sources})

set_target_properties(alimgui PROPERTIES
  DEBUG_POSTFIX d
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
)

target_include_directories(alimgui PUBLIC
  include
)

target_link_libraries(alimgui PUBLIC
  imgui
  algraphics
)