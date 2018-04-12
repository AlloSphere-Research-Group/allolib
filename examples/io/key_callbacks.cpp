#include "al/core.hpp"

#include <iostream>
#include <string>

using namespace al;
using std::cout;
using std::endl;

// make a type with interface void key_callback(GLFWwindow*, int, int, int, int)
// and the address of object of this type can be added to keycallback list
// the callback will be called 
struct type_with_keycallback_t
{
  std::string name = "noname";
  void key_callback(GLFWwindow* window, int key, int scancode, int action,
                    int mods) {
    cout << "callback on " << name << " with " << key << ", " << scancode
         << ", " << action << ", " << mods << "!" << endl;
  }
};

struct my_app : App
{

  type_with_keycallback_t callback_object0;
  type_with_keycallback_t callback_object1 {"obj1"};
  type_with_keycallback_t callback_object2 {"obj2"};

  void onCreate() override {
    auto& handler_list = glfw::get_keycallback_handler_list();
    handler_list.emplace_back(&callback_object0);
    handler_list.emplace_back(&callback_object1);
    handler_list.emplace_back(&callback_object2);
  }

  void onAnimate(double dt) override {
  }

  void onDraw(Graphics& g) override {
    g.clear(0);
  }

};

int main()
{
  my_app app;
  app.start();
}