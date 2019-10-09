#include "al/graphics/al_Lens.hpp"
#include <math.h>
#include "al/math/al_Constants.hpp"

namespace al {

Lens ::Lens(double fovy_, double nearClip, double farClip, double focalLength,
            double eyeSep)
    : mNear(nearClip),
      mFar(farClip),
      mFocalLength(focalLength),
      mEyeSep(eyeSep) {
  fovy(fovy_);
}

Lens& Lens::fovy(double v) {
  static const double cDeg2Rad = M_PI / 180.;
  mFovy = v;
  mTanFOV = tan(fovy() * cDeg2Rad * 0.5);
  return *this;
}

Lens& Lens::fovx(double v, double aspect) {
  fovy(Lens::getFovyForFovX(v, aspect));
  return *this;
}

double Lens::getFovyForHeight(double height, double depth) {
  return 2. * M_RAD2DEG * atan(height / depth);
}

double Lens::getFovyForFovX(double fovx, double aspect) {
  double farW = tan(0.5 * fovx * M_DEG2RAD);
  return 2. * M_RAD2DEG * atan(farW / aspect);
}

}  // namespace al
