#include "al/core/app/al_Window.hpp"
#include "al/core/gl/al_GLEW.hpp"
#include "al/core/gl/al_GLFW.hpp"

#include <map>
#include <iostream>
#include <cmath>

using namespace std;

namespace al {

class WindowImpl {
public:

  typedef std::map<GLFWwindow*, WindowImpl*> WindowsMap;
  typedef std::map<int, int> KeyMap;

  WindowImpl(Window* w) : mWindow(w), mDimPrev(0) {
    resetState();
    initKeymap(); // set static keymapping map
  }

  ~WindowImpl() {
    destroy();
  }

  void resetState() {
    mGLFWwindow = nullptr;
    mIDGameMode = -1;
    mInGameMode = false;
  }

  GLFWwindow* glfwWindow() { return mGLFWwindow; }
  bool created() const { return mGLFWwindow != nullptr; }

  Window::Dim dimensionsGLFW() const {
    Window::Dim d(0,0,0,0);
    if(created()){
      glfwMakeContextCurrent(mGLFWwindow);
      // TODO
      // d.l = glutGet(GLUT_WINDOW_X);
      // d.t = glutGet(GLUT_WINDOW_Y);
      // d.w = glutGet(GLUT_WINDOW_WIDTH);
      // d.h = glutGet(GLUT_WINDOW_HEIGHT);
    }
    return d;
  }

  void makeMainWindow() {
    glfwMakeContextCurrent(mGLFWwindow);
  }

  void destroy(){
    if(created()){
      windows().erase(mGLFWwindow);
      glfwDestroyWindow(mGLFWwindow);
      resetState();
    }
  }

  // TODO
  void gameMode(bool v) {
    // fullscreen
    if (v) {
      mInGameMode = true;
    }
    else {
      mInGameMode = false;
    }
  }

  static Window * getWindow(){
    WindowImpl * w = getWindowImpl();
    return w ? w->mWindow : 0;
  }

  static WindowImpl * getWindowImpl(){
    return getWindowImpl(glfwGetCurrentContext());
  }

  static WindowImpl * getWindowImpl(GLFWwindow* w){
    WindowsMap::iterator it = windows().find(w);
    if(windows().end() != it){
      return it->second;
    }
    return nullptr;
  }

  // is done in keyboard callback
  // static void setModifiers(Keyboard& k){
    // int mod = glutGetModifiers();
    // k.alt  (mod & GLUT_ACTIVE_ALT);
    // k.ctrl (mod & GLUT_ACTIVE_CTRL);
    // k.shift(mod & GLUT_ACTIVE_SHIFT);
    //printf("a:%d c:%d s:%d\n", k.alt(), k.ctrl(), k.shift());
  // }

  // An invalid key to indicate an error
  static const int INVALID_KEY = -1;

  static int remapKey(int key){
    auto search = keymap().find(key);
    if(search != keymap().end()) return search->second;
    return 0;
  }

  static void cbKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods){
    Window* w = getWindow();
    if (!w) return;

    // first set modifiers
    Keyboard& k = w->mKeyboard;
    k.alt(mods & GLFW_MOD_ALT);
    k.ctrl(mods & GLFW_MOD_CONTROL);
    k.shift(mods & GLFW_MOD_SHIFT);

    switch(action) {
      case GLFW_PRESS:
        k.setKey(remapKey(key), true);
        w->callHandlersKeyDown();
        break;
      case GLFW_REPEAT:
        break;
      case GLFW_RELEASE:
        k.setKey(remapKey(key), false);
        w->callHandlersKeyUp();
        break;
    }
  }

  static void cbMouse(GLFWwindow* window, int button, int action, int mods){
    Window * w = getWindow();
    if(!w) return;

    switch(button){
      case GLFW_MOUSE_BUTTON_LEFT: button = Mouse::LEFT; break;
      case GLFW_MOUSE_BUTTON_RIGHT: button = Mouse::MIDDLE; break;
      case GLFW_MOUSE_BUTTON_MIDDLE: button = Mouse::RIGHT; break;
      default: button = Mouse::EXTRA;    // unrecognized button
    }

    Keyboard& k = w->mKeyboard;
    k.alt(mods & GLFW_MOD_ALT);
    k.ctrl(mods & GLFW_MOD_CONTROL);
    k.shift(mods & GLFW_MOD_SHIFT);

    Mouse& m = w->mMouse;

    if (GLFW_PRESS == action) {
      double xpos, ypos;
      glfwGetCursorPos(window, &xpos, &ypos);
      m.position((int)round(xpos), (int)round(ypos));
      m.button(button, true);
      w->callHandlersMouseDown();
    }
    if (GLFW_RELEASE == action) {
      double xpos, ypos;
      glfwGetCursorPos(window, &xpos, &ypos);
      m.position((int)round(xpos), (int)round(ypos));
      m.button(button, false);
      w->callHandlersMouseUp();
    }
  }

  static void cbMotion(GLFWwindow* window, double mx, double my) {
    Window * win = getWindow();
    if (!win) return;

    win->mMouse.position((int)round(mx), (int)round(my));

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS ||
        glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS ||
        glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_3) == GLFW_PRESS)
    {
      win->callHandlersMouseDrag();
      return;
    }

    win->callHandlersMouseMove();
  }

  static void cbReshape(GLFWwindow* window, int w, int h) {
    Window * win = getWindow();
    if (!win) return;

    Window::Dim& dimCurr = win->mDim;
    if (dimCurr.w != w || dimCurr.h != h) {
      dimCurr.w = w;
      dimCurr.h = h;
      win->callHandlersResize(w, h);
      // needed to get title back after exiting full screen
      // win->title(win->title());
    }
  }

  static void cbVisibility(int v) {
    // Window * win = getWindow();
    // if(win){
    //   // win->mVisible = (v == GLUT_VISIBLE);
    //   win->callHandlersVisibility(win->mVisible);
    // }
  }

  static void initKeymap() {
    static bool initd = false;
    if (initd) return;
    initd = false;

    KeyMap& km = keymap();

    km[GLFW_KEY_SPACE] = ' ';
    km[GLFW_KEY_APOSTROPHE] = '\'';
    km[GLFW_KEY_COMMA] = ',';
    km[GLFW_KEY_MINUS] = '-';
    km[GLFW_KEY_PERIOD] = '.';
    km[GLFW_KEY_SLASH] = '/';
    km[GLFW_KEY_0] = '0';
    km[GLFW_KEY_1] = '1';
    km[GLFW_KEY_2] = '2';
    km[GLFW_KEY_3] = '3';
    km[GLFW_KEY_4] = '4';
    km[GLFW_KEY_5] = '5';
    km[GLFW_KEY_6] = '6';
    km[GLFW_KEY_7] = '7';
    km[GLFW_KEY_8] = '8';
    km[GLFW_KEY_9] = '9';
    km[GLFW_KEY_SEMICOLON] = ';';
    km[GLFW_KEY_EQUAL] = '=';
    km[GLFW_KEY_A] = 'a';
    km[GLFW_KEY_B] = 'b';
    km[GLFW_KEY_C] = 'c';
    km[GLFW_KEY_D] = 'd';
    km[GLFW_KEY_E] = 'e';
    km[GLFW_KEY_F] = 'f';
    km[GLFW_KEY_G] = 'g';
    km[GLFW_KEY_H] = 'h';
    km[GLFW_KEY_I] = 'i';
    km[GLFW_KEY_J] = 'j';
    km[GLFW_KEY_K] = 'k';
    km[GLFW_KEY_L] = 'l';
    km[GLFW_KEY_M] = 'm';
    km[GLFW_KEY_N] = 'n';
    km[GLFW_KEY_O] = 'o';
    km[GLFW_KEY_P] = 'p';
    km[GLFW_KEY_Q] = 'q';
    km[GLFW_KEY_R] = 'r';
    km[GLFW_KEY_S] = 's';
    km[GLFW_KEY_T] = 't';
    km[GLFW_KEY_U] = 'u';
    km[GLFW_KEY_V] = 'v';
    km[GLFW_KEY_W] = 'w';
    km[GLFW_KEY_X] = 'x';
    km[GLFW_KEY_Y] = 'y';
    km[GLFW_KEY_Z] = 'z';
    km[GLFW_KEY_LEFT_BRACKET] = '[';
    km[GLFW_KEY_BACKSLASH] = '\\';
    km[GLFW_KEY_RIGHT_BRACKET] = ']';
    km[GLFW_KEY_GRAVE_ACCENT] = '`';
    km[GLFW_KEY_ESCAPE] = Keyboard::ESCAPE;
    km[GLFW_KEY_ENTER] = Keyboard::ENTER;
    km[GLFW_KEY_TAB] = Keyboard::TAB;
    km[GLFW_KEY_BACKSPACE] = Keyboard::BACKSPACE;
    km[GLFW_KEY_INSERT] = Keyboard::INSERT;
    km[GLFW_KEY_DELETE] = Keyboard::DELETE;
    km[GLFW_KEY_RIGHT] = Keyboard::RIGHT;
    km[GLFW_KEY_LEFT] = Keyboard::LEFT;
    km[GLFW_KEY_DOWN] = Keyboard::DOWN;
    km[GLFW_KEY_UP] = Keyboard::UP;
    km[GLFW_KEY_PAGE_UP] = Keyboard::PAGE_UP;
    km[GLFW_KEY_PAGE_DOWN] = Keyboard::PAGE_DOWN;
    km[GLFW_KEY_HOME] = Keyboard::HOME;
    km[GLFW_KEY_END] = Keyboard::END;
    km[GLFW_KEY_F1] = Keyboard::F1;
    km[GLFW_KEY_F2] = Keyboard::F2;
    km[GLFW_KEY_F3] = Keyboard::F3;
    km[GLFW_KEY_F4] = Keyboard::F4;
    km[GLFW_KEY_F5] = Keyboard::F5;
    km[GLFW_KEY_F6] = Keyboard::F6;
    km[GLFW_KEY_F7] = Keyboard::F7;
    km[GLFW_KEY_F8] = Keyboard::F8;
    km[GLFW_KEY_F9] = Keyboard::F9;
    km[GLFW_KEY_F10] = Keyboard::F10;
    km[GLFW_KEY_F11] = Keyboard::F11;
    km[GLFW_KEY_F12] = Keyboard::F12;
  }

  void registerCBs(){ 
    glfwSetKeyCallback(mGLFWwindow, cbKeyboard);
    glfwSetMouseButtonCallback(mGLFWwindow, cbMouse);
    glfwSetWindowSizeCallback(mGLFWwindow, cbReshape);
    glfwSetCursorPosCallback(mGLFWwindow, cbMotion);
    // glfwSetWindowPosCallback(window, cb_windowpos);
    // glfwSetFramebufferSizeCallback(window, cb_framebuffersize);
    // glfwSetWindowCloseCallback(window, cb_windowclose);
    // glfwSetWindowRefreshCallback(window, cb_windowrefresh);
    // glfwSetWindowFocusCallback(window, cb_windowfocus);
    // glfwSetErrorCallback(errorCallback);
  }

  void refresh() {
    // [!] POLLEVENTS IS AFTER SWAPBUFFERS
    // why: if an event destroys window
    //      swapbuffers will throw error
    //      since mGLFWwindow becomes nullptr
    glfwSwapBuffers(mGLFWwindow);
  	glfwPollEvents();
  }

private:
  // friend class Main;

  static const char * errorString(bool verbose){
  	// TODO
    return "";
  }

  static WindowsMap& windows(){
    static WindowsMap* v = new WindowsMap;
    return *v;
  }

  static KeyMap& keymap() {
    static KeyMap* k = new KeyMap;
    return *k;
  }

  Window * mWindow;
  GLFWwindow* mGLFWwindow;
  int mIDGameMode;
  Window::Dim mDimPrev;

  bool mInGameMode;

  friend class Window;
};

// ---------------------------------------------------------

void Window::implCtor(){
  mImpl = new WindowImpl(this);
}

void Window::implDtor(){
  delete mImpl;
}

void Window::implDestroy(){
  mImpl->destroy();
}

bool Window::implCreate(){
  int w = mDim.w;
  int h = mDim.h;
  // mDim.w = 0;
  // mDim.h = 0;
  // mImpl->mDimPrev.set(0,0,0,0);

  glfw::init();

  glfwDefaultWindowHints();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // TODO
  // bits: DOUBLE_BUF? STENCIL_BUF? etc. ...

  mImpl->mGLFWwindow = glfwCreateWindow(w, h, mTitle.c_str(), NULL, NULL);
  if (!mImpl->created()) {
    return false;
  }

  glfwMakeContextCurrent(mImpl->mGLFWwindow);

  const GLubyte* renderer = glGetString(GL_RENDERER);
  std::cout << "renderer: " << renderer << std::endl;
  int mj = glfwGetWindowAttrib(mImpl->mGLFWwindow, GLFW_CONTEXT_VERSION_MAJOR);
  int mn = glfwGetWindowAttrib(mImpl->mGLFWwindow, GLFW_CONTEXT_VERSION_MINOR);
  std::cout << "opengl window version: " << mj << "." << mn << std::endl;
  char* glsl_version = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
  std::cout << "glsl version: " << glsl_version << std::endl;

  int fbw, fbh;
  glfwGetFramebufferSize(mImpl->mGLFWwindow, &fbw, &fbh);
  HIGHRES_FACTOR_W = fbw / float(w);
  HIGHRES_FACTOR_H = fbh / float(h);
  std::cout << "highres factor: " << HIGHRES_FACTOR_W << ", " << HIGHRES_FACTOR_H << std::endl;

  mImpl->registerCBs();
  WindowImpl::windows()[mImpl->mGLFWwindow] = mImpl;

  glew::init();

  vsync(mVSync); // glfwSwapInterval(1);
  
  return true;
}

void Window::refresh() {
  mImpl->refresh();
}

void Window::destroyAll(){
  //printf("Window::destroyAll\n");
  // WindowImpl::WindowsMap::iterator it = WindowImpl::windows().begin();
  auto it = WindowImpl::windows().begin();
  while (it != WindowImpl::windows().end()) {
    if (it->second && it->second->mWindow) {
      (it++)->second->mWindow->destroy();
    }
    else {
      ++it;
    }
  }
}

bool Window::created() const {
  return mImpl->created();
}

void Window::implSetDimensions() {
  // TODO!!
  mImpl->makeMainWindow();
  // glutPositionWindow(mDim.l, mDim.t);

  // Set mDim extent to actual window extent so reshape callback triggers
  // handlers.
  int w = mDim.w;
  int h = mDim.h;
  // mDim.w = glutGet(GLUT_WINDOW_WIDTH);
  // mDim.h = glutGet(GLUT_WINDOW_HEIGHT);
  // glutReshapeWindow(w, h);
}

void Window::implSetCursor() {
  // TODO!
  if(!mCursorHide){
    mImpl->makeMainWindow();
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    // switch(mCursor){
      // case CROSSHAIR:  glutSetCursor(GLUT_CURSOR_CROSSHAIR); break;
      // case POINTER:  glutSetCursor(GLUT_CURSOR_INHERIT); break;
      // default:;
    // }
  }
}

void Window::implSetCursorHide() {
  // TODO!
  mImpl->makeMainWindow();
  // if(mCursorHide)  glutSetCursor(GLUT_CURSOR_NONE);
  // else      cursor(mCursor);
}

void Window::implSetFullScreen() {
  // TODO!
  // Note that on Linux, we must use GLUT's "game mode" to get a borderless
  // fullscreen window.

  // enter full screen
  // if(mFullScreen){
  //   #ifdef AL_LINUX
  //     callHandlersOnDestroy();
  //     // GLUT automatically calls reshape CB upon entering game mode...
  //     mImpl->gameMode(true);
  //     callHandlersOnCreate();
  //     cursorHide(cursorHide());
  //   #else
  //     glutSetWindow(mImpl->mID);
  //     mImpl->mDimPrev = mImpl->dimensionsGLUT();
  //     glutFullScreen(); // calls glutReshapeWindow
  //   #endif
  // }

  // // exit full screen
  // else {
  //   #ifdef AL_LINUX
  //     callHandlersOnDestroy();
  //     mImpl->gameMode(false);
  //     mDim = mImpl->dimensionsGLUT();
  //     callHandlersOnCreate();
  //     callHandlersOnResize(mDim.w, mDim.h);
  //     hide(); show(); // need to force focus to get key callbacks to work
  //   #else
  //     // Calls glutReshapeWindow which exits from a glutFullScreen call.
  //     // This also calls our reshape callback which sets the mDim member.
  //     dimensions(mImpl->mDimPrev);
  //   #endif
  // }
}

void Window::implSetTitle() {
  // TODO!
  mImpl->makeMainWindow();
  // glutSetWindowTitle(mTitle.c_str());
  //printf("Window::title(%s)\n", mTitle.c_str());
}

// See: https://www.opengl.org/wiki/Swap_Interval
void Window::implSetVSync() {
  // TODO!
  mImpl->makeMainWindow();
  // #if defined AL_OSX
  //   GLint VBL = GLint(mVSync);
  //   CGLContextObj ctx = CGLGetCurrentContext();
  //   CGLSetParameter(ctx, kCGLCPSwapInterval, &VBL);
  // #elif defined AL_LINUX
  // #elif defined AL_WINDOWS
  // #endif
}

Window& Window::hide(){
  // TODO!
  mImpl->makeMainWindow();
  // glutHideWindow();
  return *this;
}

Window& Window::iconify() {
  // TODO!
  mImpl->makeMainWindow();
  // glutIconifyWindow();
  return *this;
}

Window& Window::show() {
  // TODO!
  mImpl->makeMainWindow();
  // glutShowWindow();
  return *this;
}

}