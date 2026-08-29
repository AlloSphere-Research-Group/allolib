## Graphics-space UI + imgui bindings. Scene widgets include al::scene types.

add_library(alui ${AL_LIBRARY_TYPE})
add_library(al::ui ALIAS alui)

target_sources(alui
  PRIVATE
    src/ui/al_BoundingBox.cpp
    src/ui/al_BoundingBoxData.cpp
    src/ui/al_ControlGUI.cpp
    src/ui/al_Dialog.cpp
    src/ui/al_FileSelector.cpp
    src/ui/al_Gnomon.cpp
    src/ui/al_ParameterGUI.cpp
    src/ui/al_Pickable.cpp
    src/ui/al_PickableManager.cpp
  PUBLIC
    FILE_SET HEADERS
      BASE_DIRS include
      FILES
        include/al/ui/al_BoundingBox.hpp
        include/al/ui/al_BoundingBoxData.hpp
        include/al/ui/al_ControlGUI.hpp
        include/al/ui/al_Dialog.hpp
        include/al/ui/al_FileSelector.hpp
        include/al/ui/al_Gnomon.hpp
        include/al/ui/al_ParameterGUI.hpp
        include/al/ui/al_Pickable.hpp
        include/al/ui/al_PickableManager.hpp
        include/al/ui/al_PickableRotateHandle.hpp
)

al_configure_module(alui EXPORT_NAME ui)

target_link_libraries(alui PUBLIC
  al::parameter
  al::scene
  al::math
  al::graphics
  al::imgui
)

al_install_module(alui)
