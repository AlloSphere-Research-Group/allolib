#ifndef INCLUDE_AL_GLFW_HPP
#define INCLUDE_AL_GLFW_HPP

/*  Keehong Youn, 2017, younkeehong@gmail.com
*/

#include "al/core/graphics/al_GLEW.hpp"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <vector>
#include <memory>

namespace al {

namespace glfw
{

void init(bool is_verbose=false);
void terminate(bool is_verbose=false);
inline GLFWwindow* current_window() { return glfwGetCurrentContext(); }

// type for wrapping and storing pointer to objects
// that has functions with glfw callback signature
// no template for this type itself so one can store these in one std::vector
struct KeycallbackHandler
{
  // base pointer for interface
  struct handler_concept_t {
    virtual ~handler_concept_t() = default;
    virtual void key_callback(GLFWwindow*, int, int, int, int) = 0;
  };

  // derived type that wrap and stores address to object
  template<typename T>
  struct handler_model_t final : handler_concept_t {
    T* object_ptr;
    handler_model_t(T* t): object_ptr(t) {}
    void key_callback(GLFWwindow* window, int key, int scancode, int action,
                      int mods) override {
      object_ptr->key_callback(window, key, scancode, action, mods);
    }
  };

  // no templated member so all wrapped handlers look same from outside
  // keep data as pointer for polymorphism,
  // use smart pointer to be smart
  std::unique_ptr<handler_concept_t> handler_ptr;

  // make a unique pointer of derived type and store as a pointer to base type
  template<typename T>
  KeycallbackHandler(T* t)
    : handler_ptr(std::make_unique<handler_model_t<T>>(t)) {}

  // call the callback of object that is stored as wrapped
  void key_callback(GLFWwindow* window, int key, int scancode, int action,
                    int mods) {
    handler_ptr->key_callback(window, key, scancode, action, mods);
  }

};

using KeycallbackHandlerList = std::vector<KeycallbackHandler>;

// get singleton instance of handler list
KeycallbackHandlerList& get_keycallback_handler_list();

}

}

#endif