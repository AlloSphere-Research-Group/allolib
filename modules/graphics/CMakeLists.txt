find_package(OpenGL REQUIRED)

add_subdirectory(external/glad)
add_subdirectory(external/stb)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries (general)" FORCE)
set(BUILD_TESTING     OFF CACHE BOOL "Build test targets (general)" FORCE)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "Build the GLFW test programs" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "Build the GLFW documentation" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "Generate installation target" FORCE)
set(GLFW_VULKAN_STATIC  OFF CACHE BOOL "Assume Vulkan loader linked with app" FORCE)
set(GLFW_USE_OSMESA     OFF CACHE BOOL "OSMesa for offscreen context" FORCE)
set(GLFW_USE_HYBRID_HPG OFF CACHE BOOL "Force high-performance GPU" FORCE)
set(GLFW_USE_WAYLAND    OFF CACHE BOOL "Use Wayland for window creation" FORCE)
set(USE_MSVC_RUNTIME_LIBRARY_DLL ON CACHE BOOL "MSVC runtime library DLL" FORCE)

add_subdirectory(external/glfw)

set(headers
  include/al/graphics/al_BufferObject.hpp
  include/al/graphics/al_DefaultShaders.hpp
  include/al/graphics/al_DefaultShaderString.hpp
  include/al/graphics/al_EasyFBO.hpp
  include/al/graphics/al_EasyVAO.hpp
  include/al/graphics/al_FBO.hpp
  include/al/graphics/al_Font.hpp
  include/al/graphics/al_GPUObject.hpp
  include/al/graphics/al_Graphics.hpp
  include/al/graphics/al_Image.hpp
  include/al/graphics/al_Isosurface.hpp
  include/al/graphics/al_Lens.hpp
  include/al/graphics/al_Light.hpp
  include/al/graphics/al_Mesh.hpp
  include/al/graphics/al_OpenGL.hpp
  include/al/graphics/al_RenderManager.hpp
  include/al/graphics/al_Shader.hpp
  include/al/graphics/al_Shapes.hpp
  include/al/graphics/al_Texture.hpp
  include/al/graphics/al_VAO.hpp
  include/al/graphics/al_VAOMesh.hpp
  include/al/graphics/al_Viewpoint.hpp

  include/al/io/al_Window.hpp
  include/al/io/al_ControlNav.hpp
)

set(sources
  src/graphics/al_BufferObject.cpp
  src/graphics/al_DefaultShaders.cpp
  src/graphics/al_DefaultShaderString.cpp
  src/graphics/al_EasyFBO.cpp
  src/graphics/al_EasyVAO.cpp
  src/graphics/al_FBO.cpp
  src/graphics/al_Font.cpp
  src/graphics/al_GPUObject.cpp
  src/graphics/al_Graphics.cpp
  src/graphics/al_Image.cpp
  src/graphics/al_Isosurface.cpp
  src/graphics/al_Lens.cpp
  src/graphics/al_Light.cpp
  src/graphics/al_Mesh.cpp
  src/graphics/al_OpenGL.cpp
  src/graphics/al_RenderManager.cpp
  src/graphics/al_Shader.cpp
  src/graphics/al_Shapes.cpp
  src/graphics/al_Texture.cpp
  src/graphics/al_VAO.cpp
  src/graphics/al_VAOMesh.cpp
  src/graphics/al_Viewpoint.cpp

  src/graphics/al_stb_image.cpp
  src/graphics/al_stb_font.cpp

  src/io/al_Window.cpp
  src/io/al_WindowGLFW.cpp
  src/io/al_ControlNav.cpp
)

add_library(algraphics ${AL_LIBRARY_TYPE} ${headers} ${sources})

set_target_properties(algraphics PROPERTIES
  DEBUG_POSTFIX d
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
)

target_include_directories(algraphics PUBLIC
  include
)

target_link_libraries(algraphics PUBLIC
  almath
  altypes
  alspatial
  alsystem
  ${OPENGL_gl_LIBRARY}
  glad
  stb
  glfw
)