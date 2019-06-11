#include "al_OpenVRDomain.hpp"

using namespace al;

bool OpenVRDomain::initialize(ComputationDomain *parent) {
#ifdef AL_EXT_OPENVR
  // A graphics context is needed to initialize OpenVR
  if(!mOpenVR.init()) {
    //      std::cerr << "ERROR: OpenVR init returned error" << std::endl;
    return false;
  }
  if (dynamic_cast<GraphicsDomain *>(parent)) {
    g = &dynamic_cast<GraphicsDomain *>(parent)->graphics();
  }
  return true;
#else
  std::cerr << "Not building wiht OpenVR support" << std::endl;
  return false;
#endif
}

bool OpenVRDomain::tick() {
#ifdef AL_EXT_OPENVR
  // Update traking and controller data;
  mOpenVR.update();
  mOpenVR.draw(drawSceneFunc, *g);
#endif
  return true;
}

bool OpenVRDomain::cleanup(ComputationDomain *parent) { return true; }
