#ifndef INCLUDE_AL_IMGUI_HPP
#define INCLUDE_AL_IMGUI_HPP

#include "imgui.h"
#include "al/util/imgui/imgui_impl_glfw_gl3.h"
#include "al/core/graphics/al_GLFW.hpp"

namespace al {

void initIMGUI();
void beginIMGUI();
void endIMGUI();
void shutdownIMGUI();

} // namespace al

#endif