# sets:
#   CORE_INCLUDE_DIRS
#   CORE_LIBRARIES

# al_path needs to be set prior to calling this script

find_package(OpenGL REQUIRED)

if (WINDOWS)

  set(GLFW_INCLUDE_DIRS ${al_path}/dependencies/glfw/include)
  set(GLFW_LIBRARIES ${al_path}/dependencies/glfw/glfw3dll.lib)
  set(GLEW_INCLUDE_DIRS ${al_path}/dependencies/glew/include)
  set(GLEW_LIBRARIES ${al_path}/dependencies/glew/glew32.lib)

else ()

  find_package(GLEW REQUIRED)
  find_package(PkgConfig REQUIRED)
  pkg_search_module(GLFW REQUIRED glfw3)

endif (WINDOWS)

set(CORE_INCLUDE_DIRS
  ${GLFW_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
)

set(CORE_LIBRARIES
  ${OPENGL_LIBRARY}
  ${GLEW_LIBRARIES}
  ${GLFW_LIBRARIES}
)