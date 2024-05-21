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

#include "portaudio.h"

namespace al {

static void warn(const char *msg, const char *src) {
  fprintf(stderr, "%s%swarning: %s\n", src, src[0] ? " " : "", msg);
}

// #ifdef AL_AUDIO_DUMMY

// struct AudioBackendData {
//   int numOutChans, numInChans;
//   std::string streamName;
// };

// AudioBackend::AudioBackend() {
//   mBackendData = std::make_shared<AudioBackendData>();
//   static_cast<AudioBackendData *>(mBackendData.get())->numOutChans = 2;
//   static_cast<AudioBackendData *>(mBackendData.get())->numInChans = 2;
//   mOpen = false;
//   mRunning = false;
// }

// bool AudioBackend::isOpen() const { return mOpen; }

// bool AudioBackend::isRunning() const { return mRunning; }

// bool AudioBackend::error() const { return false; }

// void AudioBackend::printError(const char *text) const {
//   if (error()) {
//     fprintf(stderr, "%s: Dummy error.\n", text);
//   }
// }

// void AudioBackend::printInfo() const {
//   printf("Using dummy backend (no audio).\n");
// }

// bool AudioBackend::supportsFPS(double fps) { return true; }

// void AudioBackend::inDevice(int index) { return; }

// void AudioBackend::outDevice(int index) { return; }

// void AudioBackend::channels(int num, bool forOutput) {
//   if (forOutput) {
//     setOutDeviceChans(num);
//   } else {
//     setInDeviceChans(num);
//   }
// }

// int AudioBackend::inDeviceChans() { return 2; }

// int AudioBackend::outDeviceChans() { return 2; }

// void AudioBackend::setInDeviceChans(int num) {
//   static_cast<AudioBackendData *>(mBackendData.get())->numInChans = num;
// }

// void AudioBackend::setOutDeviceChans(int num) {
//   static_cast<AudioBackendData *>(mBackendData.get())->numOutChans = num;
// }

// double AudioBackend::time() { return 0.0; }

// bool AudioBackend::open(int framesPerSecond, unsigned int framesPerBuffer,
//                         void *userdata) {
//   mOpen = true;
//   return true;
// }

// bool AudioBackend::close() {
//   mOpen = false;
//   return true;
// }

// bool AudioBackend::start(int framesPerSecond, int framesPerBuffer,
//                          void *userdata) {
//   mRunning = true;
//   return true;
// }

// bool AudioBackend::stop() {
//   mRunning = false;
//   return true;
// }

// double AudioBackend::cpu() { return 0.0; }

// AudioDevice AudioBackend::defaultInput() { return AudioDevice(0); }

// AudioDevice AudioBackend::defaultOutput() { return AudioDevice(0); }

// int AudioBackend::numDevices() { return 1; }

// int AudioBackend::deviceMaxInputChannels(int num) { return 2; }

// int AudioBackend::deviceMaxOutputChannels(int num) { return 2; }

// double AudioBackend::devicePreferredSamplingRate(int num) { return 44100; }

// void AudioBackend::setStreamName(std::string name) {
//   AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
//   data->streamName = name;
// }

// std::string AudioBackend::deviceName(int num) { return "dummy_device"; }

// #endif



struct InitSingleton {
  InitSingleton() { mCleanUp = paNoError == Pa_Initialize(); }
  ~InitSingleton() {
    if (mCleanUp) {
      Pa_Terminate();
    }
  }
  bool mCleanUp;
};

static InitSingleton dummy;

struct AudioBackendData {
  PaStreamParameters mInParams,
      mOutParams;          // Input and output stream parameters
  PaStream *mStream;       // i/o stream
  mutable PaError mErrNum; // Most recent error number
  std::string streamName;
};

AudioBackend::AudioBackend() {
  mBackendData = std::make_shared<AudioBackendData>();
  static_cast<AudioBackendData *>(mBackendData.get())->mStream = nullptr;
  static_cast<AudioBackendData *>(mBackendData.get())->mErrNum = paNoError;
}
bool AudioBackend::isOpen() const { return mOpen; }

bool AudioBackend::isRunning() const { return mRunning; }

bool AudioBackend::error() const {
  return static_cast<AudioBackendData *>(mBackendData.get())->mErrNum !=
         paNoError;
}

void AudioBackend::printError(const char *text) const {
  if (error()) {
    fprintf(stderr, "%s: %s\n", text,
            Pa_GetErrorText(
                static_cast<AudioBackendData *>(mBackendData.get())->mErrNum));
  }
}

void AudioBackend::printInfo() const {
  const PaStreamInfo *sInfo = Pa_GetStreamInfo(
      static_cast<AudioBackendData *>(mBackendData.get())->mStream);
  if (sInfo) {
    printf("In Latency:  %.0f ms\nOut Latency: %0.f ms\nSample Rate: %0.f Hz\n",
           sInfo->inputLatency * 1000., sInfo->outputLatency * 1000.,
           sInfo->sampleRate);
  }
}

bool AudioBackend::supportsFPS(double fps) {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  const PaStreamParameters *pi =
      data->mInParams.channelCount == 0 ? 0 : &data->mInParams;
  const PaStreamParameters *po =
      data->mOutParams.channelCount == 0 ? 0 : &data->mOutParams;
  data->mErrNum = Pa_IsFormatSupported(pi, po, fps);
  printError("AudioIO::Impl::supportsFPS");
  return paFormatIsSupported == data->mErrNum;
}

void AudioBackend::inDevice(int index) {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  data->mInParams.device = index;
  const PaDeviceInfo *dInfo = Pa_GetDeviceInfo(data->mInParams.device);
  if (dInfo)
    data->mInParams.suggestedLatency = dInfo->defaultLowInputLatency; // for RT
  data->mInParams.sampleFormat = paFloat32 | paNonInterleaved;
  data->mInParams.hostApiSpecificStreamInfo = NULL;
}

void AudioBackend::outDevice(int index) {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  data->mOutParams.device = index;
  const PaDeviceInfo *dInfo = Pa_GetDeviceInfo(data->mOutParams.device);
  if (dInfo)
    data->mOutParams.suggestedLatency =
        dInfo->defaultLowOutputLatency; // for RT
  data->mOutParams.sampleFormat = paFloat32 | paNonInterleaved;
  data->mOutParams.hostApiSpecificStreamInfo = NULL;
}

void AudioBackend::channels(int num, bool forOutput) {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  if (isOpen()) {
    warn("the number of channels cannnot be set with the stream open",
         "AudioIO");
    return;
  }

  PaStreamParameters *params = forOutput ? &data->mOutParams : &data->mInParams;

  if (num == 0) {
    // params->device = paNoDevice;
    params->channelCount = 0;
    return;
  }

  const PaDeviceInfo *info = Pa_GetDeviceInfo(params->device);
  if (0 == info) {
    if (forOutput)
      warn("attempt to set number of channels on invalid output device",
           "AudioIO");
    else
      warn("attempt to set number of channels on invalid input device",
           "AudioIO");
    return; // this particular device is not open, so return
  }

  // compute number of channels to give PortAudio
  int maxChans =
      (int)(forOutput ? info->maxOutputChannels : info->maxInputChannels);

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

  params->channelCount = num;
}

int AudioBackend::inDeviceChans() {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  return (int)data->mInParams.channelCount;
}

int AudioBackend::outDeviceChans() {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  return (int)data->mOutParams.channelCount;
}

void AudioBackend::setInDeviceChans(int num) {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  data->mInParams.channelCount = num;
}

void AudioBackend::setOutDeviceChans(int num) {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  data->mOutParams.channelCount = num;
}

double AudioBackend::time() {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  return (double)Pa_GetStreamTime(data->mStream);
}

static int paCallback(const void *input, void *output, unsigned long frameCount,
                      const PaStreamCallbackTimeInfo *timeInfo,
                      PaStreamCallbackFlags statusFlags, void *userData);

bool AudioBackend::open(int framesPerSecond, unsigned int framesPerBuffer,
                        void *userdata) {
  assert(framesPerBuffer != 0 && framesPerSecond != 0 && userdata != NULL);
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());

  data->mErrNum = paNoError;

  if (!(isOpen() || isRunning())) {
    PaStreamParameters *inParams = &data->mInParams;
    PaStreamParameters *outParams = &data->mOutParams;

    // Must pass in 0s for input- or output-only streams.
    // Stream will not be opened if no device or channel count is zero
    if ((paNoDevice == inParams->device) || (0 == inParams->channelCount))
      inParams = 0;
    if ((paNoDevice == outParams->device) || (0 == outParams->channelCount))
      outParams = 0;

    data->mErrNum = Pa_OpenStream(
        &data->mStream,  // PortAudioStream **
        inParams,        // PaStreamParameters * in
        outParams,       // PaStreamParameters * out
        framesPerSecond, // frames/sec (double)
        framesPerBuffer, // frames/buffer (unsigned long)
        paNoFlag,        // paNoFlag, paClipOff, paDitherOff
        paCallback,      // static callback function (PaStreamCallback *)
        userdata);

    mOpen = paNoError == data->mErrNum;
  }

  printError("Error in al::AudioIO::open()");
  return paNoError == data->mErrNum;
}

bool AudioBackend::close() {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  data->mErrNum = paNoError;
  if (mOpen)
    data->mErrNum = Pa_CloseStream(data->mStream);
  if (paNoError == data->mErrNum) {
    mOpen = false;
    mRunning = false;
  }
  return paNoError == data->mErrNum;
}

bool AudioBackend::start(int framesPerSecond, int framesPerBuffer,
                         void *userdata) {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  data->mErrNum = paNoError;
  if (!isOpen()) {
    open(framesPerSecond, framesPerBuffer, userdata);
  }
  if (isOpen() && !isRunning())
    data->mErrNum = Pa_StartStream(data->mStream);
  if (paNoError == data->mErrNum)
    mRunning = true;
  printError("Error in AudioIO::start()");
  return paNoError == data->mErrNum;
}

bool AudioBackend::stop() {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  data->mErrNum = paNoError;
  if (mRunning) {
    data->mErrNum = Pa_StopStream(data->mStream);
  }
  if (paNoError == data->mErrNum)
    mRunning = false;
  return paNoError == data->mErrNum;
}

double AudioBackend::cpu() {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  return Pa_GetStreamCpuLoad(data->mStream);
}

AudioDevice AudioBackend::defaultInput() {
  return AudioDevice(Pa_GetDefaultInputDevice());
}

AudioDevice AudioBackend::defaultOutput() {
  return AudioDevice(Pa_GetDefaultOutputDevice());
}

bool AudioBackend::deviceIsValid(int num) {
  return Pa_GetDeviceInfo(num) != nullptr;
}

int AudioBackend::deviceMaxInputChannels(int num) {
  const PaDeviceInfo *info = Pa_GetDeviceInfo(num);
  return info->maxInputChannels;
}

int AudioBackend::deviceMaxOutputChannels(int num) {
  const PaDeviceInfo *info = Pa_GetDeviceInfo(num);
  return info->maxOutputChannels;
}

double AudioBackend::devicePreferredSamplingRate(int num) {
  const PaDeviceInfo *info = Pa_GetDeviceInfo(num);
  return info->defaultSampleRate;
}

void AudioBackend::setStreamName(std::string name) {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  data->streamName = name;
  //  if (isRunning()) {
  //    std::cout
  //        << __FUNCTION__
  //        << ": Audio stream is already running. Name will be set on next
  //        start."
  //        << std::endl;
  //  }
}
std::string AudioBackend::deviceName(int num) {
  const PaDeviceInfo *info = Pa_GetDeviceInfo(num);
  char name[128];
  strncpy(name, info->name, 127);
  name[127] = '\0';
  return std::string(name);
}

int AudioBackend::numDevices() { return Pa_GetDeviceCount(); }

static int paCallback(const void *input, void *output, unsigned long frameCount,
                      const PaStreamCallbackTimeInfo *timeInfo,
                      PaStreamCallbackFlags statusFlags, void *userData) {
  AudioIO &io = *(AudioIO *)userData;

  assert(frameCount == (unsigned)io.framesPerBuffer());
  const float **inBuffers = (const float **)input;
  for (int i = 0; i < io.channelsInDevice(); i++) {
    memcpy(const_cast<float *>(&io.in(i, 0)), inBuffers[i],
           frameCount * sizeof(float));
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

  float **outBuffers = (float **)output;
  for (int i = 0; i < io.channelsOutDevice(); i++) {
    memcpy(outBuffers[i], const_cast<float *>(&io.out(i, 0)),
           frameCount * sizeof(float));
  }

  return 0;
}

// #endif // AL_USE_PORTAUDIO

} // namespace al
