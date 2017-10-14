#include "al/core/graphics/al_GLEW.hpp"

#include <iostream>
#include <cstdlib> // exit, EXIT_FAILURE

using namespace al;

bool glew_loaded = false;

void glew::init() {
  if (glew_loaded) {
    std::cout << "GLEW already initialized" << std::endl;
    return;
  }
#if GLEW_VERSION < 2
  glewExperimental=GL_TRUE; // MUST if glew version is 1.13 or lower (apt!!!!)
#endif
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    std::cout << "Glew Error: " << glewGetErrorString(err) << std::endl;
    exit(EXIT_FAILURE); // FIXME? is this recommended way of terminating?
  }
  std::cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
  glew_loaded = true;
}


bool gl::loaded() {
  return glew_loaded;
}