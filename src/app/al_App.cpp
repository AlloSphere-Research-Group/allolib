#include "al/app/al_App.hpp"

using namespace al;

App::App() {
  mOSCDomain = newDomain<OSCDomain>();

  mAudioDomain = newDomain<GammaAudioDomain>();
  mAudioDomain->configure();

  mOpenGLGraphicsDomain = newDomain<OpenGLGraphicsDomain>();
  mSimulationDomain =
      mOpenGLGraphicsDomain->newSubDomain<SimulationDomain>(true);
}

void App::quit() { graphicsDomain()->quit(); }

bool App::shouldQuit() {
  return graphicsDomain()->shouldQuit(); /*|| graphicsDomain()->shouldClose();*/
}

void App::fps(double f) { graphicsDomain()->fps(f); }

Window &App::defaultWindow() {
  if (!mDefaultWindowDomain) {
    std::cerr
        << "ERROR: calling function for Window before window is available!"
        << std::endl;
  }
  return mDefaultWindowDomain->window();
}

Graphics &App::graphics() { return mDefaultWindowDomain->graphics(); }

Viewpoint &App::view() { return mDefaultWindowDomain->view(); }

Nav &App::nav() { return mDefaultWindowDomain->nav(); }

Pose &App::pose() { return mDefaultWindowDomain->nav(); }

NavInputControl &App::navControl() {
  return mDefaultWindowDomain->navControl();
}

Lens &App::lens() { return mDefaultWindowDomain->view().lens(); }

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

void App::cursorHideToggle() { defaultWindow().cursorHideToggle(); }
void App::fullScreenToggle() { defaultWindow().fullScreenToggle(); }

void App::hide() { defaultWindow().hide(); }

void App::iconify() { defaultWindow().iconify(); }

void App::append(WindowEventHandler &handler) {
  defaultWindow().append(handler);
}

void App::prepend(WindowEventHandler &handler) {
  defaultWindow().prepend(handler);
}

void App::remove(WindowEventHandler &handler) {
  defaultWindow().remove(handler);
}

void App::cursor(Window::Cursor v) {
  if (mDefaultWindowDomain) {
    defaultWindow().cursor(v);
  } else {
    mOpenGLGraphicsDomain->nextWindowProperties.cursor = v;
  }
}

void App::cursorHide(bool v) {
  if (mDefaultWindowDomain) {
    defaultWindow().cursorHide(v);
  } else {
    mOpenGLGraphicsDomain->nextWindowProperties.cursorVisible = !v;
  }
}

void App::dimensions(const Window::Dim &v) {
  if (mDefaultWindowDomain) {
    defaultWindow().dimensions(v);
  } else {
    mOpenGLGraphicsDomain->nextWindowProperties.dimensions = v;
  }
}

void App::dimensions(int w, int h) {
  if (mDefaultWindowDomain) {
    defaultWindow().dimensions(w, h);
  } else {
    mOpenGLGraphicsDomain->nextWindowProperties.dimensions = Window::Dim(
        mOpenGLGraphicsDomain->nextWindowProperties.dimensions.t,
        mOpenGLGraphicsDomain->nextWindowProperties.dimensions.l, w, h);
  }
}

void App::dimensions(int x, int y, int w, int h) {
  if (mDefaultWindowDomain) {
    defaultWindow().dimensions(x, y, w, h);
  } else {
    mOpenGLGraphicsDomain->nextWindowProperties.dimensions =
        Window::Dim(x, y, w, h);
  }
}

void App::displayMode(Window::DisplayMode v) {
  if (mDefaultWindowDomain) {
    defaultWindow().displayMode(v);
  } else {
    mOpenGLGraphicsDomain->nextWindowProperties.displayMode = v;
  }
}

void App::fullScreen(bool on) {
  if (mDefaultWindowDomain) {
    defaultWindow().fullScreen(on);
  } else {
    mOpenGLGraphicsDomain->nextWindowProperties.fullScreen = on;
  }
}

void App::title(const std::string &v) {
  if (mDefaultWindowDomain) {
    defaultWindow().title(v);
  } else {
    mOpenGLGraphicsDomain->nextWindowProperties.title = v;
  }
}

void App::vsync(bool v) {
  if (mDefaultWindowDomain) {
    defaultWindow().vsync(v);
  } else {
    mOpenGLGraphicsDomain->nextWindowProperties.vsync = v;
  }
}

void App::decorated(bool b) {
  if (mDefaultWindowDomain) {
    defaultWindow().decorated(b);
  } else {
    mOpenGLGraphicsDomain->nextWindowProperties.decorated = b;
  }
}

AudioIO &App::audioIO() { return audioDomain()->audioIO(); }

void App::configureAudio(double audioRate, int audioBlockSize, int audioOutputs,
                         int audioInputs) {
  audioDomain()->configure(audioRate, audioBlockSize, audioOutputs,
                           audioInputs);
}

void App::configureAudio(AudioDevice &dev, double audioRate, int audioBlockSize,
                         int audioOutputs, int audioInputs) {
  if (audioRate < 0) {
    audioRate = AudioBackend::devicePreferredSamplingRate(dev.id());
  }
  audioDomain()->configure(dev, audioRate, audioBlockSize, audioOutputs,
                           audioInputs);
}

ParameterServer &App::parameterServer() {
  return oscDomain()->parameterServer();
}

void App::start() {
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
  mDefaultWindowDomain->window().onResize = std::bind(
      &App::onResize, this, std::placeholders::_1, std::placeholders::_2);
  mDefaultWindowDomain->window().append(stdControls);
  stdControls.app = this;
  stdControls.mWindow = &mDefaultWindowDomain->window();

  defaultWindow().append(mDefaultWindowDomain->navControl());
  onInit();
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
    if (!domain->init()) {
      std::cerr << "ERROR initializing domain " << std::endl;
    }
  }
}

bool App::StandardWindowAppKeyControls::keyDown(const Keyboard &k) {
  if (k.ctrl()) {
    switch (k.key()) {
    case 'q':
      app->quit();
      return false;
      //          case 'h':
      //            window().hide();
      //            return false;
      //          case 'm':
      //            window().iconify();
      //            return false;
    case 'u':
      window().cursorHideToggle();
      return false;
      //          case 'w':
      //            app->graphicsDomain()->closeWindow(app->mDefaultWindowDomain);
      //            return false;
    default:;
    }
  } else {
    switch (k.key()) {
    case Keyboard::ESCAPE:
      window().fullScreenToggle();
      return false;
    default:;
    }
  }
  return true;
}
