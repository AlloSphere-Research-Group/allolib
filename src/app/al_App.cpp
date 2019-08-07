#include "al/app/al_App.hpp"

using namespace al;

App::App() {
  mOSCDomain = newDomain<OSCDomain>();

  mAudioDomain = newDomain<GammaAudioDomain>();
  mAudioDomain->configure();

  mOpenGLGraphicsDomain = newDomain<OpenGLGraphicsDomain>();
  mSimulationDomain =
      mOpenGLGraphicsDomain->newSubDomain<SimulationDomain>(true);

  initializeDomains();
  mDefaultWindowDomain = graphicsDomain()->newWindow();

  mDefaultWindowDomain->onDraw =
      std::bind(&App::onDraw, this, std::placeholders::_1);
  mDefaultWindowDomain->window().onKeyDown =
      std::bind(&App::onKeyDown, this, std::placeholders::_1);
  mDefaultWindowDomain->window().onKeyUp =
      std::bind(&App::onKeyUp, this, std::placeholders::_1);
  mDefaultWindowDomain->window().onMouseDown =
      std::bind(&App::onMouseDown, this, std::placeholders::_1);
  mDefaultWindowDomain->window().onMouseUp =
      std::bind(&App::onMouseUp, this, std::placeholders::_1);
  mDefaultWindowDomain->window().onMouseDrag =
      std::bind(&App::onMouseDrag, this, std::placeholders::_1);
  mDefaultWindowDomain->window().onMouseMove =
      std::bind(&App::onMouseMove, this, std::placeholders::_1);
  mDefaultWindowDomain->window().onMouseScroll =
      std::bind(&App::onMouseScroll, this, std::placeholders::_1);
  mDefaultWindowDomain->window().append(stdControls);
  stdControls.app = this;
  stdControls.mWindow = &mDefaultWindowDomain->window();

  append(mDefaultWindowDomain->navControl());
}

Window &App::defaultWindow() { return mDefaultWindowDomain->window(); }

Graphics &App::graphics() { return mDefaultWindowDomain->graphics(); }

Keyboard &App::keyboard() { return defaultWindow().mKeyboard; }

Mouse &App::mouse() { return defaultWindow().mMouse; }

double App::aspect() { return defaultWindow().aspect(); }

bool App::created() { return defaultWindow().created(); }

Window::Cursor App::cursor() { return defaultWindow().cursor(); }

bool App::cursorHide() { return defaultWindow().cursorHide(); }

Window::Dim App::dimensions() { return defaultWindow().dimensions(); }

Window::DisplayMode App::displayMode() { return defaultWindow().displayMode(); }

bool App::enabled(Window::DisplayMode v) { return defaultWindow().enabled(v); }

bool App::fullScreen() { return defaultWindow().fullScreen(); }

const std::string &App::title() { return defaultWindow().title(); }

bool App::visible() { return defaultWindow().visible(); }

bool App::vsync() { return defaultWindow().vsync(); }

int App::height() { return defaultWindow().height(); }

int App::width() { return defaultWindow().width(); }

int App::fbHeight() { return defaultWindow().fbHeight(); }

int App::fbWidth() { return defaultWindow().fbWidth(); }

float App::highresFactor() { return defaultWindow().mHighresFactor; }

bool App::decorated() { return defaultWindow().decorated(); }

void App::cursor(Window::Cursor v) { defaultWindow().cursor(v); }

void App::cursorHide(bool v) { defaultWindow().cursorHide(v); }

void App::cursorHideToggle() { defaultWindow().cursorHideToggle(); }

void App::dimensions(const Window::Dim &v) { defaultWindow().dimensions(v); }

void App::dimensions(int w, int h) { defaultWindow().dimensions(w, h); }

void App::dimensions(int x, int y, int w, int h) {
  defaultWindow().dimensions(x, y, w, h);
}

void App::displayMode(Window::DisplayMode v) { defaultWindow().displayMode(v); }

void App::fullScreen(bool on) { defaultWindow().fullScreen(on); }

void App::fullScreenToggle() { defaultWindow().fullScreenToggle(); }

void App::hide() { defaultWindow().hide(); }

void App::iconify() { defaultWindow().iconify(); }

void App::title(const std::string &v) { defaultWindow().title(v); }

void App::vsync(bool v) { defaultWindow().vsync(v); }

void App::decorated(bool b) { defaultWindow().decorated(b); }

Viewpoint &App::view() { return mDefaultWindowDomain->view(); }

Nav &App::nav() { return mDefaultWindowDomain->nav(); }

Pose &App::pose() { return mDefaultWindowDomain->nav(); }

NavInputControl &App::navControl() {
  return mDefaultWindowDomain->navControl();
}

Lens &App::lens() { return mDefaultWindowDomain->view().lens(); }

void App::quit() { graphicsDomain()->quit(); }

bool App::shouldQuit() {
  return graphicsDomain()->shouldQuit(); /*|| graphicsDomain()->shouldClose();*/
}

void App::fps(double f) { graphicsDomain()->fps(f); }

void App::append(WindowEventHandler &handler) {
  defaultWindow().append(handler);
}

void App::prepend(WindowEventHandler &handler) {
  defaultWindow().prepend(handler);
}

void App::remove(WindowEventHandler &handler) {
  defaultWindow().remove(handler);
}

AudioIO &App::audioIO() { return audioDomain()->audioIO(); }

void App::configureAudio(double audioRate, int audioBlockSize, int audioOutputs,
                         int audioInputs) {
  audioDomain()->configure(audioRate, audioBlockSize, audioOutputs,
                           audioInputs);
}

void App::configureAudio(AudioDevice &dev, double audioRate, int audioBlockSize,
                         int audioOutputs, int audioInputs) {
  audioDomain()->configure(dev, audioRate, audioBlockSize, audioOutputs,
                           audioInputs);
}

ParameterServer &App::parameterServer() {
  return oscDomain()->parameterServer();
}

void App::start() {
  onInit();  // onInit() can't be called in constructor as it is virtual. But it
             // is good enough here.
  for (auto &domain : mDomainList) {
    mRunningDomains.push(domain);
    if (!domain->start()) {
      std::cerr << "ERROR starting domain " << std::endl;
      break;
    }
  }

  while (mRunningDomains.size() > 0) {
    if (!mRunningDomains.top()->stop()) {
      std::cerr << "ERROR stopping domain " << std::endl;
    }
    mRunningDomains.pop();
  }

  onExit();
  mDefaultWindowDomain = nullptr;
  for (auto &domain : mDomainList) {
    if (!domain->cleanup()) {
      std::cerr << "ERROR cleaning up domain " << std::endl;
    }
  }
}

void App::initializeDomains() {
  for (auto domain : mDomainList) {
    auto domainPtr = domain.get();
    if (strcmp(typeid(*domainPtr).name(),
               typeid(OpenGLGraphicsDomain).name()) == 0) {
      dynamic_cast<OpenGLGraphicsDomain *>(domainPtr)->onCreate =
          std::bind(&App::onCreate, this);

      mSimulationDomain->simulationFunction =
          std::bind(&App::onAnimate, this, std::placeholders::_1);
    } else if (strcmp(typeid(*domainPtr).name(),
                      typeid(GammaAudioDomain).name()) == 0) {
      dynamic_cast<GammaAudioDomain *>(domainPtr)->onSound =
          std::bind(&App::onSound, this, std::placeholders::_1);
    } else if (strcmp(typeid(*domainPtr).name(), typeid(AudioDomain).name()) ==
               0) {
      dynamic_cast<AudioDomain *>(domainPtr)->onSound =
          std::bind(&App::onSound, this, std::placeholders::_1);
    } else if (strcmp(typeid(*domainPtr).name(), typeid(OSCDomain).name()) ==
               0) {
      dynamic_cast<OSCDomain *>(domainPtr)->onMessage =
          std::bind(&App::onMessage, this, std::placeholders::_1);
    } else {
      std::cout << "WARNING: Domain unknown for auto connection" << std::endl;
    }
    if (!domain->initialize()) {
      std::cerr << "ERROR initializing domain " << std::endl;
    }
  }
}
