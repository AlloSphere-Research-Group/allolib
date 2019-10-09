#ifndef INCLUDE_AL_SPHERE_UTILS_HPP
#define INCLUDE_AL_SPHERE_UTILS_HPP

#include <cassert>
#include <cmath>
#include <string>
#include <vector>

// gethostname
#ifdef AL_WINDOWS
#include <Winsock2.h>
#else
#include <unistd.h>
#endif

//#include "al/graphics/al_GLFW.hpp"
#include "al/math/al_Constants.hpp"

std::string al_get_hostname();

namespace al {

namespace sphere {

bool is_simulator(std::string const& host);

bool is_simulator();

bool is_renderer(std::string const& host);

bool is_renderer();

std::string renderer_hostname(std::string const& fallback);

bool is_in_sphere();

void get_fullscreen_dimension(int* width, int* height);

std::string config_directory(std::string const& dir_if_not_renderer);

std::string renderer_config_file_path(std::string const& host);

std::string renderer_config_file_path();

std::string config_file_path(std::string const& path_if_not_renderer);

std::vector<float> generate_equirect_sampletex(int width, int height);

}  // namespace sphere

}  // namespace al

#endif
