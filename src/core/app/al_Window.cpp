#include "al/core/app/al_Window.hpp"
#include <stdio.h>

namespace al{

Keyboard::Keyboard()
:  mKeycode(-1), mDown(false)
{
  for(int i=0; i<5; ++i) mModifiers[i] = false;
}

int Keyboard::key() const { return mKeycode; }
int Keyboard::keyAsNumber() const { return key() - 48; }
bool Keyboard::down() const { return mDown; }
bool Keyboard::isNumber() const { return (key() >= '0') && (key() <= '9'); }
bool Keyboard::alt()   const { return mModifiers[1]; }
bool Keyboard::caps()  const { return mModifiers[3]; }
bool Keyboard::ctrl()  const { return mModifiers[2]; }
bool Keyboard::meta()  const { return mModifiers[4]; }
bool Keyboard::shift() const { return mModifiers[0]; }
bool Keyboard::key(int k) const { return mKeycode == k; }
void Keyboard::alt  (bool state){mModifiers[1] = state;}
void Keyboard::caps (bool state){mModifiers[3] = state;}
void Keyboard::ctrl (bool state){mModifiers[2] = state;}
void Keyboard::meta (bool state){mModifiers[4] = state;}
void Keyboard::shift(bool state){mModifiers[0] = state;}
void Keyboard::setKey(int k, bool v){ mKeycode=k; mDown=v; }

void Keyboard::print() const {
  fprintf(stderr,
    "key=%3d (%c), alt=%i, ctrl=%i, meta=%i, shift=%i, caps=%i\n",
    key(),key(), alt(), ctrl(), meta(), shift(), caps()
  );
}


Mouse::Mouse(): mX(0), mY(0), mDX(0), mDY(0), mButton(LEFT){
  for(int i=0; i<AL_MOUSE_MAX_BUTTONS; ++i){
    mBX[i] = mBY[i] = 0; mB[i] = false;
  }
}

int Mouse::x() const { return mX; }
int Mouse::y() const { return mY; }
int Mouse::dx() const { return mDX; }
int Mouse::dy() const { return mDY; }

int Mouse::button() const { return mButton; }
bool Mouse::down() const { return down(mButton); }
bool Mouse::down(int button) const { return mB[button]; }
bool Mouse::left() const { return mB[LEFT]; }
bool Mouse::middle() const { return mB[MIDDLE]; }
bool Mouse::right() const { return mB[RIGHT]; }

void Mouse::button(int b, bool v){ mButton=b; mB[b]=v; if(v){ mBX[b]=mX; mBY[b]=mY; } }
void Mouse::position(int x, int y){ mDX=x-mX; mDY=y-mY; mX=x; mY=y; }


WindowEventHandler::WindowEventHandler() :
  mWindow(NULL)
{
  //
}

WindowEventHandler::~WindowEventHandler() {
  removeFromWindow();
}

void WindowEventHandler::removeFromWindow(){
  if(attached()){
    window().remove(this); // Window::remove calls onResize
  }
}


Window::Window() :
  mDim(0, 0, 800, 600), mDisplayMode(DEFAULT_BUF), mCursor(POINTER),
  mASAP(false), mCursorHide(false), mFullScreen(false),
  mVisible(false), mVSync(true)
{
  implCtor(); // must call first!
}

Window::~Window() {
  destroy();
  implDtor();
}

bool Window::create(
  const Dim& dim, const std::string& title, DisplayMode mode
){
  if(!created()){
    mDim = dim;
    mTitle = title;
    mDisplayMode = mode;
    if(implCreate()){
      return true;
    }
  }
  return false;
}

void Window::destroy(){
  if(created()){
    implDestroy();
  }
}

bool Window::asap() const {
  return mASAP;
}

Window& Window::asap(bool v){
  mASAP=v;
  return *this;
}

double Window::aspect() const {
  return dimensions().aspect();
}

Window& Window::cursorHideToggle(){
  cursorHide(!cursorHide());
  return *this;
}

Window::Cursor Window::cursor() const {
  return mCursor;
}

Window& Window::cursor(Cursor v){
  mCursor = v;
  if(created()) implSetCursor();
  return *this;
}

bool Window::cursorHide() const {
  return mCursorHide;
}

Window& Window::cursorHide(bool v){
  mCursorHide = v;
  if(created()) implSetCursorHide();
  return *this;
}

Window::Dim Window::dimensions() const {
  return mDim;
}

Window& Window::dimensions(const Dim& v){
  mDim = v;
  if(created()) implSetDimensions();
  return *this;
}

Window::DisplayMode Window::displayMode() const {
  return mDisplayMode;
}

Window& Window::displayMode(DisplayMode v){
  if(mDisplayMode != v){
    if(created()){
      const Cursor cursor_ = cursor();
      const bool cursorHide_ = cursorHide();
      const Dim dim_ = dimensions();
      const bool fullScreen_ = fullScreen();
      const std::string& title_ = title();

      destroy();
      create(dim_, title_, v);
      cursor(cursor_);
      cursorHide(cursorHide_);
      fullScreen(fullScreen_);
    }
    else{
      mDisplayMode = v;
    }
  }
  return *this;
}

bool Window::fullScreen() const {
  return mFullScreen;
}

Window& Window::fullScreen(bool v){
  if(v != mFullScreen){
    mFullScreen = v;
    if(created()) implSetFullScreen();
  }
  return *this;
}

Window& Window::fullScreenToggle(){
  fullScreen(!fullScreen());
  return *this;
}

const std::string& Window::title() const {
  return mTitle;
}

Window& Window::title(const std::string& v){
  mTitle = v;
  if(created()) implSetTitle();
  return *this;
}

bool Window::visible() const {
  return mVisible;
}

bool Window::vsync() const {
  return mVSync;
}

Window& Window::vsync(bool v){
  mVSync = v;
  if(created()) implSetVSync();
  return *this;
}

bool Window::enabled(DisplayMode v) const {
  return mDisplayMode & v;
}

Window& Window::insert(WindowEventHandler& v, int i){
  WindowEventHandlers& H = mWindowEventHandlers;
  if(std::find(H.begin(), H.end(), &v) == H.end()){
    v.removeFromWindow();
    H.insert(H.begin()+i, &(v.window(this)));

    // notify new handler of changes if the window already is created
    // otherwise, the window will call the proper handlers when created
    if(created()){
      v.resize(width(), height());
    }
  }
  return *this;
}

Window& Window::append(WindowEventHandler& v) {
  return insert(v, mWindowEventHandlers.size());
}

Window& Window::prepend(WindowEventHandler& v){
  return insert(v,0);
}

Window& Window::remove(WindowEventHandler& v) {
  WindowEventHandlers& H = mWindowEventHandlers;
  WindowEventHandlers::iterator it = std::find(H.begin(), H.end(), &v);
  if(it != H.end()) {
    H.erase(it);
    // the proper way to do it:
    // H.erase(std::remove(H.begin(), H.end(), &v), H.end());
    // if(started()){
    //   v.onResize(-width(), -height());
    //   printf("WindowEventHandler %p onResize(%d, %d)\n", &v, width(), height());
    // }
    // if(created()){
    //   v.onDestroy();
    // }
    v.mWindow = NULL;
  }
  return *this;
}

} // al::
