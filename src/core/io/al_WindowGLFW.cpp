#include "al/core/io/al_Window.hpp"
#include "al/core/graphics/al_GLEW.hpp"
#include "al/core/graphics/al_GLFW.hpp"

#include <map>
#include <unordered_map>
#include <iostream>
#include <cmath>

using namespace std;

namespace al {

class WindowImpl {
public:
  typedef std::map<GLFWwindow*, WindowImpl*> WindowsMap;
  typedef std::unordered_map<int, int> KeyMap;

  WindowImpl(Window* w) : mWindow(w) {

  }

  ~WindowImpl() {
    destroy();
  }

  GLFWwindow* glfwWindow() {
    return mGLFWwindow;
  }

  bool created() const {
    return mGLFWwindow != nullptr;
  }

  void makeCurrent() {
    if (mGLFWwindow != mCurrentGLFWwindow) {
      glfwMakeContextCurrent(mGLFWwindow);
      mCurrentGLFWwindow = glfwGetCurrentContext();
    }
  }

  void destroy(){
    if(created()){
      windows().erase(mGLFWwindow);
      glfwDestroyWindow(mGLFWwindow);
      mGLFWwindow = nullptr;
    }
  }

  static WindowsMap& windows(){
    static WindowsMap* v = new WindowsMap;
    return *v;
  }

  static Window* getWindow(GLFWwindow* w) {
    WindowImpl* impl = getWindowImpl(w);
    return impl? impl->mWindow : nullptr;
  }

  static WindowImpl * getWindowImpl(GLFWwindow* w){
    WindowsMap::iterator it = windows().find(w);
    if(windows().end() != it){
      return it->second;
    }
    return nullptr;
  }

  static void cbKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods){
    auto* w = getWindow(window);
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
    auto* w = getWindow(window);
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
    auto* w = getWindow(window);
    if (!w) return;

    w->mMouse.position((int)round(mx), (int)round(my));

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS ||
        glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS ||
        glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_3) == GLFW_PRESS)
    {
      w->callHandlersMouseDrag();
      return;
    }

    w->callHandlersMouseMove();
  }

  static void cbReshape(GLFWwindow* window, int w, int h) {
    auto* win = getWindow(window);
    if (!win) return;
    Window::Dim& dimCurr = win->mFullScreen? win->mFullScreenDim : win->mDim;
    dimCurr.w = w;
    dimCurr.h = h;

    // update framebuffer size
    glfwGetFramebufferSize(window, &(win->mFramebufferWidth), &(win->mFramebufferHeight));
    win->mHighresFactor = win->mFramebufferWidth / float(w);

    win->callHandlersResize(w, h);
  }

  //static void window_close_callback(GLFWwindow* window) {
  //  glfwSetWindowShouldClose(window, GLFW_FALSE);
  //}

  void registerCBs(){ 
    glfwSetKeyCallback(mGLFWwindow, cbKeyboard);
    glfwSetMouseButtonCallback(mGLFWwindow, cbMouse);
    glfwSetWindowSizeCallback(mGLFWwindow, cbReshape);
    glfwSetCursorPosCallback(mGLFWwindow, cbMotion);
    // glfwSetWindowPosCallback(window, cb_windowpos);
    // glfwSetFramebufferSizeCallback(window, cb_framebuffersize);
    //glfwSetWindowCloseCallback(mGLFWwindow, window_close_callback);
    // glfwSetWindowRefreshCallback(window, cb_windowrefresh);
    // glfwSetWindowFocusCallback(window, cb_windowfocus);
    // glfwSetErrorCallback(errorCallback);
  }

  static KeyMap& keymap() {
    static KeyMap k = make_glfw_keymap();
    return k;
  }

  static KeyMap make_glfw_keymap();

  static int remapKey(int key){
    auto search = keymap().find(key);
    if(search != keymap().end()) return search->second;
    return 0;
  }

private:
  friend class Window;
  Window * mWindow = nullptr;
  GLFWwindow* mGLFWwindow = nullptr;
  static GLFWwindow* mCurrentGLFWwindow;
};

GLFWwindow* WindowImpl::mCurrentGLFWwindow = nullptr;

// ---------------------------------------------------------

Window::Window() {
  mImpl = make_unique<WindowImpl>(this);
}

Window::~Window() {

}

bool Window::implCreate() {
  glfw::init();
  glfwDefaultWindowHints();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_DECORATED, mDecorated ? true : false);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true); // if OSX, this is a must
  

  // TODO
  // bits: STENCIL_BUF etc. ...

  mImpl->mGLFWwindow = glfwCreateWindow(mDim.w, mDim.h, mTitle.c_str(), NULL, NULL);
  if (!mImpl->created()) {
    return false;
  }

  mImpl->makeCurrent();
  glfwSetWindowPos(mImpl->mGLFWwindow, mDim.l, mDim.t);

  // sometimes OS makes window's size different from what we requested
  int actual_width, actual_height, actual_left, actual_top;
  glfwGetWindowSize(mImpl->mGLFWwindow, &actual_width, &actual_height);
  glfwGetWindowPos(mImpl->mGLFWwindow, &actual_left, &actual_top);
  if (actual_width != mDim.w ||
      actual_height != mDim.h ||
      actual_left != mDim.l ||
      actual_top != mDim.t
  ) {
    cout << "screen dimension different from requested" << endl;
    cout << actual_width << " X " << actual_height << " at " << actual_left << ", " << actual_top << endl;
    mDim.w = actual_width;
    mDim.h = actual_height;
    mDim.l = actual_left;
    mDim.t = actual_top;
  }
  
  glew::init();

  const GLubyte* renderer = glGetString(GL_RENDERER);
  std::cout << "renderer: " << renderer << std::endl;
  int mj = glfwGetWindowAttrib(mImpl->mGLFWwindow, GLFW_CONTEXT_VERSION_MAJOR);
  int mn = glfwGetWindowAttrib(mImpl->mGLFWwindow, GLFW_CONTEXT_VERSION_MINOR);
  std::cout << "opengl window version: " << mj << "." << mn << std::endl;
  char* glsl_version = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
  std::cout << "glsl version: " << glsl_version << std::endl;

  // int fbw, fbh;
  glfwGetFramebufferSize(mImpl->mGLFWwindow, &mFramebufferWidth, &mFramebufferHeight);
  mHighresFactor = mFramebufferWidth / float(mDim.w);
  cout << "framebuffer size: " << mFramebufferWidth << ", " << mFramebufferHeight << endl;
  cout << "highres factor: " << mHighresFactor << endl;

  mImpl->registerCBs();
  vsync(mVSync);
  WindowImpl::windows()[mImpl->mGLFWwindow] = mImpl.get();
  return true;
}

bool Window::implCreated() const {
  return mImpl->created();
}

void Window::implRefresh() {
  mImpl->makeCurrent();
  // [!] POLLEVENTS IS AFTER SWAPBUFFERS
  // why: if an event destroys window
  //      swapbuffers will throw error
  //      since mGLFWwindow becomes nullptr
  glfwSwapBuffers(mImpl->mGLFWwindow);
  glfwPollEvents();
}

void Window::implDestroy() {
  mImpl->destroy();
}

void Window::implClose () {
	glfwSetWindowShouldClose(mImpl->mGLFWwindow, true);
}

bool Window::implShouldClose() {
    if (glfwWindowShouldClose(mImpl->mGLFWwindow)) {
        return true;
    }
    else {
        return false;
    }
}

void Window::implSetCursor() {
  
}

void Window::implSetCursorHide() {
  glfwSetInputMode(mImpl->mGLFWwindow, GLFW_CURSOR, mCursorHide? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
}

void Window::implSetDimensions() {
  mImpl->makeCurrent();
  if (mFullScreen) {
    fullScreen(false);
  }
  else {
    glfwSetWindowSize(mImpl->mGLFWwindow, mDim.w, mDim.h);
    glfwSetWindowPos(mImpl->mGLFWwindow, mDim.l, mDim.t);
  }
}

void Window::implSetFullScreen() {
  mImpl->makeCurrent();

// glfwSetWindowMonitor came in at glfw 3.2
#if GLFW_VERSION_MINOR > 1
  if (mFullScreen) {
    // TODO: selection for multi-monitor
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primary);
    glfwSetWindowMonitor(
      mImpl->mGLFWwindow, primary,
      0, 0, mode->width, mode->height,
      mode->refreshRate
    );
    glfwGetWindowSize(mImpl->mGLFWwindow, &mFullScreenDim.w, &mFullScreenDim.h);
    glfwGetWindowPos(mImpl->mGLFWwindow, &mFullScreenDim.l, &mFullScreenDim.t);
    vsync(mVSync);
  }
  else {
    glfwSetWindowMonitor(
      mImpl->mGLFWwindow, NULL,
      mDim.l, mDim.t, mDim.w, mDim.h,
      GLFW_DONT_CARE // refreshRate 
    );
    vsync(mVSync);
  }
#endif
}

void Window::implSetTitle() {
}

// See: https://www.opengl.org/wiki/Swap_Interval
void Window::implSetVSync() {
  mImpl->makeCurrent();
  glfwSwapInterval(mVSync? 1 : 0);
}

void Window::implHide() {

}

void Window::implIconify() {

}

void Window::implSetDecorated() {

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

WindowImpl::KeyMap WindowImpl::make_glfw_keymap() {
  KeyMap km;

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

  return km;
}

}