#include "al/io/al_AudioIO.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring> /* memset() */
#include <cstring>
#include <iostream>
#include <string>


#include "RtAudio.h"

namespace al {

static void warn(const char *msg, const char *src) {
  fprintf(stderr, "%s%swarning: %s\n", src, src[0] ? " " : "", msg);
}

struct AudioBackendData {
  RtAudio audio;
  RtAudio::StreamParameters iParams, oParams;
  RtAudio::StreamOptions options;
};

AudioBackend::AudioBackend() {
  mBackendData = std::make_shared<AudioBackendData>();
}

bool AudioBackend::isOpen() const {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  return data->audio.isStreamOpen();
}

bool AudioBackend::isRunning() const {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  return data->audio.isStreamRunning();
}

bool AudioBackend::error() const { return false; }

void AudioBackend::printError(const char *text) const {
  //    if(error()){
  //      fprintf(stderr, "%s: %s\n", text, Pa_GetErrorText(mErrNum));
  //    }
}

void AudioBackend::printInfo() const {
  //    audio.getVersion()
  //    const PaStreamInfo * sInfo = Pa_GetStreamInfo(mStream);
  //    if(sInfo){
  //      printf("In Latency:  %.0f ms\nOut Latency: %0.f ms\nSample Rate:
  //      %0.f Hz\n",
  //             sInfo->inputLatency * 1000., sInfo->outputLatency * 1000.,
  //             sInfo->sampleRate);
  //    }
}

bool AudioBackend::supportsFPS(double fps) {
  // const RtAudio::StreamParameters * pi = iParams.nChannels  == 0 ? nullptr
  // : &iParams;
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  const RtAudio::StreamParameters *po =
      data->oParams.nChannels == 0 ? nullptr : &data->oParams;

  if (!po)
    return false;

  unsigned int f = fps;
  auto const &supported = data->audio.getDeviceInfo(po->deviceId).sampleRates;
  for (auto const &r : supported) {
    if (r == f) {
      // std::cout << "RtAudioBackend::supportsFPS, rate " << f << "
      // supported" << std::endl;
      return true;
    }
  }

  // std::cout << "rate " << f << " not supported" << std::endl;
  return false;

  //    mErrNum = Pa_IsFormatSupported(pi, po, fps);
  //    printError("AudioIO::Impl::supportsFPS");
  //    return paFormatIsSupported == mErrNum;
  // return true; // FIXME return correct value...
}

void AudioBackend::inDevice(int index) {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  data->iParams.deviceId = index;
  data->iParams.firstChannel = 0;
  if (data->iParams.nChannels < 1) {
    data->iParams.nChannels = AudioDevice(index).channelsInMax();
  }
}

void AudioBackend::outDevice(int index) {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  data->oParams.deviceId = index;
  data->oParams.firstChannel = 0;
  if (data->oParams.nChannels < 1) {
    data->oParams.nChannels = AudioDevice(index).channelsOutMax();
  }
}

int AudioBackend::inDeviceChans() {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  return (int)data->iParams.nChannels;
}

int AudioBackend::outDeviceChans() {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  return (int)data->oParams.nChannels;
}

void AudioBackend::setInDeviceChans(int num) {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  data->iParams.nChannels = num;
}

void AudioBackend::setOutDeviceChans(int num) {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  data->oParams.nChannels = num;
}

double AudioBackend::time() {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  return data->audio.getStreamTime();
}

static int rtaudioCallback(void *output, void *input, unsigned int frameCount,
                           double streamTime, RtAudioStreamStatus status,
                           void *userData);

bool AudioBackend::open(int framesPerSecond, unsigned int framesPerBuffer,
                        void *userdata) {
  assert(framesPerBuffer != 0 && framesPerSecond != 0 && userdata != NULL);
  // Set the same number of channels for both input and output.
  //    unsigned int bufferBytes, bufferFrames = 512;

  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  unsigned int deviceBufferSize = static_cast<unsigned int>(framesPerBuffer);
  auto *ip = data->iParams.nChannels > 0 ? &data->iParams : nullptr;
  auto *op = data->oParams.nChannels > 0 ? &data->oParams : nullptr;
  try {
    data->audio.openStream(op, ip, RTAUDIO_FLOAT32, framesPerSecond,
                           &deviceBufferSize, rtaudioCallback, userdata,
                           &data->options);
  } catch (RtAudioError &e) {
    e.printMessage();
    return false;
  }

  if (deviceBufferSize != framesPerBuffer) {
    printf("WARNING: Device opened with buffer size: %d", deviceBufferSize);
  }
  return true;
}

bool AudioBackend::close() {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  if (data->audio.isStreamOpen()) {
    if (data->audio.isStreamRunning()) {
      try {
        data->audio.stopStream();
      } catch (RtAudioError &e) {
        e.printMessage();
      }
    }
    if (data->audio.isStreamOpen()) {
      data->audio.closeStream();
    }
  }

  return true;
}

bool AudioBackend::start(int framesPerSecond, int framesPerBuffer,
                         void *userdata) {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  try {
    data->audio.startStream();
  } catch (RtAudioError &e) {
    e.printMessage();
    //          goto cleanup;
    return false;
  }
  return true;
}

bool AudioBackend::stop() {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  try {
    if (data->audio.isStreamRunning()) {
      data->audio.stopStream();
    }
  } catch (RtAudioError &e) {
    e.printMessage();
    return false;
  }
  return true;
}

double AudioBackend::cpu() { return -1.0; }

AudioDevice AudioBackend::defaultInput() {
  RtAudio audio_;
  return AudioDevice(audio_.getDefaultInputDevice());
}

AudioDevice AudioBackend::defaultOutput() {
  RtAudio audio_;
  return AudioDevice(audio_.getDefaultOutputDevice());
}

int AudioBackend::numDevices() {
  RtAudio audio_;
  return audio_.getDeviceCount();
}

static int rtaudioCallback(void *output, void *input, unsigned int frameCount,
                           double streamTime, RtAudioStreamStatus status,
                           void *userData) {
  if (status) {
    std::cout << "Stream underflow detected!" << std::endl;
  }

  AudioIO &io = *(AudioIO *)userData;

  assert(frameCount == (unsigned)io.framesPerBuffer());

  if (input != NULL) {
    const float *inBuffers = (const float *)input;
    float *hwInBuffer = const_cast<float *>(io.inBuffer(0));
    for (unsigned int frame = 0; frame < io.framesPerBuffer(); frame++) {
      for (int i = 0; i < io.channelsInDevice(); i++) {
        hwInBuffer[i * frameCount + frame] = *inBuffers++;
      }
    }
  }

  if (io.autoZeroOut())
    io.zeroOut();

  io.processAudio(); // call callback

  // apply smoothly-ramped gain to all output channels
  if (io.usingGain()) {
    float dgain = (io.mGain - io.mGainPrev) / frameCount;

    for (int j = 0; j < io.channelsOutDevice(); ++j) {
      float *out = io.outBuffer(j);
      float gain = io.mGainPrev;

      for (unsigned i = 0; i < frameCount; ++i) {
        out[i] *= gain;
        gain += dgain;
      }
    }

    io.mGainPrev = io.mGain;
  }

  // kill pesky nans so we don't hurt anyone's ears
  if (io.zeroNANs()) {
    for (unsigned i = 0; i < unsigned(frameCount * io.channelsOutDevice());
         ++i) {
      float &s = (&io.out(0, 0))[i];
      // if(isnan(s)) s = 0.f;
      if (s != s)
        s = 0.f; // portable isnan; only nans do not equal themselves
    }
  }

  if (io.clipOut()) {
    for (unsigned i = 0; i < unsigned(frameCount * io.channelsOutDevice());
         ++i) {
      float &s = (&io.out(0, 0))[i];
      if (s < -1.f)
        s = -1.f;
      else if (s > 1.f)
        s = 1.f;
    }
  }

  float *outBuffers = (float *)output;

  float *finalOutBuffer = const_cast<float *>(io.outBuffer(0));
  for (unsigned int frame = 0; frame < io.framesPerBuffer(); frame++) {
    for (int i = 0; i < io.channelsOutDevice(); i++) {
      *outBuffers++ = finalOutBuffer[i * frameCount + frame];
    }
  }

  return 0;
}

void AudioBackend::channels(int num, bool forOutput) {
  if (isOpen()) {
    warn("the number of channels cannnot be set with the stream open",
         "AudioIO");
    return;
  }

  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  RtAudio::StreamParameters *params =
      forOutput ? &data->oParams : &data->iParams;

  if (num == 0) {
    // params->device = paNoDevice;
    params->nChannels = 0;
    return;
  }
  RtAudio::DeviceInfo info;
  try {
    info = data->audio.getDeviceInfo(params->deviceId);
  } catch (RtAudioError &e) {
    e.printMessage();
  }
  if (!info.probed) {
    if (forOutput)
      warn("attempt to set number of channels on invalid output device",
           "AudioIO");
    else
      warn("attempt to set number of channels on invalid input device",
           "AudioIO");
    return; // this particular device is not open, so return
  }

  // compute number of channels to give PortAudio
  int maxChans = (int)(forOutput ? info.outputChannels : info.inputChannels);

  // -1 means open all channels
  if (-1 == num) {
    num = maxChans;
#ifdef AL_LINUX
    /* The default device can report an insane number of max channels,
      presumably because it's being remapped through a software mixer;
      Opening all of them can cause an assertion dump in snd_pcm_area_copy
      so we limit "all channels" to a reasonable number.*/
    if (num >= 128)
      num = 2;
#endif
  } else {
    num = std::min(num, maxChans);
  }

  params->nChannels = num;
}

void AudioBackend::setStreamName(std::string name) {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  data->options.streamName = name;
  if (isRunning()) {
    std::cout
        << __FUNCTION__
        << ": Audio stream is already running. Name will be set on next start."
        << std::endl;
  }
}

std::string AudioBackend::deviceName(int num) {
  RtAudio rt;
  std::string name;
  try {
    RtAudio::DeviceInfo info = rt.getDeviceInfo(num);
    name = info.name;
  } catch (RtAudioError &e) {
    std::cerr << "deviceName(" << num << ") " << e.getMessage() << std::endl;
  }
  return name;
}

int AudioBackend::deviceMaxInputChannels(int num) {
  RtAudio rt;
  unsigned int numChannels = 0;
  try {
    RtAudio::DeviceInfo info = rt.getDeviceInfo(num);
    numChannels = info.inputChannels;
  } catch (RtAudioError &e) {
    std::cerr << "deviceMaxInputChannels(" << num << ") " << e.getMessage()
              << std::endl;
  }
  return numChannels;
}

int AudioBackend::deviceMaxOutputChannels(int num) {
  RtAudio rt;
  unsigned int numChannels = 0;
  try {
    RtAudio::DeviceInfo info = rt.getDeviceInfo(num);
    numChannels = info.outputChannels;
  } catch (RtAudioError &e) {
    std::cerr << "deviceMaxOutputChannels(" << num << ") " << e.getMessage()
              << std::endl;
  }
  return numChannels;
}

double AudioBackend::devicePreferredSamplingRate(int num) {
  RtAudio rt;
  double sr = -0.0;
  try {
    RtAudio::DeviceInfo info = rt.getDeviceInfo(num);
    sr = info.preferredSampleRate;
  } catch (RtAudioError &e) {
    std::cerr << "devicePreferredSamplingRate(" << num << ") " << e.getMessage()
              << std::endl;
  }
  return sr;
}

}
