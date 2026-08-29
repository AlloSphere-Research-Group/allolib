#ifndef INCLUDE_AL_SEQUENCESERVER
#define INCLUDE_AL_SEQUENCESERVER

#include <string>

#include "al/ui/al_ParameterServer.hpp"
#include "al/ui/al_PresetSequencer.hpp"
#include "al/ui/al_SequenceRecorder.hpp"

namespace al {

/// SequenceServer
/// @ingroup UI
class SequenceServer : public osc::PacketHandler, public OSCNotifier {
 public:
  /**
   * @brief SequenceServer constructor
   *
   * @param oscAddress The network address on which to listen to. If empty use
   * all available network interfaces. Defaults to "127.0.0.1".
   * @param oscPort The network port on which to listen. Defaults to 9012.
   *
   * The sequencer server triggers sequences when it receives a valid sequence
   * name on OSC path /sequence.
   */

  SequenceServer(std::string oscAddress = "127.0.0.1", int oscPort = 9012);
  /**
   * @brief using this constructor reuses the existing osc::Recv server from the
   * ParameterServer object
   * @param paramServer an existing ParameterServer object
   *
   * You will want to reuse an osc::Recv server when you want to expose the
   * interface thorugh the same network port. Since network ports are exclusive,
   * once a port is bound, it can't be used. You might need to expose the
   * parameters on the same network port when using things like
   * interface.simpleserver.js That must connect all interfaces to the same
   * network port.
   */
  SequenceServer(ParameterServer &paramServer);
  ~SequenceServer();

  virtual void onMessage(osc::Message &m);

  // Special cases of objects that are handled in specific ways
  SequenceServer &registerSequencer(PresetSequencer &sequencer);
  SequenceServer &registerRecorder(SequenceRecorder &recorder);
  SequenceServer &registerMessageConsumer(osc::MessageConsumer &consumer);

  /**
   * @brief print prints information about the server to std::out
   */
  void print();

  /**
   * @brief stopServer stops the OSC server thread. Calling this function
   * is sometimes required when this object is destroyed abruptly and the
   * destructor is not called.
   */
  void stopServer();

  SequenceServer &operator<<(PresetSequencer &sequencer) {
    return registerSequencer(sequencer);
  }
  SequenceServer &operator<<(SequenceRecorder &recorder) {
    return registerRecorder(recorder);
  }
  SequenceServer &operator<<(osc::MessageConsumer &consumer) {
    return registerMessageConsumer(consumer);
  }

  void setAddress(std::string address);
  std::string getAddress();

 protected:
  //	void attachPacketHandler(osc::PacketHandler *handler);
  static void changeCallback(int value, void *sender, void *userData);

 private:
  osc::Recv *mServer;
  PresetSequencer *mSequencer;
  SequenceRecorder *mRecorder;
  // ParameterServer *mParamServer;
  std::vector<Composition *> mCompositions;
  //	std::mutex mServerLock;
  std::string mOSCpath;
  std::string mOSCQueryPath;
  //	std::mutex mHandlerLock;
  //	std::vector<osc::PacketHandler *> mHandlers;
  std::vector<osc::MessageConsumer *> mConsumers;
};

}  // namespace al

#endif  // INCLUDE_AL_SEQUENCESERVER
