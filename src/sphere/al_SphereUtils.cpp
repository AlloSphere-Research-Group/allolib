

#include "al/sphere/al_SphereUtils.hpp"

std::string al_get_hostname()
{
  char hostname[256];
#ifdef AL_WINDOWS
  gethostname(hostname, 256);
#else
  gethostname(hostname, 256);
#endif
  return std::string {hostname};
}

using namespace al;

bool sphere::is_simulator(const std::string &host)
{
  return (host.substr(0, 5) == "audio");
}

bool sphere::is_simulator()
{
  return is_simulator(al_get_hostname());
}

bool sphere::is_renderer(const std::string &host)
{
  return (host.substr(0, 2) == "gr");
}

bool sphere::is_renderer()
{
  return is_renderer(al_get_hostname());
}

std::string sphere::renderer_hostname(const std::string &fallback) {
  auto const host = al_get_hostname();
  if (is_renderer(host)) return host;
  else return fallback;
}

bool sphere::is_in_sphere()
{
  auto const host = al_get_hostname();
  return is_simulator(host) || is_renderer(host);
}

void sphere::get_fullscreen_dimension(int *width, int *height)
{
  // Window Size in AlloSphere

  // considering Mosaic'ed displays as one bing screen
  // (since glfw can't see Mosaic settings)

  // original code by Donghao Ren
  // https://github.com/donghaoren
  // donghaoren@cs.ucsb.edu

  int count;

  *width = 0;
  *height = 0;

  assert(0 == 1);

  //FIXME implement
  //    GLFWmonitor** monitors = glfwGetMonitors(&count);

  //    for (int i = 0; i < count; i += 1) {
  //        int x, y;
  //        glfwGetMonitorPos(monitors[i], &x, &y);
  //        const GLFWvidmode* vm = glfwGetVideoMode(monitors[i]);
  //        int xmax = x + vm->width;
  //        int ymax = y + vm->height;
  //        if (*width < xmax) *width = xmax;
  //        if (*height < ymax) *height = ymax;
  //    }
}

std::string sphere::config_directory(const std::string &dir_if_not_renderer) {
  if (is_renderer()) return "/home/sphere/calibration-current/";
  else return dir_if_not_renderer;
}

std::string sphere::renderer_config_file_path(const std::string &host)
{
  return "/home/sphere/calibration-current/" + host + ".txt";
}

std::string sphere::renderer_config_file_path()
{
  return renderer_config_file_path(al_get_hostname());
}

std::string sphere::config_file_path(const std::string &path_if_not_renderer)
{
  auto const host = al_get_hostname();
  if (is_renderer(host)) return renderer_config_file_path(host);
  else return path_if_not_renderer;
}

std::vector<float> sphere::generate_equirect_sampletex(int width, int height)
{
  std::vector<float> arr;
  arr.resize(width * height * 4);
  for (int i = 0; i < width; i++) {
    float longi = float(i / width * M_2PI);
    for (int j = 0; j < height; j++) {
      int idx = i + width * j;
      float latti = float((j /height - 0.5) * M_PI);
      arr[4 * idx + 0] = std::cos(longi) * std::cos(latti);
      arr[4 * idx + 1] = std::sin(latti);
      arr[4 * idx + 2] = std::sin(longi) * std::cos(latti);
      arr[4 * idx + 3] = 0.0f;
    }
  }
  return arr;
}
