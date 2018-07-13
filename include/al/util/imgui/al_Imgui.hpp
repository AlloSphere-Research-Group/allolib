#ifndef INCLUDE_AL_IMGUI_HPP
#define INCLUDE_AL_IMGUI_HPP

#include "imgui.h"
#include "al/util/imgui/imgui_impl_glfw_gl3.h"

namespace al {

void initIMGUI();
void beginIMGUI();
void endIMGUI();
void shutdownIMGUI();

// beginIMGUI_minimal: calls beginIMGUI and start a window with miminal setup:
//                     transparent background, no titlebar, no moving, no resize
//                     autofit, and window is place at top-left. passing bool
//                     `use_input` will activate/deactivate receiving input
// endIMGUI_minimal  : ends the window started at beginIMGUI_minimal and calls
//                     endIMGUI, passing bool `show` will do/not do rendering of
//                     the every imgui widgets
// also see example/imgui/minimal.cpp for usage
void beginIMGUI_minimal(bool use_input, const char *name = "", int posX = 0, int posY = 0, float alpha = 0);
void endIMGUI_minimal(bool show);

// returns true if keyboard/mouse input is getting used by imgui
// can be used to disable other interactions if needed
bool imgui_is_using_input();

} // namespace al

#endif
