#include "al/graphics/al_GLEW.hpp"

#include <iostream>
#include <cstdlib> // exit, EXIT_FAILURE

using namespace al;

bool glew_loaded = false;

void glew::init(bool is_verbose) {
  if (glew_loaded) {
    if (is_verbose) std::cout << "GLEW already initialized" << std::endl;
    return;
  }
  glewExperimental = GL_TRUE; // MUST if glew 1.13 or lower (ubuntu 16.04)
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    std::cout << "Glew Error: " << glewGetErrorString(err) << std::endl;
    exit(EXIT_FAILURE); // FIXME? is this recommended way of terminating?
  }
  if (is_verbose) {
    std::cout << "Initialized GLEW " << glewGetString(GLEW_VERSION) << std::endl;
  }
  glew_loaded = true;
}


bool gl::loaded() {
  return glew_loaded;
}