#ifndef INCLUDE_AL_IMGUI_HPP
#define INCLUDE_AL_IMGUI_HPP

#include "imgui.h"

namespace al {

void imguiInit();
void imguiBeginFrame();
void imguiEndFrame();
void imguiDraw();
void imguiShutdown();
bool isImguiUsingInput();
bool isImguiUsingKeyboard();

}  // namespace al

#endif
