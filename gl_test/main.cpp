#include "al/core.hpp"

#include "GL/glew.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
  if (!glfwInit()) {
    cout << "ERROR: could not start GLFW" << endl;
    return 1;
  }
  cout << "Initialized GLFW " << glfwGetVersionString() << endl;
  glfwDefaultWindowHints();
  auto win = glfwCreateWindow(640, 480, "al gl test", NULL, NULL);
  glfwMakeContextCurrent(win);
  auto err = glewInit();
  if (GLEW_OK != err) {
    cout << "Error: " << glewGetErrorString(err) << endl;
    return 1;
  }
  cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << endl;
  return 0;
}