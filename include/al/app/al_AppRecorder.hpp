#ifndef AL_APPRECORDER_H
#define AL_APPRECORDER_H

// Author: Andres Cabrera and Lance Putnam

#include "al/app/al_App.hpp"
#include "al/app/al_AudioDomain.hpp"
#include "al/app/al_ComputationDomain.hpp"
#include "al/app/al_OpenGLGraphicsDomain.hpp"
#include "al/app/al_SimulationDomain.hpp"
#include "al/io/al_File.hpp"

/** @defgroup App Tools for recording audio and video from an App
 *
 */

namespace al {

class RecordingDomain : public SynchronousDomain {
public:
  // Domain functions
  //    bool init(ComputationDomain *parent = nullptr) override {return true;}
  bool tick() override {
    mFreeze = true;
    while (mFreeze) {
      al_sleep(0.25);
    }
    return true;
  }
  //  bool cleanup(ComputationDomain *parent = nullptr) override;

  bool mFreeze{true};
};

/**
 * @brief Simple App class
 * @ingroup App
 */
class AppRecorder {
public:
  AppRecorder() {}

  ~AppRecorder() { stopRecording(); }

  void connectApp(App *app) {
    mGraphicsDomain = app->graphicsDomain();
    mAudioDomain = app->audioDomain();
    mSimulationDomain = app->simulationDomain();
    mWindowDomain = app->defaultWindowDomain();
  }

  void startRecordingOffline(double totalTime = 120.0);

  //  void startRecordingRT() {}

  void stopRecording() { mRunning = false; }

private:
  std::shared_ptr<OpenGLGraphicsDomain> mGraphicsDomain;
  std::shared_ptr<GLFWOpenGLWindowDomain> mWindowDomain;
  std::shared_ptr<AudioDomain> mAudioDomain;
  std::shared_ptr<SimulationDomain> mSimulationDomain;

  bool mRunning;
};

} // namespace al

#endif // AL_APPRECORDER_H
