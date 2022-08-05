#include <cfloat>

#include "al/sphere/al_Meter.hpp"

void Meter::init(const Speakers &sl) {
  addCube(mMesh);
  mSl = sl;
}

void Meter::processSound(AudioIOData &io) {

  if (tempValues.size() != io.channelsOut()) {
    tempValues.resize(io.channelsOut());
    values.resize(io.channelsOut());
    std::cout << "Resizing Meter buffers" << std::endl;
  }
  for (int i = 0; i < io.channelsOut(); i++) {
    tempValues[i] = FLT_MIN;
    auto *outBuf = io.outBuffer(i);
    auto fpb = io.framesPerBuffer();
    for (int samp = 0; samp < fpb; samp++) {
      float val = fabs(*outBuf);
      if (tempValues[i] < val) {
        tempValues[i] = val;
      }
      outBuf++;
    }
    if (tempValues[i] == 0) {
      tempValues[i] = 0.01;
    } else {
      float db = 20.0 * log10(tempValues[i]);
      if (db < -60) {
        tempValues[i] = 0.01;
      } else {
        tempValues[i] = 0.01 + 0.005 * (60 + db);
      }
    }
    if (values[i] > tempValues[i]) {
      values[i] = values[i] - 0.05 * (values[i] - tempValues[i]);
    } else {
      values[i] = tempValues[i];
    }
  }
}

void Meter::draw(Graphics &g) {
  g.polygonLine();
  int index = 0;
  auto spkrIt = mSl.begin();
  g.color(1);
  for (const auto &v : values) {
    if (spkrIt != mSl.end()) {
      // FIXME assumes speakers are sorted by device channel index
      // Should sort inside init()
      if (spkrIt->deviceChannel == index) {
        g.pushMatrix();
        g.scale(1 / 5.0f);
        g.translate(spkrIt->vecGraphics());
        g.scale(0.1 + v * 5);
        g.draw(mMesh);
        g.popMatrix();
        spkrIt++;
      }
    } else {
      spkrIt = mSl.begin();
    }
    index++;
  }
}

void Meter::setMeterValues(float *newValues, size_t count) {
  if (tempValues.size() != count) {
    tempValues.resize(count);
    values.resize(count);
    std::cout << "Resizing Meter buffers" << std::endl;
  }
  count = values.size();
  for (int i = 0; i < count; i++) {
    values[i] = *newValues;
    newValues++;
  }
}
