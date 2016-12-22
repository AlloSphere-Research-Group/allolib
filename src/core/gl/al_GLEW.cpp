#include "al/core/gl/al_GLEW.hpp"

#include <iostream>
#include <cstdlib> // exit, EXIT_FAILURE

using namespace al;

void glew::init() {
  static bool inited = false;
  if (inited) {
    std::cout << "GLEW already initialized" << std::endl;
    return;
  }
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    std::cout << "Error: " << glewGetErrorString(err) << std::endl;
    exit(EXIT_FAILURE);
  }
  std::cout << "Status: Using GLEW "
    << glewGetString(GLEW_VERSION) << std::endl;
  inited = true;
}