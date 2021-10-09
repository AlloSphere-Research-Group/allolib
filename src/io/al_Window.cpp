#include "al/io/al_Window.hpp"

#include <stdio.h>

namespace al {

Keyboard::Keyboard() : mKeycode(-1), mDown(false) {
  for (int i = 0; i < 5; ++i)
    mModifiers[i] = false;
}
int Keyboard::key() const { return mKeycode; }
int Keyboard::keyAsNumber() const { return key() - 48; }
bool Keyboard::down() const { return mDown; }
bool Keyboard::isNumber() const { return (key() >= '0') && (key() <= '9'); }
bool Keyboard::alt() const { return mModifiers[1]; }
bool Keyboard::caps() const { return mModifiers[3]; }
bool Keyboard::ctrl() const { return mModifiers[2]; }
bool Keyboard::meta() const { return mModifiers[4]; }
bool Keyboard::shift() const { return mModifiers[0]; }
bool Keyboard::key(int k) const { return mKeycode == k; }
void Keyboard::alt(bool state) { mModifiers[1] = state; }
void Keyboard::caps(bool state) { mModifiers[3] = state; }
void Keyboard::ctrl(bool state) { mModifiers[2] = state; }
void Keyboard::meta(bool state) { mModifiers[4] = state; }
void Keyboard::shift(bool state) { mModifiers[0] = state; }
void Keyboard::setKey(int k, bool v) {
  mKeycode = k;
  mDown = v;
}
void Keyboard::print(std::ostream &stream) const {
  stream << "key= " << key() << "(" << char(key()) << "), alt=" << alt()
         << ", ctrl=" << ctrl() << ", meta=" << meta() << ", shift=" << shift()
         << ", caps=" << caps() << std::endl;
}

Mouse::Mouse() : mX(0), mY(0), mDX(0), mDY(0), mButton(LEFT) {
  for (int i = 0; i < AL_MOUSE_MAX_BUTTONS; ++i) {
    mBX[i] = mBY[i] = 0;
    mB[i] = false;
  }
}
int Mouse::x() const { return mX; }
int Mouse::y() const { return mY; }
int Mouse::dx() const { return mDX; }
int Mouse::dy() const { return mDY; }
double Mouse::scrollX() const { return mScrollX; }
double Mouse::scrollY() const { return mScrollY; }
int Mouse::button() const { return mButton; }
bool Mouse::down() const { return down(mButton); }
bool Mouse::down(int button) const { return mB[button]; }
bool Mouse::left() const { return mB[LEFT]; }
bool Mouse::middle() const { return mB[MIDDLE]; }
bool Mouse::right() const { return mB[RIGHT]; }
void Mouse::button(int b, bool v) {
  mButton = b;
  mB[b] = v;
  if (v) {
    mBX[b] = mX;
    mBY[b] = mY;
  }
}
void Mouse::position(int x, int y) {
  mDX = x - mX;
  mDY = y - mY;
  mX = x;
  mY = y;
}
void Mouse::scroll(double x, double y) {
  mScrollX = x;
  mScrollY = y;
}

WindowEventHandler::WindowEventHandler() : mWindow(nullptr) {}
WindowEventHandler::~WindowEventHandler() { removeFromWindow(); }
void WindowEventHandler::removeFromWindow() {
  if (attached()) {
    window().remove(*this);
    mWindow = nullptr;
  }
}

bool Window::create(bool verbose) {
  if (!created()) {
    if (implCreate(verbose)) {
      return true;
    }
  }
  return false;
}
bool Window::created() const { return implCreated(); }
void Window::makeCurrent() { implMakeCurrent(); }
void Window::refresh() { implRefresh(); }
void Window::destroy() {
  if (created()) {
    implDestroy();

    for (auto handler : windowEventHandlers()) {
      handler->removeFromWindow();
    }
  }
}

void Window::close() {
  if (created())
    implClose();
}

bool Window::shouldClose() {
  if (created()) {
    return implShouldClose();
  } else {
    return true;
  }
}

Window::Dim Window::dimensions() const {
  if (!mFullScreen) {
    return mDim;
  } else {
    return mFullScreenDim;
  }
}
double Window::aspect() const { return dimensions().aspect(); }
bool Window::cursorHide() const { return mCursorHide; }
bool Window::fullScreen() const { return mFullScreen; }
const std::string &Window::title() const { return mTitle; }
bool Window::visible() const { return mVisible; }
bool Window::vsync() const { return mVSync; }
bool Window::enabled(DisplayMode v) const { return mDisplayMode & v; }
int Window::height() const { return dimensions().h; }
int Window::width() const { return dimensions().w; }
int Window::fbHeight() const { return mFramebufferHeight; }
int Window::fbWidth() const { return mFramebufferWidth; }
Window::Cursor Window::cursor() const { return mCursor; }
bool Window::decorated() const { return mDecorated; }

void Window::cursorHideToggle() { cursorHide(!cursorHide()); }

void Window::cursor(Cursor v) {
  mCursor = v;
  if (created())
    implSetCursor();
}

void Window::cursorHide(bool v) {
  mCursorHide = v;
  if (created())
    implSetCursorHide();
}

void Window::dimensions(const Dim &v) {
  mDim = v;
  if (created())
    implSetDimensions();
}

void Window::dimensions(int w, int h) {
  return dimensions(Window::Dim(mDim.l, mDim.t, w, h));
}

void Window::dimensions(int x, int y, int w, int h) {
  return dimensions(Window::Dim(x, y, w, h));
}

Window::DisplayMode Window::displayMode() const { return mDisplayMode; }

void Window::displayMode(DisplayMode v) {
  if (mDisplayMode != v) {
    if (created()) {
      const Cursor cursor_ = cursor();
      const bool cursorHide_ = cursorHide();
      const Dim dim_ = dimensions();
      const bool fullScreen_ = fullScreen();
      const std::string &title_ = title();

      destroy();
      mDisplayMode = v;
      dimensions(dim_);
      title(title_);
      create();
      cursor(cursor_);
      cursorHide(cursorHide_);
      fullScreen(fullScreen_);
    } else {
      mDisplayMode = v;
    }
  }
}

void Window::fullScreen(bool v, int monitorIndex) {
  if (v != mFullScreen) {
    mFullScreen = v;
    if (created())
      implSetFullScreen(monitorIndex);
  }
}

void Window::fullScreenToggle() { fullScreen(!fullScreen()); }

void Window::title(const std::string &v) {
  mTitle = v;
  if (created())
    implSetTitle();
}

void Window::vsync(bool v) {
  mVSync = v;
  if (created())
    implSetVSync();
}

void Window::hide() {
  if (created())
    implHide();
}

void Window::iconify() {
  if (created())
    implIconify();
}

void Window::decorated(bool b) {
  mDecorated = b;
  if (created()) {
    implSetDecorated(b);
  }
}

Window &Window::insert(WindowEventHandler &v, int i) {
  WindowEventHandlers &H = mWindowEventHandlers;
  if (std::find(H.begin(), H.end(), &v) == H.end()) {
    v.removeFromWindow();
    H.insert(H.begin() + i, &(v.window(this)));

    // notify new handler of changes if the window already is created
    // otherwise, the window will call the proper handlers when created
    if (created()) {
      v.resize(width(), height());
    }
  }
  return *this;
}

Window &Window::append(WindowEventHandler &v) {
  return insert(v, (int)mWindowEventHandlers.size());
}

Window &Window::prepend(WindowEventHandler &v) { return insert(v, 0); }

Window &Window::remove(WindowEventHandler &v) {
  WindowEventHandlers &H = mWindowEventHandlers;
  WindowEventHandlers::iterator it = std::find(H.begin(), H.end(), &v);
  if (it != H.end()) {
    H.erase(it);
    // the proper way to do it:
    // H.erase(std::remove(H.begin(), H.end(), &v), H.end());
    // if(started()){
    //   v.onResize(-width(), -height());
    //   printf("WindowEventHandler %p onResize(%d, %d)\n", &v, width(),
    //   height());
    // }
    // if(created()){
    //   v.onDestroy();
    // }
    v.mWindow = NULL;
  }
  return *this;
}

void *Window::windowHandle() { return implWindowHandle(); }

} // namespace al
