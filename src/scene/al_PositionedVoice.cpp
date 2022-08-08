#include "al/scene/al_PositionedVoice.hpp"

//#include "al/graphics/al_Shapes.hpp"

#include <algorithm>

using namespace std;
using namespace al;

bool PositionedVoice::setTriggerParams(float *pFields, int numFields) {
  bool ok = SynthVoice::setTriggerParams(pFields, numFields);
  if (numFields ==
      (int)mTriggerParams.size() +
          8) { // If seven extra, it means pose and size are there too
    pFields += mTriggerParams.size();
    double x = *pFields++;
    double y = *pFields++;
    double z = *pFields++;
    double qw = *pFields++;
    double qx = *pFields++;
    double qy = *pFields++;
    double qz = *pFields++;
    mPose.set({Vec3d(x, y, z), Quatd(qw, qx, qy, qz)});
    mSize.set(*pFields);
  } else {
    ok = false;
  }
  return ok;
}

bool PositionedVoice::setTriggerParams(const std::vector<float> &pFields,
                                       bool noCalls) {
  bool ok = SynthVoice::setTriggerParams(pFields, noCalls);
  if (pFields.size() ==
      mTriggerParams.size() +
          8) { // If seven extra, it means pose and size are there too
    size_t index = mTriggerParams.size();
    double x = pFields[index++];
    double y = pFields[index++];
    double z = pFields[index++];
    double qw = pFields[index++];
    double qx = pFields[index++];
    double qy = pFields[index++];
    double qz = pFields[index++];
    if (noCalls) {
      mPose.setNoCalls({Vec3d(x, y, z), Quatd(qw, qx, qy, qz)});
      mSize.setNoCalls(pFields[index++]);
    } else {
      mPose.set({Vec3d(x, y, z), Quatd(qw, qx, qy, qz)});
      mSize.set(pFields[index++]);
    }
  } else {
    //            std::cout << "Not setting position for voice" << std::endl;
    ok = false;
  }
  return ok;
  //  return setTriggerParams(pFields.data(), pFields.size(), noCalls);
}

bool PositionedVoice::setTriggerParams(const std::vector<VariantValue> &pFields,
                                       bool noCalls) {
  bool ok = SynthVoice::setTriggerParams(pFields);
  if (pFields.size() ==
      mTriggerParams.size() +
          8) { // If eight extra, it means pose and size are there too
    size_t index = mTriggerParams.size();

    double x;
    if (pFields[index].type() == VariantType::VARIANT_DOUBLE) {
      x = pFields[index++].get<double>();
    } else {
      x = pFields[index++].get<float>();
    }
    double y;

    if (pFields[index].type() == VariantType::VARIANT_DOUBLE) {
      y = pFields[index++].get<double>();
    } else {
      y = pFields[index++].get<float>();
    }
    double z;
    if (pFields[index].type() == VariantType::VARIANT_DOUBLE) {
      z = pFields[index++].get<double>();
    } else {
      z = pFields[index++].get<float>();
    }
    double qw;
    if (pFields[index].type() == VariantType::VARIANT_DOUBLE) {
      qw = pFields[index++].get<double>();
    } else {
      qw = pFields[index++].get<float>();
    }
    double qx;
    if (pFields[index].type() == VariantType::VARIANT_DOUBLE) {
      qx = pFields[index++].get<double>();
    } else {
      qx = pFields[index++].get<float>();
    }
    double qy;
    if (pFields[index].type() == VariantType::VARIANT_DOUBLE) {
      qy = pFields[index++].get<double>();
    } else {
      qy = pFields[index++].get<float>();
    }
    double qz;
    if (pFields[index].type() == VariantType::VARIANT_DOUBLE) {
      qz = pFields[index++].get<double>();
    } else {
      qz = pFields[index++].get<float>();
    }
    if (noCalls) {
      mPose.setNoCalls({Vec3d(x, y, z), Quatd(qw, qx, qy, qz)});
      mSize.setNoCalls(pFields[index++].get<float>());
    } else {
      mPose.set({Vec3d(x, y, z), Quatd(qw, qx, qy, qz)});
      mSize.set(pFields[index++].get<float>());
    }
  } else {
    //            std::cout << "Not setting position for voice" << std::endl;
    ok = false;
  }
  return ok;
}

void PositionedVoice::applyTransformations(Graphics &g) {
  auto pose = mPose.get();
  g.translate(pose.x(), pose.y(), pose.z());
  g.rotate(pose.quat());
  g.scale(size());
}
