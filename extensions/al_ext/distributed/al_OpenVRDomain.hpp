#ifndef OPENVRDOMAIN_H
#define OPENVRDOMAIN_H

#include <iostream>
#include <functional>

#include "al_ComputationDomain.hpp"
#include "al_GraphicsDomain.hpp"

 // ----------------------------------------------------------------
// The AL_EXT_OPENVR macro is set if OpenVR is found.
#ifdef AL_EXT_OPENVR
#include "al_ext/openvr/al_OpenVRWrapper.hpp"
#endif

namespace al {


class OpenVRDomain: public SynchronousDomain {
public:

  ~OpenVRDomain() {}

  // Domain management functions
  bool initialize(ComputationDomain *parent = nullptr) override;
  bool tick() override;
  bool cleanup(ComputationDomain *parent = nullptr) override;

  void setDrawFunction(std::function<void(Graphics &)> func) {
    drawSceneFunc = func;
  }

  Graphics *g;

private:

  std::function<void(Graphics &)> drawSceneFunc = [](Graphics &g){ g.clear(0, 0, 1.0); };

#ifdef AL_EXT_OPENVR
    al::OpenVRWrapper mOpenVR;
#endif
};

}


#endif // OPENVRDOMAIN_H
