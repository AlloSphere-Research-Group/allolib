#define GLFW_INCLUDE_NONE
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#if defined(_MSC_VER)  // for imgui_impl_opengl3.cpp in windows
#pragma warning(push)
#pragma warning(disable : 4996)  // _CRT_SECURE_NO_WARNINGS
#endif

// these are in external/imgui/examples
#include "imgui_impl_glfw.cpp"
#include "imgui_impl_opengl3.cpp"

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
