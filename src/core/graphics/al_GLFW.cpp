#include "al/core/graphics/al_GLFW.hpp"

#include <iostream>
#include <cstdlib> // exit, EXIT_FAILURE


void cbError(int code, const char* description) {
  std::cout << "glfw error [" << code << "]: " << description << std::endl;
}

namespace al {

void glfw::init(bool is_verbose) {
  static bool inited = false;
  if (inited) {
    //std::cout << "GLFW already initialized" << std::endl;
    return;
  }

  if (!glfwInit()) {
    std::cout << "ERROR: could not start GLFW" << std::endl;
    exit(EXIT_FAILURE);
  }
  if (is_verbose) std::cout << "Initialized GLFW " << glfwGetVersionString() << std::endl;
  glfwSetErrorCallback(cbError);

  inited = true;
}

void glfw::terminate(bool is_verbose) {
  if (is_verbose) std::cout << "Terminating GLFW ... ";
  glfwTerminate();
  if (is_verbose) std::cout << "Done." << std::endl;
}

glfw::KeycallbackHandlerList& glfw::get_keycallback_handler_list() {
  // it is fine to have list as stack var since the list does not own any memory
  static KeycallbackHandlerList list;
  return list;
}

}