#ifndef INCLUDE_AL_IMGUI_HPP
#define INCLUDE_AL_IMGUI_HPP

#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"
#include "al/core/graphics/al_GLFW.hpp"

namespace al {

inline void initIMGUI()
{
  ImGui::CreateContext();
  ImGui_ImplGlfwGL3_Init(glfwGetCurrentContext(), false);
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