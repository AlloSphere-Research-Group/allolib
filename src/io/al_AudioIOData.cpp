#include "al/io/al_AudioIOData.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring> /* memset() */

namespace al {

//==============================================================================

AudioDeviceInfo::AudioDeviceInfo(int deviceNum)
    : mID(deviceNum),
      mChannelsInMax(0),
      mChannelsOutMax(0),
      mDefaultSampleRate(0.0) {}

bool AudioDeviceInfo::valid() const { return true; }
int AudioDeviceInfo::id() const { return mID; }
const char *AudioDeviceInfo::name() const { return mName; }
int AudioDeviceInfo::channelsInMax() const { return mChannelsInMax; }
int AudioDeviceInfo::channelsOutMax() const { return mChannelsOutMax; }
double AudioDeviceInfo::defaultSampleRate() const { return mDefaultSampleRate; }

void AudioDeviceInfo::setName(char *name) {
  strncpy(mName, name, 127);
  mName[127] = '\0';
}
void AudioDeviceInfo::setID(int iD) { mID = iD; }
void AudioDeviceInfo::setChannelsInMax(int num) { mChannelsInMax = num; }
void AudioDeviceInfo::setChannelsOutMax(int num) { mChannelsOutMax = num; }
void AudioDeviceInfo::setDefaultSampleRate(double rate) {
  mDefaultSampleRate = rate;
}

//==============================================================================

AudioIOData::AudioIOData(void *userData)
    : mGain(1),
      mGainPrev(1),
      mUser(userData),
      mFrame(0),
      mFramesPerBuffer(512),
      mFramesPerSecond(44100),
      mBufI(nullptr),
      mBufO(nullptr),
      mBufB(nullptr),
      mBufT(nullptr),
      mNumI(0),
      mNumO(0),
      mNumB(0) {}

AudioIOData::~AudioIOData() {
  deleteBuf(mBufI);
  deleteBuf(mBufO);
  deleteBuf(mBufB);
  deleteBuf(mBufT);
}

void AudioIOData::zeroBus() { zero(mBufB, framesPerBuffer() * mNumB); }
void AudioIOData::zeroOut() { zero(mBufO, channelsOut() * framesPerBuffer()); }

void AudioIOData::channelsBus(int num) {
  resize(mBufB, num * mFramesPerBuffer);
  mNumB = num;
}

void AudioIOData::channels(int num, bool forOutput) {
  // printf("Requested %d %s channels\n", num, forOutput?"output":"input");

  const int oldChans = channels(forOutput);

  if (oldChans != num) {
    forOutput ? mNumO = num : mNumI = num;
    resizeBuffer(forOutput);
  }
  // printf("Set %d %s channels\n", forOutput?mNumO:mNumI,
  // forOutput?"output":"input");
}

void AudioIOData::channelsIn(int n) { channels(n, false); }
void AudioIOData::channelsOut(int n) { channels(n, true); }

void AudioIOData::framesPerSecond(
    double v) {  // printf("AudioIO::fps(%f)\n", v);
  if (framesPerSecond() != v) {
    mFramesPerSecond = v;
  }
}

void AudioIOData::framesPerBuffer(unsigned int n) {
  if (framesPerBuffer() != n) {
    mFramesPerBuffer = n;
    resizeBuffer(true);
    resizeBuffer(false);
    channelsBus(AudioIOData::channelsBus());
    resize(mBufT, mFramesPerBuffer);
  }
}

void AudioIOData::resizeBuffer(bool forOutput) {
  float *&buffer = forOutput ? mBufO : mBufI;
  unsigned int &chans = forOutput ? mNumO : mNumI;

  if (chans > 0 && mFramesPerBuffer > 0) {
    int n = resize(buffer, chans * mFramesPerBuffer);
    if (0 == n) chans = 0;
  } else {
    deleteBuf(buffer);
  }
}

int AudioIOData::channels(bool forOutput) const {
  return forOutput ? channelsOut() : channelsIn();
}

unsigned int AudioIOData::channelsIn() const { return mNumI; }
unsigned int AudioIOData::channelsOut() const { return mNumO; }
unsigned int AudioIOData::channelsBus() const { return mNumB; }

double AudioIOData::framesPerSecond() const { return mFramesPerSecond; }
uint64_t AudioIOData::framesPerBuffer() const { return mFramesPerBuffer; }
double AudioIOData::secondsPerBuffer() const {
  return (double)framesPerBuffer() / framesPerSecond();
}

}  // namespace al
