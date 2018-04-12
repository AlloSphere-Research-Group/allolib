#include "al/util/imgui/al_Imgui.hpp"

struct imgui_keycallback_t
{
  void key_callback(GLFWwindow* window, int key, int scancode, int action,
                    int mods) {
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
  }
};

imgui_keycallback_t* get_imgui_keycallback_singleton_object_ptr() {
  static imgui_keycallback_t obj;
  return &obj;
}

void al::initIMGUI()
{
  ImGui::CreateContext();
  auto* window = glfwGetCurrentContext();

  // false: don't register glfw event callbacks, allolib will handle them
  ImGui_ImplGlfwGL3_Init(window, false);

  // scroll and char callback are not handled by allolib
  glfwSetScrollCallback(window, ImGui_ImplGlfw_ScrollCallback);
  glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);

  // still keycallback needs to be called in some way
  // do it by registering to glfw raw keycallback handler list
  auto& handler_list = al::glfw::get_keycallback_handler_list();
  handler_list.emplace_back(get_imgui_keycallback_singleton_object_ptr());

}

void al::beginIMGUI()
{
  ImGui_ImplGlfwGL3_NewFrame();
}

void al::endIMGUI()
{
  ImGui::Render();
  ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
}

void al::shutdownIMGUI()
{
  ImGui_ImplGlfwGL3_Shutdown();
  ImGui::DestroyContext();
}