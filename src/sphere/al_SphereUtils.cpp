
#define GLFW_INCLUDE_NONE
#include "al/sphere/al_SphereUtils.hpp"

// gethostname
#ifdef AL_WINDOWS
#include <Winsock2.h>
#else
#include <unistd.h>
#endif

#include <GLFW/glfw3.h>

std::string al_get_hostname() {
  char hostname[256];
#ifdef AL_WINDOWS
  gethostname(hostname, 256);
#else
  gethostname(hostname, 256);
#endif
  return std::string{hostname};
}

using namespace al;

bool sphere::isSimulatorMachine(const std::string &host) {
  return (host.substr(0, 5) == "audio");
}

bool sphere::isSimulatorMachine() {
  return isSimulatorMachine(al_get_hostname());
}

bool sphere::isRendererMachine(const std::string &host) {
  return (host.substr(0, 2) == "gr");
}

bool sphere::isRendererMachine() {
  return isRendererMachine(al_get_hostname());
}

std::string sphere::renderer_hostname(const std::string &fallback) {
  auto const host = al_get_hostname();
  if (isRendererMachine(host))
    return host;
  else
    return fallback;
}

bool sphere::isSphereMachine() {
  auto const host = al_get_hostname();
  return isSimulatorMachine(host) || isRendererMachine(host);
}

void sphere::getFullscreenDimension(int *width, int *height) {
  // Window Size in AlloSphere

  // considering Mosaic'ed displays as one bing screen
  // (since glfw can't see Mosaic settings)

  // original code by Donghao Ren
  // https://github.com/donghaoren
  // donghaoren@cs.ucsb.edu

  int count;

  *width = 0;
  *height = 0;

  GLFWmonitor **monitors = glfwGetMonitors(&count);

  for (int i = 0; i < count; i += 1) {
    int x, y;
    glfwGetMonitorPos(monitors[i], &x, &y);
    const GLFWvidmode *vm = glfwGetVideoMode(monitors[i]);
    int xmax = x + vm->width;
    int ymax = y + vm->height;
    if (*width < xmax) *width = xmax;
    if (*height < ymax) *height = ymax;
  }
}

std::string sphere::getCalibrationDirectory(
    const std::string &dir_if_not_renderer) {
  if (isRendererMachine())
    return "/home/sphere/calibration-current/";
  else
    return dir_if_not_renderer;
}

std::string sphere::getRendererCalibrationFilepath(const std::string &host) {
  return "/home/sphere/calibration-current/" + host + ".txt";
}

std::string sphere::getRendererCalibrationFilepath() {
  return getRendererCalibrationFilepath(al_get_hostname());
}

std::string sphere::getCalibrationFilepath(
    const std::string &path_if_not_renderer) {
  auto const host = al_get_hostname();
  if (isRendererMachine(host))
    return getRendererCalibrationFilepath(host);
  else
    return path_if_not_renderer;
}

std::vector<float> sphere::generateEquirectSampletex(int width, int height) {
  std::vector<float> arr;
  arr.resize(width * height * 4);
  for (int i = 0; i < width; i++) {
    float longi = float(i / width * M_2PI);
    for (int j = 0; j < height; j++) {
      int idx = i + width * j;
      float latti = float((j / height - 0.5) * M_PI);
      arr[4 * idx + 0] = std::cos(longi) * std::cos(latti);
      arr[4 * idx + 1] = std::sin(latti);
      arr[4 * idx + 2] = std::sin(longi) * std::cos(latti);
      arr[4 * idx + 3] = 0.0f;
    }
  }
  return arr;
}

std::map<std::string, NodeConfiguration> sphere::getSphereNodes() {
  std::map<std::string, NodeConfiguration> nodes = {
      {"ar01.1g",
       NodeConfiguration{
           0, 0, "/Volumes/Data",
           (Capability)(Capability::CAP_SIMULATOR | Capability::CAP_RENDERING |
                        Capability::CAP_AUDIO_IO | Capability::CAP_OSC)}},
      {"atari.1g",
       NodeConfiguration{
           0, 0, "e:/",
           (Capability)(Capability::CAP_SIMULATOR | Capability::CAP_RENDERING |
                        Capability::CAP_AUDIO_IO | Capability::CAP_OSC)}},

  };

  char str[3];
  for (uint16_t i = 1; i <= 14; i++) {
    snprintf(str, 3, "%02d", i);
    std::string name = "gr" + std::string(str);
    nodes[name] = NodeConfiguration{
        i, 0, "/alloshare",
        (Capability)(CAP_SIMULATOR | CAP_OMNIRENDERING | CAP_OSC)};
  }
  return nodes;
}
