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

#ifdef AL_AUDIO_RTAUDIO
#include "RtAudio.h"
#endif

#ifdef AL_AUDIO_PORTAUDIO
#include "portaudio.h"
#endif

namespace al {

static void warn(const char *msg, const char *src) {
  fprintf(stderr, "%s%swarning: %s\n", src, src[0] ? " " : "", msg);
}

#ifdef AL_AUDIO_DUMMY

struct AudioBackendData {
  int numOutChans, numInChans;
  std::string streamName;
};

AudioBackend::AudioBackend() {
  mBackendData = std::make_shared<AudioBackendData>();
  static_cast<AudioBackendData *>(mBackendData.get())->numOutChans = 2;
  static_cast<AudioBackendData *>(mBackendData.get())->numInChans = 2;
  mOpen = false;
  mRunning = false;
}

bool AudioBackend::isOpen() const { return mOpen; }

bool AudioBackend::isRunning() const { return mRunning; }

bool AudioBackend::error() const { return false; }

void AudioBackend::printError(const char *text) const {
  if (error()) {
    fprintf(stderr, "%s: Dummy error.\n", text);
  }
}

void AudioBackend::printInfo() const {
  printf("Using dummy backend (no audio).\n");
}

bool AudioBackend::supportsFPS(double fps) { return true; }

void AudioBackend::inDevice(int index) { return; }

void AudioBackend::outDevice(int index) { return; }

void AudioBackend::channels(int num, bool forOutput) {
  if (forOutput) {
    setOutDeviceChans(num);
  } else {
    setInDeviceChans(num);
  }
}

int AudioBackend::inDeviceChans() { return 2; }

int AudioBackend::outDeviceChans() { return 2; }

void AudioBackend::setInDeviceChans(int num) {
  static_cast<AudioBackendData *>(mBackendData.get())->numInChans = num;
}

void AudioBackend::setOutDeviceChans(int num) {
  static_cast<AudioBackendData *>(mBackendData.get())->numOutChans = num;
}

double AudioBackend::time() { return 0.0; }

bool AudioBackend::open(int framesPerSecond, unsigned int framesPerBuffer,
                        void *userdata) {
  mOpen = true;
  return true;
}

bool AudioBackend::close() {
  mOpen = false;
  return true;
}

bool AudioBackend::start(int framesPerSecond, int framesPerBuffer,
                         void *userdata) {
  mRunning = true;
  return true;
}

bool AudioBackend::stop() {
  mRunning = false;
  return true;
}

double AudioBackend::cpu() { return 0.0; }

AudioDevice AudioBackend::defaultInput() { return AudioDevice(0); }

AudioDevice AudioBackend::defaultOutput() { return AudioDevice(0); }

int AudioBackend::numDevices() { return 1; }

int AudioBackend::deviceMaxInputChannels(int num) { return 2; }

int AudioBackend::deviceMaxOutputChannels(int num) { return 2; }

double AudioBackend::devicePreferredSamplingRate(int num) { return 44100; }

void AudioBackend::setStreamName(std::string name) {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  data->streamName = name;
}

std::string AudioBackend::deviceName(int num) { return "dummy_device"; }

#endif

//==============================================================================
#ifdef AL_AUDIO_PORTAUDIO

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

bool AudioBackend::open(int framesPerSecond, unsinged int framesPerBuffer,
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

#endif // AL_USE_PORTAUDIO

#ifdef AL_AUDIO_RTAUDIO

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

#endif

//==============================================================================

AudioDevice::AudioDevice(int deviceNum) : AudioDeviceInfo(deviceNum) {
  if (deviceNum < 0) {
    deviceNum = defaultOutput().id();
  }
  setImpl(deviceNum);
}

AudioDevice::AudioDevice(const std::string &nameKeyword, StreamMode stream)
    : AudioDeviceInfo(0) {
  for (int i = 0; i < numDevices(); ++i) {
    AudioDevice d(i);
    bool bi = (stream & INPUT) && d.hasInput();
    bool bo = (stream & OUTPUT) && d.hasOutput();
    std::string n = d.name();

    if ((bi || bo) && n.find(nameKeyword) != std::string::npos) {
      setImpl(i);
      break;
    }
  }
}

AudioDevice AudioDevice::defaultInput() { return AudioBackend::defaultInput(); }

AudioDevice AudioDevice::defaultOutput() {
  return AudioBackend::defaultOutput();
}

void AudioDevice::initDevices() {}

int AudioDevice::numDevices() { return AudioBackend::numDevices(); }

void AudioDevice::print() const {
  if (!valid()) {
    printf("Invalid device\n");
    return;
  }

  printf("[%2d] %s, ", id(), name());

  int chans = channelsInMax();
  if (chans > 0)
    printf("%2i in, ", chans);
  chans = channelsOutMax();
  if (chans > 0)
    printf("%2i out, ", chans);

  printf("%.0f Hz\n", defaultSampleRate());

  //	PaSampleFormat sampleFormats = info->nativeSampleFormats;

  //	printf("[ ");
  //	if(0 != sampleFormats & paFloat32)		printf("f32 ");
  //	if(0 != sampleFormats & paInt32)		printf("i32 ");
  //	if(0 != sampleFormats & paInt24)		printf("i24 ");
  //	if(0 != sampleFormats & paInt16)		printf("i16 ");
  //	if(0 != sampleFormats & paInt8)			printf("i8 ");
  //	if(0 != sampleFormats & paUInt8)		printf("ui8 ");
  //	printf("], ");

  //	if(info->numSampleRates != -1){
  //		printf("[");
  //		for(int i=0; i<info->numSampleRates; i++){
  //			printf("%f ", info->sampleRates[i]);
  //		}
  //		printf("] Hz");
  //	}
  //	else{
  //		printf("[%.0f <-> %.0f] Hz", info->sampleRates[0],
  // info->sampleRates[1]);
  //	}
  //	printf("\n");
}

void AudioDevice::printAll() {
  for (int i = 0; i < numDevices(); i++) {
    printf("[%2d] ", i);
    AudioDevice dev(i);
    dev.print();
  }
}

void AudioDevice::setImpl(int deviceNum) {
  if (deviceNum >= 0) {
    initDevices();
    mID = deviceNum;
    mChannelsInMax = AudioBackend::deviceMaxInputChannels(deviceNum);
    mChannelsOutMax = AudioBackend::deviceMaxOutputChannels(deviceNum);
    mDefaultSampleRate = AudioBackend::devicePreferredSamplingRate(deviceNum);
    strncpy(mName, AudioBackend::deviceName(deviceNum).c_str(), 128);
    mName[127] = '\0';
    mValid = true;
  } else {
    mValid = false;
  }
}

//==============================================================================

AudioIO::AudioIO()
    : AudioIOData(nullptr), callback(nullptr), mZeroNANs(true), mClipOut(true),
      mAutoZeroOut(true), mBackend{std::make_unique<AudioBackend>()} {}

AudioIO::~AudioIO() { close(); }

void AudioIO::init(void (*callbackA)(AudioIOData &), void *userData,
                   int framesPerBuf, double framesPerSec, int outChansA,
                   int inChansA) {
  AudioDevice dev = AudioDevice::defaultOutput();
  init(callbackA, userData, dev, framesPerBuf, framesPerSec, outChansA,
       inChansA);
}

void AudioIO::init(void (*callbackA)(AudioIOData &), void *userData,
                   AudioDevice &dev, int framesPerBuf, double framesPerSec,
                   int outChansA, int inChansA) {
  // mBackend = std::make_unique<AudioBackend>();
  callback = callbackA;
  user(userData);
  device(dev);
  //	init(outChansA, inChansA);
  channels(inChansA, false);
  channels(outChansA, true);
  this->framesPerBuffer(framesPerBuf);
  this->framesPerSecond(framesPerSec);
}

void AudioIO::initWithDefaults(void (*callback)(AudioIOData &), void *userData,
                               bool use_out, bool use_in,
                               int framesPerBuffer // default 256
) {
  bool use_both = use_out & use_in;
  bool use_either = use_out | use_in;

  auto default_in = AudioDevice::defaultInput();
  auto default_out = AudioDevice::defaultOutput();

  int in_channels = use_in ? default_in.channelsInMax() : 0;
  int out_channels = use_out ? default_out.channelsOutMax() : 0;

  double out_sampling_rate = default_out.defaultSampleRate();
  double in_sampling_rate = default_in.defaultSampleRate();
  double sampling_rate = 0;
  if (use_both) {
    if (out_sampling_rate != in_sampling_rate) {
      std::cout
          << "default sampling rate different for in device and out device\n"
          << "using only out device" << std::endl;
      in_channels = 0;
    }
    sampling_rate = out_sampling_rate;
  } else if (use_either) {
    sampling_rate = use_out ? out_sampling_rate : in_sampling_rate;
  } else {
    std::cout << "not initializing any audio device" << std::endl;
    return;
  }

  std::cout << "AudioIO: using default with\n"
            << "in : [" << default_in.id() << "] " << in_channels
            << " channels \n"
            << "out: [" << default_out.id() << "] " << out_channels
            << " channels \n"
            << "buffer size: " << framesPerBuffer
            << ", sampling rate: " << sampling_rate << std::endl;
  init(callback, userData, framesPerBuffer, sampling_rate, out_channels,
       in_channels);
}

AudioIO &AudioIO::append(AudioCallback &v) {
  mAudioCallbacks.push_back(&v);
  return *this;
}

AudioIO &AudioIO::prepend(AudioCallback &v) {
  mAudioCallbacks.insert(mAudioCallbacks.begin(), &v);
  return *this;
}

AudioIO &AudioIO::insertBefore(AudioCallback &v, AudioCallback &beforeThis) {
  std::vector<AudioCallback *>::iterator pos =
      std::find(mAudioCallbacks.begin(), mAudioCallbacks.end(), &beforeThis);
  if (pos == mAudioCallbacks.begin()) {
    prepend(v);
  } else {
    mAudioCallbacks.insert(--pos, 1, &v);
  }
  return *this;
}

AudioIO &AudioIO::insertAfter(AudioCallback &v, AudioCallback &afterThis) {
  std::vector<AudioCallback *>::iterator pos =
      std::find(mAudioCallbacks.begin(), mAudioCallbacks.end(), &afterThis);
  if (pos == mAudioCallbacks.end()) {
    append(v);
  } else {
    mAudioCallbacks.insert(pos, 1, &v);
  }
  return *this;
}

AudioIO &AudioIO::remove(AudioCallback &v) {
  // the proper way to do it:
  mAudioCallbacks.erase(
      std::remove(mAudioCallbacks.begin(), mAudioCallbacks.end(), &v),
      mAudioCallbacks.end());
  return *this;
}

void AudioIO::deviceIn(const AudioDevice &v) {
  if (v.valid() && v.hasInput()) {
    //		printf("deviceIn: %s, %d\n", v.name(), v.id());
    mInDevice = v;
    mBackend->inDevice(v.id());
    channelsIn(v.channelsInMax());
  } else {
    //    warn("attempt to set input device to a device without inputs",
    //    "AudioIO");
  }
}

void AudioIO::deviceOut(const AudioDevice &v) {
  if (v.valid() && v.hasOutput()) {
    mOutDevice = v;
    mBackend->outDevice(v.id());
    channelsOut(v.channelsOutMax());
  } else {
    //    warn("attempt to set output device to a device without outputs",
    //    "AudioIO");
  }
}

void AudioIO::device(const AudioDevice &v) {
  deviceIn(v);
  deviceOut(v);
}

void AudioIO::channelsBus(int num) {
  if (mBackend->isOpen()) {
    warn("the number of channels cannnot be set with the stream open",
         "AudioIO");
    return;
  }

  AudioIOData::channelsBus(num);
}

void AudioIO::setStreamName(std::string name) { mBackend->setStreamName(name); }

void AudioIO::channels(int num, bool forOutput) {
  // printf("Requested %d %s channels\n", num, forOutput?"output":"input");

  if (mBackend->isOpen()) {
    warn("the number of channels cannnot be set with the stream open",
         "AudioIO");
    return;
  }
  mBackend->channels(num, forOutput);

  if (num == -1) { // Open all device channels?
    num = (forOutput ? channelsOutDevice() : channelsInDevice());
  }

  AudioIOData::channels(num, forOutput);
}

int AudioIO::channelsInDevice() const { return (int)mBackend->inDeviceChans(); }
int AudioIO::channelsOutDevice() const {
  return (int)mBackend->outDeviceChans();
}

bool AudioIO::close() {
  if (mBackend != nullptr) {
    return mBackend->close();
  } else {
    return true;
  }
}

bool AudioIO::open() {
  return mBackend->open(mFramesPerSecond, mFramesPerBuffer, this);
}

void AudioIO::reopen() {
  if (mBackend->isRunning()) {
    close();
    start();
  } else if (mBackend->isOpen()) {
    close();
    open();
  }
}

void AudioIO::framesPerSecond(double v) { // printf("AudioIO::fps(%f)\n", v);
  if (framesPerSecond() != v) {
    if (!supportsFPS(v))
      v = mOutDevice.defaultSampleRate();
    mFramesPerSecond = v;
    reopen();
  }
}

void AudioIO::framesPerBuffer(unsigned int n) {
  if (mBackend->isOpen()) {
    warn("the number of frames/buffer cannnot be set with the stream open",
         "AudioIO");
    return;
  }

  AudioIOData::framesPerBuffer(n);
}

bool AudioIO::start() {
  if (!mBackend->isOpen())
    open();
  return mBackend->start(mFramesPerSecond, mFramesPerBuffer, this);
}

bool AudioIO::stop() { return mBackend->stop(); }

bool AudioIO::supportsFPS(double fps) { return mBackend->supportsFPS(fps); }

void AudioIO::print() const {
  if (mInDevice.id() == mOutDevice.id()) {
    printf("I/O Device:  ");
    mInDevice.print();
  } else {
    printf("Device In:   ");
    mInDevice.print();
    printf("Device Out:  ");
    mOutDevice.print();
  }

  printf("Chans In:    %d (%dD + %dV)\n", channelsIn(), channelsInDevice(),
         channelsIn() - channelsInDevice());
  printf("Chans Out:   %d (%dD + %dV)\n", channelsOut(), channelsOutDevice(),
         channelsOut() - channelsOutDevice());

  mBackend->printInfo();
  printf("Frames/Buf:  %d\n", mFramesPerBuffer);
}

// void AudioIO::processAudio(){ frame(0); if(callback) callback(*this); }
void AudioIO::processAudio() {
  frame(0);
  if (callback)
    callback(*this);

  std::vector<AudioCallback *>::iterator iter = mAudioCallbacks.begin();
  while (iter != mAudioCallbacks.end()) {
    frame(0);
    (*iter++)->onAudioCB(*this);
  }
}

bool AudioIO::isOpen() { return mBackend->isOpen(); }

bool AudioIO::isRunning() { return mBackend->isRunning(); }

double AudioIO::cpu() const { return mBackend->cpu(); }
bool AudioIO::zeroNANs() const { return mZeroNANs; }

void AudioIO::clipOut(bool v) { mClipOut = v; }

double AudioIO::time() const {
  assert(mBackend);
  return mBackend->time();
}
double AudioIO::time(int frame) const {
  return (double)frame / framesPerSecond() + time();
}

} // namespace al
