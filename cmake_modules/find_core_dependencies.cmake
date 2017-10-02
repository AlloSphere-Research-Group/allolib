# sets:
#   CORE_INCLUDE_DIRS
#   CORE_LIBRARIES
#   CORE_LIBRARY_DIRS

# al_path needs to be set prior to calling this script

find_package(OpenGL REQUIRED) # >> ${OPENGL_LIBRARY}, inlcude dirs are handled with glew

if (AL_WINDOWS)

  # set(GLFW_INCLUDE_DIRS ${al_path}/dependencies/glfw/include)
  # set(GLFW_LIBRARIES ${al_path}/dependencies/glfw/lib-vc2015/glfw3dll.lib)
  # set(GLEW_INCLUDE_DIRS ${al_path}/dependencies/glew/include)
  # set(GLEW_LIBRARIES ${al_path}/dependencies/glew/lib/Release/x64/glew32.lib)

    add_library(GLEW::GLEW SHARED IMPORTED)
    set_target_properties(GLEW::GLEW PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES
            ${al_path}/dependencies/glew/include
        IMPORTED_IMPLIB
            ${al_path}/dependencies/glew/lib/Release/x64/glew32.lib
        IMPORTED_LOCATION
            ${al_path}/dependencies/glew/bin/Release/x64/glew32.dll
    )

    add_library(PkgConfig::GLFW SHARED IMPORTED)
    set_target_properties(PkgConfig::GLFW PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES
            ${al_path}/dependencies/glfw/include
        IMPORTED_IMPLIB
            ${al_path}/dependencies/glfw/lib-vc2015/glfw3dll.lib
        IMPORTED_LOCATION
            ${al_path}/dependencies/glfw/lib-vc2015/glfw3.dll
    )

else ()

  find_package(GLEW REQUIRED)
  find_package(PkgConfig REQUIRED)
  pkg_search_module(GLFW REQUIRED IMPORTED_TARGET glfw3)

endif (AL_WINDOWS)

message("GLFW_INCLUDE_DIRS: ${GLFW_INCLUDE_DIRS}")
message("GLFW_LIBRARIES: ${GLFW_LIBRARIES}")
message("GLFW_LIBRARY_DIRS: ${GLFW_LIBRARY_DIRS}")


get_target_property(AL_GLFW_INCLUDE_DIR PkgConfig::GLFW INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(AL_GLFW_LINK_LIBS PkgConfig::GLFW INTERFACE_LINK_LIBRARIES)
message("pkgconfig::glfw include dirs: ${AL_GLFW_INCLUDE_DIR}")
message("pkgconfig::glfw link libs: ${AL_GLFW_LINK_LIBS}")

message("opengl lib: ${OPENGL_gl_LIBRARY}")
message("opengl lib: ${OPENGL_glu_LIBRARY}")

# set(CORE_INCLUDE_DIRS
  # ${GLFW_INCLUDE_DIRS}
  # ${GLEW_INCLUDE_DIRS}
# )

# set(CORE_LIBRARIES
  # ${OPENGL_LIBRARY}
  # ${GLEW_LIBRARIES}
  # ${GLFW_LIBRARIES}
# )

# set(CORE_LIBRARY_DIRS
  # ${GLFW_LIBRARY_DIRS}
# )

# add_library(GLEW::GLEW SHARED IMPORTED)
# set_target_properties(GLEW::GLEW PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${GLEW_INCLUDE_DIRS})
# set_target_properties(GLEW::GLEW PROPERTIES IMPORTED_IMPLIB ${}) # .lib
# set_target_properties(GLEW::GLEW PROPERTIES IMPORTED_LOCATION ${GLEW_LIBRARIES}) # .dll


# set_target_properties(GLEW::GLEW PROPERTIES
#   INTERFACE_INCLUDE_DIRECTORIES
#     ${GLEW_INCLUDE_DIRS}
#   IMPORTED_LOCATION
#     ${GLEW_LIBRARIES}
# )
