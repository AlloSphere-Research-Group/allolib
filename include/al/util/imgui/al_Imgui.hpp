#ifndef INCLUDE_AL_IMGUI_HPP
#define INCLUDE_AL_IMGUI_HPP

#include "imgui.h"
#include "al/util/imgui/imgui_impl_glfw_gl3.h"
#include "al/core/graphics/al_GLFW.hpp"

namespace al {

inline void initIMGUI()
{
  ImGui::CreateContext();
  auto* window = glfwGetCurrentContext();
  // false: don't register glfw event callbacks, allolib will handle them
  ImGui_ImplGlfwGL3_Init(window, false);
  // scroll and char callback are not handled by allolib
  glfwSetScrollCallback(window, ImGui_ImplGlfw_ScrollCallback);
  glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);
}

inline void beginIMGUI()
{
  ImGui_ImplGlfwGL3_NewFrame();
}

inline void endIMGUI()
{
  ImGui::Render();
  ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
}

inline void shutdownIMGUI()
{
  ImGui_ImplGlfwGL3_Shutdown();
  ImGui::DestroyContext();
}

} // namespace al

#endif