#ifndef INCLUDE_AL_SPHERE_UTILS_HPP
#define INCLUDE_AL_SPHERE_UTILS_HPP

#include <cassert>
#include <cmath>
#include <map>
#include <string>
#include <vector>

#include "al/app/al_NodeConfiguration.hpp"
#include "al/math/al_Constants.hpp"

std::string al_get_hostname();

namespace al {

namespace sphere {

bool isSimulatorMachine(std::string const& host);

bool isSimulatorMachine();

bool isRendererMachine(std::string const& host);

bool isRendererMachine();

std::string renderer_hostname(std::string const& fallback);

bool isSphereMachine();

void getFullscreenDimension(int* width, int* height);

std::string getCalibrationDirectory(std::string const& dir_if_not_renderer);

std::string getRendererCalibrationFilepath(std::string const& host);

std::string getRendererCalibrationFilepath();

std::string getCalibrationFilepath(std::string const& path_if_not_renderer);

std::vector<float> generateEquirectSampletex(int width, int height);

std::map<std::string, ::al::NodeConfiguration> getSphereNodes();

}  // namespace sphere

}  // namespace al

#endif
