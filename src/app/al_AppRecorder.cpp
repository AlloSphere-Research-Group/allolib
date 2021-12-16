#include "al/app/al_AppRecorder.hpp"

#include "al/graphics/al_Image.hpp"

#include "Gamma/SoundFile.h"

#include <condition_variable>
#include <thread>

void al::AppRecorder::startRecordingOffline(double totalTime) {
  if (!mAudioDomain || !mWindowDomain) {
    std::cerr << "ERROR starting AppRecorder::startRecordingOffline"
              << std::endl;
    std::cerr << "Call AppRecorder::connectApp() before calling." << std::endl;
  }
  mAudioDomain->stop();
  auto &audioIO = mAudioDomain->audioIO();
  audioIO.zeroOut();

  auto recordingDomain = mGraphicsDomain->newSubDomain<RecordingDomain>();
  double audioTime = 0.0;
  double time = 0.0;
  uint64_t frameCount = 0;
  std::string pathPrefix = "capture_";
  uint16_t counter = 0;
  std::string outPath;
  do {
    outPath = pathPrefix + std::to_string(counter) + "/";
    counter++;
  } while (File::isDirectory(outPath));
  if (!Dir::make(outPath)) {
    std::cerr << "Error creating directory: " << outPath << std::endl;
  }
#ifdef AL_LIBSNDFILE
  gam::SoundFile sf(outPath + "audio.wav");
  sf.channels(audioIO.channelsOut());
  sf.frameRate(audioIO.framesPerSecond());
  if (!sf.openWrite()) {
    std::cerr << "Error opening file for write: " << outPath + "audio.wav"
              << std::endl;
  }
#else
  std::cout << "Warning: libsndfile not available. Not recording audio"
            << std::endl;
#endif
  float *interleavedBuf = (float *)calloc(
      audioIO.channelsOut() * audioIO.framesPerBuffer(), sizeof(float));
  float **outbuf = (float **)malloc(audioIO.channelsOut() * sizeof(float *));
  for (unsigned int i = 0; i < audioIO.channelsOut(); i++) {
    outbuf[i] = audioIO.outBuffer(i);
  }
  // Prepare audio thread
  float audioRunning = true;
  std::condition_variable audioSignal;
  std::mutex audioSignalLock;

  std::thread audioProcessingThread([&]() {
    while (audioRunning && audioTime < totalTime) {
      std::unique_lock<std::mutex> lk(audioSignalLock);
      audioSignal.wait(lk);
      while (audioTime < time) {
        audioIO.frame(0);
        mAudioDomain->onSound(audioIO);
        audioTime += audioIO.framesPerBuffer() / audioIO.framesPerSecond();
#ifdef AL_LIBSNDFILE
        interleave(interleavedBuf, outbuf, audioIO.framesPerBuffer(),
                   audioIO.channelsOut());
        sf.write(interleavedBuf, audioIO.framesPerBuffer());
#endif
      }
    }
  });

  // Run recorder
  mRunning = true;
  int imageCounter = 0;
  while (time < totalTime && mRunning) {
    frameCount++;
    time = frameCount / mGraphicsDomain->fps();
    // Tick simulation and window domains, this will update state and
    // draw unto the main window framebuffer
    mSimulationDomain->setTimeDelta(1.0 / mGraphicsDomain->fps());
    mSimulationDomain->tick();
    {
      // Once we have ticked the simulation domain, we can render audio with new
      // states.
      std::unique_lock<std::mutex> lk(audioSignalLock);
      audioSignal.notify_one();
    }
    mWindowDomain->tick();
    // Now capture the active framebuffer to file
    std::vector<unsigned char> mPixels;

    mPixels.resize(mWindowDomain->window().width() *
                   mWindowDomain->window().height() * 3);
    unsigned char *pixs = &mPixels[0];
    glReadPixels(1, 1, mWindowDomain->window().width(),
                 mWindowDomain->window().height(), GL_RGB, GL_UNSIGNED_BYTE,
                 pixs);
    std::string imagePath =
        outPath + "out" + std::to_string(imageCounter++) + ".png";

    Image::saveImage(imagePath, pixs, mWindowDomain->window().width(),
                     mWindowDomain->window().height(), true);
  }

  // cleanup audio thread
  {
    audioRunning = false;
    std::unique_lock<std::mutex> lk(audioSignalLock);
    audioSignal.notify_one();
  }
  audioProcessingThread.join();
#ifdef AL_LIBSNDFILE
  sf.close();
#endif
  free(interleavedBuf);
  free(outbuf);

  // Put domains back to normal
  mGraphicsDomain->removeSubDomain(recordingDomain);
  mAudioDomain->start();

  // Now try to call ffmpeg to join assets
  std::cout << "Calling ffmpeg on " + outPath << std::endl;
  std::string prog;
#ifdef AL_WINDOWS
  // Note: path must be DOS style for std::system
  prog = "c:\\Program Files\\ffmpeg\\bin\\ffmpeg";
#else
#if defined(AL_OSX)
  prog = "/usr/local/bin/ffmpeg";
#else
  prog = "ffmpeg";
#endif
#endif

  std::string args;
  args += " -r " + std::to_string(mGraphicsDomain->fps());
  args += " -framerate " + std::to_string(mGraphicsDomain->fps());
  args += " -i " + outPath + "/out%d.png";

#ifdef AL_LIBSNDFILE
  args += " -i " + outPath + "audio.wav -c:a aac -b:a 192k";
#endif
  args += " -pix_fmt yuv420p";
  // for compatibility with outdated media players
  // args += " -crf 20 -preset slower";

  // video compression amount in [0,51] inversely related to quality
  int videoCompress = 20;
  args += " -crf " + std::to_string(videoCompress);

  int videoEncodeSpeed = 2;

  static const std::string speedStrings[] = {
      "placebo", "veryslow", "slower",   "slow",      "medium",
      "fast",    "faster",   "veryfast", "superfast", "ultrafast"};
  args += " -preset " + speedStrings[videoEncodeSpeed];
  args += " " + outPath + "movie.mp4";

  std::string cmd = "\"" + prog + "\"" + args;
  // printf("%s\n", cmd.c_str());

  // TODO: thread this; std::system blocks until the command finishes
  std::system(cmd.c_str());
  std::cout << cmd << std::endl;

  std::cout << "Done calling ffmpeg on " + outPath << std::endl;
}
