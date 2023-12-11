
#ifndef AL_DYNAMICSCENE_HPP
#define AL_DYNAMICSCENE_HPP

/*	Allocore --
        Multimedia / virtual environment application class library

   Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology,
   UCSB. Copyright (C) 2012-2022. The Regents of the University of California.
        All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

   Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the
   distribution.

   Neither the name of the University of California nor the names
   of its contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

        File description:
        Dynamic Scene Manager

        File author(s):
        Andrés Cabrera mantaraya36@gmail.com
*/

#include <condition_variable>
#include <memory>
#include <queue>
#include <thread>

#include "al/math/al_Vec.hpp"
#include "al/scene/al_SynthSequencer.hpp"
#include "al/sound/al_Speaker.hpp"
#include "al/sound/al_StereoPanner.hpp"
#include "al/spatial/al_DistAtten.hpp"
#include "al/spatial/al_Pose.hpp"

#include "al/scene/al_PositionedVoice.hpp"

namespace al {

struct UpdateThreadFuncData {
  SynthVoice *voice;
  double dt;
};

// thread pool from
// https://stackoverflow.com/questions/23896421/efficiently-waiting-for-all-tasks-in-a-threadpool-to-finish
class ThreadPool {
public:
  ThreadPool(unsigned int n = std::thread::hardware_concurrency());

  template <class F> void enqueue(F &&f, UpdateThreadFuncData &data);
  void waitForProcessingDone();
  ~ThreadPool();

  size_t size() { return workers.size(); }

  void stopThreads();

private:
  std::vector<std::thread> workers;
  std::deque<std::pair<std::function<void(UpdateThreadFuncData)>,
                       UpdateThreadFuncData>>
      tasks;
  std::mutex queue_mutex;
  std::condition_variable cv_task;
  std::condition_variable cv_finished;
  unsigned int busy;
  bool stop{false};

  void thread_proc();
};

template <class F> void ThreadPool::enqueue(F &&f, UpdateThreadFuncData &data) {
  std::unique_lock<std::mutex> lock(queue_mutex);
  tasks.emplace_back(std::pair<std::function<void(UpdateThreadFuncData)>,
                               UpdateThreadFuncData>(std::forward<F>(f), data));
  cv_task.notify_one();
}

/**
 * @brief The DynamicScene class
 * @ingroup Scene
 *
 * A voice manager that includes positional data for graphics and audio
 * rendering.
 */
class DynamicScene : public PolySynth {
public:
  DynamicScene(int threadPoolSize = 0,
               TimeMasterMode masterMode = TimeMasterMode::TIME_MASTER_AUDIO);

  virtual ~DynamicScene();

  /**
   * @brief Set a Spatializar to use.
   * @param sl The Speaker layout to use
   * @return the allocated spatializer of the requested TSpatilizer type
   *
   *  If not called, the default is stereo panning over two speakers.
   */
  template <class TSpatializer>
  std::shared_ptr<TSpatializer> setSpatializer(const Speakers &sl) {
    mSpatializer = std::make_shared<TSpatializer>(sl);
    mSpatializer->compile();
    return std::static_pointer_cast<TSpatializer>(mSpatializer);
  }

  template <class TSpatializer>
  std::shared_ptr<TSpatializer> setSpatializer(const Speakers &&sl) {
    mSpatializer = std::make_shared<TSpatializer>(sl);
    mSpatializer->compile();
    return std::static_pointer_cast<TSpatializer>(mSpatializer);
  }

  /**
   * @brief Prepares internals for run.
   * @param io The audio device AudioIO/AudioIOData object
   *
   * This function must be called before any render() calls as it allocates
   * and resizes the internal AudioIOData objects that are passed to the
   * voices in the chain. You should call setVoiceMaxOutputChannels() or
   * setVoiceBusChannels() to set specific values if your voices use more
   * than one output channel or you need to use buses.
   */
  void prepare(AudioIOData &io);

  virtual void cleanup() {}

  /**
   * @brief The listener pose is used to determine both the graphic view and the
   * audio spatialization
   * @return
   */
  Pose &listenerPose();
  void listenerPose(Pose &pose);

  /**
   * @brief Render graphics context
   * @param g
   */
  virtual void render(Graphics &g) final;

  /**
   * @brief Render audio context
   * @param io
   */
  virtual void render(AudioIOData &io) final;

  /**
   * @brief This function runs the simulation/update of internal states for each
   * voice
   * @param dt The elapsed time since the previous call to this function
   *
   */
  virtual void update(double dt = 0) final;

  /**
   * @brief Set update context to use threading
   * @param threaded
   */
  void setUpdateThreaded(bool threaded);

  /**
   * @brief Set audio context to use thread pool to render voices
   * @param threaded
   */
  void setAudioThreaded(bool threaded);

  /**
   * @brief Get distance attenuation object for query or modification
   * @return
   */
  DistAtten<> &distanceAttenuation();

  void print(std::ostream &stream = std::cout);

  /**
   * @brief Show a visual marker in the origin
   * @param show
   */
  void showWorldMarker(bool show = true) { mDrawWorldMarker = show; }

  /**
   * @brief Enables/disables sorting by distance to listener on graphics render
   */
  void sortDrawingByDistance(bool sort = true);

  /**
   * @brief Stop all audio threads. No processing is possible after calling this
   * function
   *
   * You might need to close all threads by calling this function to have
   * applications close neatly. Will only have effect if threading is enabled
   * for simulation or audio
   */
  void stopAudioThreads();

protected:
private:
  // A speaker layout and spatializer
  std::shared_ptr<Spatializer> mSpatializer;

  Pose mListenerPose;
  DistAtten<> mDistAtten;

  bool mSortDrawingByDistance{false};
  // For threaded simulation
  std::unique_ptr<ThreadPool> mWorkerThreads; // Update worker threads
  bool mThreadedUpdate{true};

  // For threaded audio
  bool mThreadedAudio{false};
  std::vector<std::thread> mAudioThreads;
  std::vector<AudioIOData> mThreadedAudioData;
  std::map<int, std::vector<int>>
      mThreadMap; // Defines which threads run which voices. Key is thread id,
                  // value is voice ids.
  std::condition_variable mThreadTrigger;
  std::condition_variable mAudioThreadDone;
  std::mutex mSpatializerLock;
  AudioIOData
      *externalAudioIO; // This is captured by the audio callback and passed to
                        // the audio threads. Protected by mSpatializerLock
  std::mutex mThreadTriggerLock;
  bool mSynthRunning{true};
  unsigned int mAudioBusy = 0;

  static void updateThreadFunc(UpdateThreadFuncData data);

  static void audioThreadFunc(DynamicScene *scene, int id);

  // World marker
  bool mDrawWorldMarker{false};
  Mesh mWorldMarker;
};

} // namespace al

#endif // AL_DYNAMICSCENE_HPP
