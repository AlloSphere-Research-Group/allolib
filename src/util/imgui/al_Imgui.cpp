#include "al/util/imgui/al_Imgui.hpp"
#include "al/core/graphics/al_GLFW.hpp"

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

  // still keycallback needs to be called in some way for text input
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

void al::beginIMGUI_minimal(bool use_input, const char *name, int posX, int posY, float alpha)
{
  beginIMGUI();
  ImGui::SetNextWindowBgAlpha(alpha);

  ImGuiWindowFlags window_flags = 0;
  window_flags |= ImGuiWindowFlags_NoTitleBar; // if (no_titlebar)
  // window_flags |= ImGuiWindowFlags_NoScrollbar; // if (no_scrollbar)
  // window_flags |= ImGuiWindowFlags_MenuBar; // if (!no_menu)
  window_flags |= ImGuiWindowFlags_NoMove; // if (no_move)
  window_flags |= ImGuiWindowFlags_NoResize; // if (no_resize)
  window_flags |= ImGuiWindowFlags_NoCollapse; // if (no_collapse)
  window_flags |= ImGuiWindowFlags_NoNav; // if (no_nav)
  if (!use_input) window_flags |= ImGuiWindowFlags_NoInputs;

  ImGui::SetNextWindowSize(ImVec2(0, 0));
  ImGui::SetNextWindowPos(ImVec2(posX, posY));
  ImGui::Begin(name , nullptr, window_flags);
}

void al::endIMGUI_minimal(bool show)
{
  ImGui::End();
  if (show) endIMGUI();
}

bool al::imgui_is_using_input()
{
  auto& io = ImGui::GetIO();
  return io.WantCaptureMouse | io.WantCaptureKeyboard | io.WantTextInput;
}
