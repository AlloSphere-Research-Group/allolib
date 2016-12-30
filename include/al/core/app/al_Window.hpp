#ifndef INCLUDE_AL_WINDOWGL_HPP
#define INCLUDE_AL_WINDOWGL_HPP

/*  Allocore --
  Multimedia / virtual environment application class library

  Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
  Copyright (C) 2012. The Regents of the University of California.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    Neither the name of the University of California nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.


  File description:
  An interface to an OS window

  File author(s):
  Lance Putnam, 2010, putnam.lance@gmail.com
  Graham Wakefield, 2010, grrrwaaa@gmail.com
  Wesley Smith, 2010, wesley.hoke@gmail.com
*/

#include <algorithm>
#include <vector>
#include <string>
#include <stdio.h>
#include <iostream>
#include <memory>

using namespace std;

namespace al{

class Window;

// can redefine, but should be at least 4
#ifndef AL_MOUSE_MAX_BUTTONS
#define AL_MOUSE_MAX_BUTTONS 4
#endif


/// Keyboard state
///
/// @ingroup allocore
class Keyboard{
public:

  /// Non-printable keys
  enum Key{

    // Standard ASCII non-printable characters
    ENTER    =3,    /**< */
    BACKSPACE  =8,    /**< */
    TAB      =9,    /**< */
    RETURN    =13,  /**< */
    ESCAPE    =27,  /**< */
    DELETE    =127,  /**< */

    // Non-standard, but common keys
    F1=256, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    INSERT, LEFT, UP, RIGHT, DOWN, PAGE_DOWN, PAGE_UP, END, HOME
  };

  Keyboard();

  int key() const;      ///< Returns character or code of last key event
  int keyAsNumber() const;  ///< Returns decimal number correlating to key code
  bool alt() const;      ///< Whether an alt key is down
  bool caps() const;      ///< Whether capslock is down
  bool ctrl() const;      ///< Whether a control key is down
  bool meta() const;      ///< Whether a meta (e.g. windows, apple) key is down
  bool shift() const;      ///< Whether a shift key is down
  bool down() const;      ///< Whether last event was button down
  bool isNumber() const;    ///< Whether key is a number key
  bool key(int k) const;    ///< Whether the last key was 'k'

  void alt  (bool state);    ///< Set alt key state
  void caps (bool state);    ///< Set alt key state
  void ctrl (bool state);    ///< Set ctrl key state
  void meta (bool state);    ///< Set meta key state
  void shift(bool state);    ///< Set shift key state

  void print() const;      ///< Print keyboard state

protected:
  friend class WindowImpl;

  int  mKeycode;    // last key event key number
  bool mDown;      // last key event state (pressed or released)
  bool mModifiers[5];  // Modifier key state array (shift, alt, ctrl, caps, meta)

  void setKey(int k, bool v);
};


/// Mouse state
///
/// @ingroup allocore
class Mouse{
public:
  enum{
    LEFT  = 0,      /**< Left button */
    MIDDLE  = 1,      /**< Middle button */
    RIGHT  = 2,      /**< Right button */
    EXTRA  = 3        /**< Start of any extra buttons */
  };

  Mouse();

  int x() const;        ///< Get x position relative to top-left corner of window, in pixels
  int y() const;        ///< Get x position relative to top-left corner of window, in pixels
  int dx() const;        ///< Get change in x position, in pixels
  int dy() const;        ///< Get change in y position, in pixels

  int button() const;      ///< Get last clicked button
  bool down() const;      ///< Get state of last clicked button
  bool down(int button) const;///< Get state of a button
  bool left() const;      ///< Get whether left button is down
  bool middle() const;    ///< Get whether middle button is down
  bool right() const;      ///< Get whether right button is down

protected:
  friend class WindowImpl;

  int mX, mY;            // x,y positions
  int mDX, mDY;          // change in x,y positions
  int mButton;          // most recent button changed
  int mBX[AL_MOUSE_MAX_BUTTONS];  // button down xs
  int mBY[AL_MOUSE_MAX_BUTTONS];  // button down ys
  bool mB[AL_MOUSE_MAX_BUTTONS];  // button states

  void button(int b, bool v);
  void position(int x, int y);
};

/// Controller for handling input and window events

/// The return value of the event handlers determines whether or not
/// the event should be propagated to other handlers.
///
/// @ingroup allocore
class WindowEventHandler {
public:
  WindowEventHandler();
  virtual ~WindowEventHandler();

  /// Called when a keyboard key is pressed
  virtual bool keyDown(const Keyboard& k){return true;}

  /// Called when a keyboard key is released
  virtual bool keyUp(const Keyboard& k){return true;}

  /// Called when a mouse button is pressed
  virtual bool mouseDown(const Mouse& m){return true;}

  /// Called when the mouse moves while a button is down
  virtual bool mouseDrag(const Mouse& m){return true;}

  /// Called when the mouse moves
  virtual bool mouseMove(const Mouse& m){return true;}

  /// Called when a mouse button is released
  virtual bool mouseUp(const Mouse& m){return true;}

  /// Called whenever window dimensions change
  virtual bool resize(int dw, int dh){ return true; }

  /// Called when window changes from hidden to shown and vice versa
  virtual bool visibility(bool v){ return true; }

  /// Return self
  WindowEventHandler& windowEventHandler(){ return *this; }

  bool attached() const { return NULL != mWindow; }
  Window& window(){ return *mWindow; }
  const Window& window() const { return *mWindow; }

private:
  friend class Window;
  Window * mWindow;
  WindowEventHandler& window(Window * v){ mWindow=v; return *this; }
  void removeFromWindow();
};


/// Window with OpenGL context

/// Upon construction, the Window will add itself to its list of input event
/// handlers and its list of window event handlers.
///
/// @ingroup allocore
// class Window : public InputEventHandler, public WindowEventHandler {
class Window {
public:
  typedef std::vector<WindowEventHandler *> WindowEventHandlers;

  /// Window display mode bit flags
  enum DisplayMode{
    SINGLE_BUF  = 1<<0,    /**< Do single-buffering */
    DOUBLE_BUF  = 1<<1,    /**< Do double-buffering */
    STEREO_BUF  = 1<<2,    /**< Do left-right stereo buffering */
    ACCUM_BUF  = 1<<3,    /**< Use accumulation buffer */
    ALPHA_BUF  = 1<<4,    /**< Use alpha buffer */
    DEPTH_BUF  = 1<<5,    /**< Use depth buffer */
    STENCIL_BUF  = 1<<6,    /**< Use stencil buffer */
    MULTISAMPLE = 1<<7,    /**< Multisampling support */
    DEFAULT_BUF  = DOUBLE_BUF|ALPHA_BUF|DEPTH_BUF /**< Default display mode */
  };

  /// Cursor icon types
  enum Cursor{
    NONE    = 0,    /**< */
    POINTER    = 1,    /**< */
    CROSSHAIR        /**< */
  };

  /// Window pixel dimensions
  struct Dim{
    int l,t,w,h;
    Dim(int v=0): l(0), t(0), w(v), h(v){}
    Dim(int w_, int h_): l(0), t(0), w(w_), h(h_){}
    Dim(int l_, int t_, int w_, int h_): l(l_), t(t_), w(w_), h(h_){}
    void set(int l_, int t_, int w_, int h_){l=l_;t=t_;w=w_;h=h_;}

    float aspect() const { return (w!=0 && h!=0) ? double(w)/h : 1; }
    void print() const {printf("Dim: %4d x %4d @ (%4d, %4d)\n", w,h, l,t); }
  };

  Window();
  virtual ~Window();

  /// Create window and its associated graphics context using current settings
  /// This will create a new window only if the the window has not already
  /// been created.
  /// \returns whether a valid window is created
  bool create();

  // refresh window (swap buffers, poll events, etc.)
  void refresh();

  /// Destroy current window and its associated graphics context
  void destroy();

  const Keyboard& keyboard() const { return mKeyboard; }  ///< Get current keyboard state
  const Mouse& mouse() const { return mMouse; }      ///< Get current mouse state

  double aspect() const;        ///< Get aspect ratio (width divided by height)
  bool created() const;        ///< Whether window has been created providing a valid graphics context
  Cursor cursor() const;        ///< Get current cursor type
  bool cursorHide() const;      ///< Whether the cursor is hidden
  Dim dimensions() const;        ///< Get current dimensions of window
  DisplayMode displayMode() const;  ///< Get current display mode
  bool enabled(DisplayMode v) const;  ///< Get whether display mode flag is set
  bool fullScreen() const;      ///< Get whether window is in fullscreen
  const std::string& title() const;  ///< Get title of window
  bool visible() const;        ///< Get whether window is visible
  bool vsync() const;          ///< Get whether v-sync is enabled

  int height() const; ///< Get window height, in pixels
  int width() const; ///< Get window width, in pixels

  // get frambuffer size
  // it will be different from window widht and height
  // if the display is a high resolution one (ex: RETINA display)
  int fbHeight() const { return int(mDim.h * highres_factor_h); };
  int fbWidth() const { return int(mDim.w * highres_factor_w); };

  Window& cursor(Cursor v);      ///< Set cursor type
  Window& cursorHide(bool v);      ///< Set cursor hiding
  Window& cursorHideToggle();      ///< Toggle cursor hiding
  Window& dimensions(const Dim& v);  ///< Set dimensions
  Window& dimensions(int w, int h);  ///< Set dimensions
  Window& displayMode(DisplayMode v);  ///< Set display mode; will recreate window if different from current

  /// This will make the window go fullscreen without borders and,
  /// if posssible, without changing the display resolution.
  Window& fullScreen(bool on);
  Window& fullScreenToggle(); ///< Toggle fullscreen
  Window& hide(); ///< Hide window (if showing)
  Window& iconify(); ///< Iconify window
  Window& show(); ///< Show window (if hidden)
  Window& title(const std::string& v); ///< Set title
  Window& vsync(bool v); ///< Set whether to sync the frame rate to the monitor's refresh rate
  Window& asap(bool v);  ///< Set whether window renders as fast as possible

  WindowEventHandlers const& windowEventHandlers() const {
    return mWindowEventHandlers;
  }

  /// Append handler to window event handler list
  /// The order of handlers in the list matches their calling order.
  Window& append(WindowEventHandler& v);

  /// Prepend handler to input event handler list
  /// The order of handlers in the list matches their calling order.
  Window& prepend(WindowEventHandler& v);

  /// Remove all window event handlers matching argument
  Window& remove(WindowEventHandler& v);

  /// Destroy all created windows
  static void destroyAll();

protected:
  friend class WindowImpl;

  // class WindowImpl * mImpl;
  std::unique_ptr<class WindowImpl> mImpl;
  Keyboard mKeyboard;
  Mouse mMouse;
  WindowEventHandlers mWindowEventHandlers;
  Dim mDim {0, 0, 900, 450};
  Dim mFullScreenDim {0, 0, 900, 450};
  DisplayMode mDisplayMode = DEFAULT_BUF;
  std::string mTitle = "";
  Cursor mCursor = POINTER;
  bool mCursorHide = false;
  bool mFullScreen = false;
  bool mVisible = false;
  bool mVSync = false;

  // for high pixel density monitors (RETINA, etc.)
  float highres_factor_h = 1;
  float highres_factor_w = 1;

  // Must be defined in pimpl-specific file
  bool implCreate();
  bool implCreated() const;
  void implRefresh();
  void implDestroy();
  void implSetCursor();
  void implSetDimensions();
  void implSetFullScreen();
  void implSetTitle();
  void implSetVSync();
  void implHide();
  void implIconify();

  Window& insert(WindowEventHandler& v, int i);

  #define CALL(e){\
    for(unsigned i=0; i<mWindowEventHandlers.size(); ++i){\
      if(false == mWindowEventHandlers[i]->e) break;\
    }\
  }
  void callHandlersMouseDown(){ CALL(mouseDown(mMouse)); }
  void callHandlersMouseDrag(){ CALL(mouseDrag(mMouse)); }
  void callHandlersMouseMove(){ CALL(mouseMove(mMouse)); }
  void callHandlersMouseUp(){ CALL(mouseUp(mMouse)); }
  void callHandlersKeyDown(){ CALL(keyDown(mKeyboard)); }
  void callHandlersKeyUp(){ CALL(keyUp(mKeyboard)); }
  void callHandlersResize(int w, int h){ CALL(resize(w, h)); }
  void callHandlersVisibility(bool v){ CALL(visibility(v)); }
  #undef CALL

public:
  // deprecated
  Window& add(WindowEventHandler * v){ return append(*v); }
  Window& prepend(WindowEventHandler * v){ return prepend(*v); }
  Window& remove(WindowEventHandler * v){ return remove(*v); }
};

inline Window::DisplayMode
operator| (const Window::DisplayMode& a, const Window::DisplayMode& b){ return Window::DisplayMode(+a | +b); }

inline Window::DisplayMode
operator& (const Window::DisplayMode& a, const Window::DisplayMode& b){ return Window::DisplayMode(+a & +b); }

/// Standard key controls for window
///
/// @ingroup allocore
struct StandardWindowKeyControls : WindowEventHandler {
  bool keyDown(const Keyboard& k){
    if(k.ctrl()){
      switch(k.key()){
        case 'q': window().destroy(); return false;
        case 'h': window().hide(); return false;
        case 'm': window().iconify(); return false;
        case 'c': window().cursorHideToggle(); return false;
        default:;
      }
    }
    else{
      switch(k.key()){
        case Keyboard::ESCAPE: window().fullScreenToggle(); return false;
        default:;
      }
    }
    return true;
  }
};

} // al::

#endif
