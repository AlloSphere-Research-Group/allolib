#ifndef BASECOMPOSITEAPP_H
#define BASECOMPOSITEAPP_H

#include <stack>
#include <vector>
#include <memory>
#include <iostream>
#include <functional>
#include <cassert>
#include <cstring>

#include "al/app/al_ComputationDomain.hpp"
#include "al/app/al_OpenGLGraphicsDomain.hpp"
#include "al/app/al_AudioDomain.hpp"
#include "al/app/al_OSCDomain.hpp"
#include "al/app/al_SimulationDomain.hpp"
//#include "al_OpenVRDomain.hpp"

#include "al/app/al_WindowApp.hpp"

namespace  al
{

class AppNew {
public:

  AppNew() {
    mOSCDomain = newDomain<OSCDomain>();

    mAudioDomain = newDomain<AudioDomain>();
    mAudioDomain->configure();

    mOpenGLGraphicsDomain = newDomain<OpenGLGraphicsDomain>();
    mSimulationDomain = mOpenGLGraphicsDomain->newSubDomain<SimulationDomain>(true);
  }
  virtual ~AppNew() {}


  template<class DomainType>
  std::shared_ptr<DomainType> newDomain() {
    auto newDomain = std::make_shared<DomainType>();
    mDomainList.push_back(newDomain);
//    if (!newDomain->initialize()) {
//      std::cerr << "ERROR initializing domain " << std::endl;
//    }
    return newDomain;
  }

  Keyboard& keyboard() { return graphicsDomain()->app.mKeyboard; }  ///< Get current keyboard state
  Mouse& mouse() { return graphicsDomain()->app.mMouse; }      ///< Get current mouse state

  double aspect() {return graphicsDomain()->app.aspect(); }        ///< Get aspect ratio (width divided by height)
  bool created() {return graphicsDomain()->app.created(); }        ///< Whether window has been created providing a valid graphics context
  Window::Cursor cursor()  {return graphicsDomain()->app.cursor(); }        ///< Get current cursor type
  bool cursorHide() {return graphicsDomain()->app.cursorHide(); }      ///< Whether the cursor is hidden
  Window::Dim dimensions() {return graphicsDomain()->app.dimensions(); }        ///< Get current dimensions of window
  Window::DisplayMode displayMode() {return graphicsDomain()->app.displayMode(); }  ///< Get current display mode
  bool enabled(Window::DisplayMode v) {return graphicsDomain()->app.enabled(v); }  ///< Get whether display mode flag is set
  bool fullScreen() {return graphicsDomain()->app.fullScreen(); }      ///< Get whether window is in fullscreen
  const std::string& title() {return graphicsDomain()->app.title(); }  ///< Get title of window
  bool visible() {return graphicsDomain()->app.visible(); }        ///< Get whether window is visible
  bool vsync() {return graphicsDomain()->app.vsync(); }          ///< Get whether v-sync is enabled

  int height() {return graphicsDomain()->app.height(); } ///< Get window height, in pixels
  int width() {return graphicsDomain()->app.width(); } ///< Get window width, in pixels

  // get frambuffer size
  // it will be different from window widht and height
  // if the display is a high resolution one (ex: RETINA display)
  int fbHeight() {return graphicsDomain()->app.fbHeight(); }
  int fbWidth() {return graphicsDomain()->app.fbWidth(); }

  float highres_factor() { return graphicsDomain()->app.mHighresFactor; }

  bool decorated() {return graphicsDomain()->app.decorated(); }

  void cursor(Window::Cursor v) { graphicsDomain()->app.cursor(v); }      ///< Set cursor type
  void cursorHide(bool v) { graphicsDomain()->app.cursorHide(v); }      ///< Set cursor hiding
  void cursorHideToggle() { graphicsDomain()->app.cursorHideToggle(); }      ///< Toggle cursor hiding
  void dimensions(const Window::Dim& v) { graphicsDomain()->app.dimensions(v); }  ///< Set dimensions
  void dimensions(int w, int h) { graphicsDomain()->app.dimensions(w, h); }  ///< Set dimensions
  void dimensions(int x, int y, int w, int h) { graphicsDomain()->app.dimensions(x,y,w,h); }  ///< Set dimensions
  void displayMode(Window::DisplayMode v) { graphicsDomain()->app.displayMode(v); }  ///< Set display mode; will recreate window if different from current

  /// This will make the window go fullscreen without borders and,
  /// if posssible, without changing the display resolution.
  void fullScreen(bool on) { graphicsDomain()->app.fullScreen(on); }
  void fullScreenToggle() { graphicsDomain()->app.fullScreenToggle(); } ///< Toggle fullscreen
  void hide() { graphicsDomain()->app.hide(); } ///< Hide window (if showing)
  void iconify() { graphicsDomain()->app.iconify(); } ///< Iconify window
  //void show(); ///< Show window (if hidden)
  void title(const std::string& v) { graphicsDomain()->app.title(v); } ///< Set title
  void vsync(bool v) { graphicsDomain()->app.vsync(v); } ///< Set whether to sync the frame rate to the monitor's refresh rate
  void decorated(bool b) { graphicsDomain()->app.decorated(b); }

  Viewpoint& view() { return graphicsDomain()->mView; }

  Nav& nav() { return graphicsDomain()->mNav; }

  Pose& pose() { return graphicsDomain()->mNav; }

  NavInputControl& navControl() { return graphicsDomain()->mNavControl; }

  Lens& lens() { return graphicsDomain()->mView.lens(); }

  ParameterServer& parameterServer() { return oscDomain()->parameterServer();}

  Graphics& graphics() { return graphicsDomain()->app.mGraphics; }

  void quit() { graphicsDomain()->app.mShouldQuitApp = true; }
  bool shouldQuit() { return graphicsDomain()->app.mShouldQuitApp || graphicsDomain()->app.shouldClose(); }

  void fps(double f) {graphicsDomain()->app.fps(f); }

  virtual void onInit () {}
  virtual void onCreate() {}
  virtual void onAnimate(double dt) {(void) dt;}
  virtual void onDraw(Graphics &g) { (void) g;}
  virtual void onSound(AudioIOData &io) { (void) io;}
  virtual void onMessage(osc::Message &m) { (void) m;}
  virtual void onExit() {}

  virtual void onKeyDown(Keyboard const& k) {}
  virtual void onKeyUp(Keyboard const& k) {}
  virtual void onMouseDown(Mouse const& m) {}
  virtual void onMouseUp(Mouse const& m) {}
  virtual void onMouseDrag(Mouse const& m) {}
  virtual void onMouseMove(Mouse const& m) {}
  virtual void onMouseScroll(Mouse const& m) {}
//  virtual void onResize(int w, int h) {}
//  virtual void onVisibility(bool v) {}

//  // extra functionalities to be handled
//  virtual void preOnCreate();
//  virtual void preOnAnimate(double dt);
//  virtual void preOnDraw();
//  virtual void postOnDraw();
//  virtual void postOnExit();


  void setOpenVRDrawFunction(std::function<void(Graphics &)> func) {
#ifdef AL_EXT_OPENVR
    mOpenVRDomain->setDrawFunction(func);
#else
    std::cout << "Not building OpenVR support. setOpenVRDrawFunction() ignored." << std::endl;
#endif
  }

  void start();

  std::shared_ptr<OSCDomain> oscDomain() {return mOSCDomain;}
  std::shared_ptr<AudioDomain> audioDomain() { return mAudioDomain;}
  std::shared_ptr<OpenGLGraphicsDomain> graphicsDomain() { return mOpenGLGraphicsDomain;}
  std::shared_ptr<SimulationDomain> simulationDomain() { return mSimulationDomain;}

  void enableVR() {

#ifdef AL_EXT_OPENVR
    if (!mOpenVRDomain) {
      std::cout << "Attempting to create OpenVRDomain" << std::endl;
      mOpenVRDomain = graphicsDomain()->newSubDomain<OpenVRDomain>(true);
    }
#else
    std::cout << "OpenVR support not available. Ignoring enableVR()" << std::endl;
#endif
  }

  void disableVR() {

#ifdef AL_EXT_OPENVR
    graphicsDomain()->removeSubDomain(mOpenVRDomain);
    mOpenVRDomain = nullptr;
#else
    std::cout << "OpenVR support not available. Ignoring enableVR()" << std::endl;
#endif
  }

protected:

  void initializeDomains();

  std::shared_ptr<OSCDomain> mOSCDomain;
  std::shared_ptr<AudioDomain> mAudioDomain;
  std::shared_ptr<OpenGLGraphicsDomain> mOpenGLGraphicsDomain;
  std::shared_ptr<SimulationDomain> mSimulationDomain;
//  std::shared_ptr<OpenVRDomain> mOpenVRDomain;


  std::vector<std::shared_ptr<AsynchronousDomain>> mDomainList;
  std::stack<std::shared_ptr<AsynchronousDomain>> mRunningDomains;
};


class DistributedApp : public AppNew {
public:
  DistributedApp(bool primary = true) : AppNew() {

    // State will be same memory for local, but will be synced on the network for separate instances

    mOpenGLGraphicsDomain->removeSubDomain(simulationDomain());

    // Replace Simulation domain with state simulation domain
    mSimulationDomain = mOpenGLGraphicsDomain->newSubDomain<SimulationDomain>(true);

    mPrimary = primary;
    if (mPrimary) {
      std::cout << "Running Primary" << std::endl;

    } else {
      std::cout << "Running REPLICA" << std::endl;

      mSimulationDomain->disableProcessingCallback(); // Replicas won't call onAnimate()
    }
  }


  void setTitle(std::string title) {
    graphicsDomain()->app.title(title);
  }

private:
  bool mPrimary;
};

template <class TSharedState>
class DistributedAppWithState : public DistributedApp {
public:
  DistributedAppWithState(uint16_t rank_) : DistributedApp(), rank(rank_) {

    // State will be same memory for local, but will be synced on the network for separate instances

    mOpenGLGraphicsDomain->removeSubDomain(simulationDomain());

    // Replace Simulation domain with state simulation domain
    mSimulationDomain = mOpenGLGraphicsDomain->newSubDomain<StateSimulationDomain<TSharedState>>(true);

    if (rank == 0) {
      std::cout << "Running primary" << std::endl;
      auto sender = std::static_pointer_cast<StateSimulationDomain<TSharedState>>(mSimulationDomain)->addStateSender("state");
      sender->configure(10101);
    } else {
      std::cout << "Running REPLICA" << std::endl;
      auto receiver = std::static_pointer_cast<StateSimulationDomain<TSharedState>>(mSimulationDomain)->addStateReceiver("state");
      receiver->configure(10101);
      mSimulationDomain->disableProcessingCallback(); // Replicas won't call onAnimate()
    }
  }

  TSharedState &state() {
    return std::static_pointer_cast<StateSimulationDomain<TSharedState>>(mSimulationDomain)->state();
  }

  void setTitle(std::string title) {
    graphicsDomain()->app.title(title);
  }

  uint16_t rank {0};
  uint16_t group {0};
private:
};


} // namespace al

#endif // BASECOMPOSITEAPP_H
