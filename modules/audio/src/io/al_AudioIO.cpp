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


namespace al {

static void warn(const char *msg, const char *src) {
  fprintf(stderr, "%s%swarning: %s\n", src, src[0] ? " " : "", msg);
}

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
