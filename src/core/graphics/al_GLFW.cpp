#include "al/core/graphics/al_GLFW.hpp"

#include <iostream>
#include <cstdlib> // exit, EXIT_FAILURE


void cbError(int code, const char* description) {
  std::cout << "glfw error [" << code << "]: " << description << std::endl;
}

namespace al {

void glfw::init() {
  static bool inited = false;
  if (inited) {
    //std::cout << "GLFW already initialized" << std::endl;
    return;
  }

  if (!glfwInit()) {
    std::cout << "ERROR: could not start GLFW" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::cout << "Initialized GLFW " << glfwGetVersionString() << std::endl;
  glfwSetErrorCallback(cbError);

  inited = true;
}

void glfw::terminate() {
  std::cout << "Terminating GLFW ... ";
  glfwTerminate();
  std::cout << "Done." << std::endl;
}

}