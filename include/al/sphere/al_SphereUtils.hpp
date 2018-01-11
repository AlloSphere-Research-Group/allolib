#ifndef INCLUDE_AL_SPHERE_UTILS_HPP
#define INCLUDE_AL_SPHERE_UTILS_HPP

#include "al/core/graphics/al_GLFW.hpp"
#include "al/core/math/al_Constants.hpp"

#include <string>
#include <vector>
#include <cmath>

// gethostname
#ifdef AL_WINDOWS
    #include <Winsock2.h>
#else
    #include <unistd.h>
#endif


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

namespace al {

namespace sphere {

bool is_simulator(std::string const& host)
{
    return (host.substr(0, 5) == "audio");
}

bool is_simulator()
{
    return is_simulator(al_get_hostname());
}

bool is_renderer(std::string const& host)
{
    return (host.substr(0, 2) == "gr");
}

bool is_renderer()
{
    return is_renderer(al_get_hostname());
}

std::string renderer_hostname(std::string const& fallback) {
    auto const host = al_get_hostname();
    if (is_renderer(host)) return host;
    else return fallback;
}

bool is_in_sphere()
{
    auto const host = al_get_hostname();
    return is_simulator(host) || is_renderer(host);
}

void get_fullscreen_dimension(int* width, int* height)
{
    // Window Size in AlloSphere
    
    // considering Mosaic'ed displays as one bing screen
    // (since glfw can't see Mosaic settings)

    // original code by Donghao Ren
    // https://github.com/donghaoren
    // donghaoren@cs.ucsb.edu

    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);

    *width = 0;
    *height = 0;

    for (int i = 0; i < count; i += 1) {
        int x, y;
        glfwGetMonitorPos(monitors[i], &x, &y);
        const GLFWvidmode* vm = glfwGetVideoMode(monitors[i]);
        int xmax = x + vm->width;
        int ymax = y + vm->height;
        if (*width < xmax) *width = xmax;
        if (*height < ymax) *height = ymax;
    }
}

std::string config_directory(std::string const& dir_if_not_renderer) {
    if (is_renderer()) return "/home/sphere/calibration-current/";
    else return dir_if_not_renderer;
}

std::string renderer_config_file_path(std::string const& host)
{
    return "/home/sphere/calibration-current/" + host + ".txt";
}

std::string renderer_config_file_path()
{
    return renderer_config_file_path(al_get_hostname());
}

std::string config_file_path(std::string const& path_if_not_renderer)
{
    auto const host = al_get_hostname();
    if (is_renderer(host)) return renderer_config_file_path(host);
    else return path_if_not_renderer;
}

std::vector<float> generate_equirect_sampletex(int width, int height)
{
    std::vector<float> arr;
    arr.resize(width * height * 4);
    for (int i = 0; i < width; i++) {
        float longi = i / float(width) * M_2PI;
        for (int j = 0; j < height; j++) {
            int idx = i + width * j;
            float latti = (j / float(height) - 0.5) * M_PI;
            arr[4 * idx + 0] = std::cos(longi) * std::cos(latti);
            arr[4 * idx + 1] = std::sin(latti);
            arr[4 * idx + 2] = std::sin(longi) * std::cos(latti);
            arr[4 * idx + 3] = 0.0f;
        }
    }
    return arr;
}

} // namespace sphere

} // namespace al

#endif