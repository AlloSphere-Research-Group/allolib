#include "al/io/al_Imgui.hpp"
#include "al_imgui_impl.hpp"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

void al::imguiInit() {
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  // ImGuiIO& io = ImGui::GetIO(); (void)io;
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable
  // Keyboard Controls io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; //
  // Enable Gamepad Controls

  ImGui::StyleColorsDark();
  // ImGui::StyleColorsClassic();

  GLFWwindow* window = glfwGetCurrentContext();
  ImGui_ImplGlfw_InitForOpenGL(window, true);  // true: install callbacks
  // imgui installed callbacks will also call previously registered callbacks

  const char* glsl_version = "#version 330";
  ImGui_ImplOpenGL3_Init(glsl_version);
}

void al::imguiBeginFrame() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void al::imguiEndFrame() { ImGui::Render(); }

void al::imguiDraw() { ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); }

void al::imguiShutdown() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

bool al::isImguiUsingInput() {
  auto& io = ImGui::GetIO();
  return io.WantCaptureMouse | io.WantCaptureKeyboard | io.WantTextInput;
}

bool al::isImguiUsingKeyboard() {
  auto& io = ImGui::GetIO();
  return io.WantCaptureKeyboard | io.WantTextInput;
}
