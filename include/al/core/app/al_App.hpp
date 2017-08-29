#ifndef INCLUDE_AL_APP_HPP
#define INCLUDE_AL_APP_HPP

/*  Keehong Youn, 2017, younkeehong@gmail.com
*/

#include "al/core/app/al_WindowApp.hpp"
#include "al/core/app/al_AudioApp.hpp"
#include "al/core/protocol/al_OSC.hpp"

#include "al/core/gl/al_Graphics.hpp"
#include "al/core/gl/al_Viewpoint.hpp"
#include "al/core/spatial/al_Pose.hpp"
#include "al/core/io/al_ControlNav.hpp"

#include <iostream>

namespace al {

// Unified app class: single window, audioIO, and
//   single port osc recv & single port osc send
// TODO: better osc interface
class App: public WindowApp, public AudioApp, public osc::PacketHandler {
public:

  virtual void onAnimate(double dt) {}
  virtual void onExit() {}

  virtual void preOnAnimate(double dt) {}

  // overrides WindowApp's start to also initiate AudioApp and etc.
  virtual void start() override {
    open(); // WindowApp (glfw::init(), onInit(), create(), onCreate())
    startFPS(); // WindowApp (FPS)
    beginAudio(); // AudioApp (only begins if `initAudio` was called before)
    while (!shouldQuit()) {
      // user can quit this loop with WindowApp::quit() or clicking close button
      // or with stdctrl class input (ctrl+q)
      preOnAnimate(dt() / 1000000);
      onAnimate(dt() / 1000000); // millis for dt
      loop(); // WindowApp (onDraw, refresh)
      tickFPS(); // WindowApp (FPS)
    }
    onExit(); // user defined
    endAudio(); // AudioApp
    close(); // WindowApp (calls onExit)
  }

  // PacketHandler
  virtual void onMessage(osc::Message& m) override {}
};

class EasyApp : public App {
public:
  Graphics g {*this};
  Viewpoint view;
  NavInputControl nav;

  ShaderProgram mesh_shader;
  ShaderProgram color_shader;
  ShaderProgram tex_shader;

  class EasyAppEventHandler : public WindowEventHandler {
  public:
    EasyApp* app;
    EasyAppEventHandler(EasyApp& a): app(&a) {}
    virtual bool resize(int dw, int dh) override {
      app->view.viewport(0, 0, app->fbWidth(), app->fbHeight());
      return true;
    }
  };
  EasyAppEventHandler eventHandler {*this};

  virtual void preOnCreate() override {
    append(eventHandler);
    append(nav);
    nav.target(view);
    mesh_shader.compile(al_mesh_vert_shader(), al_mesh_frag_shader());
    color_shader.compile(al_color_vert_shader(), al_color_frag_shader());
    tex_shader.compile(al_tex_vert_shader(), al_tex_frag_shader());
    view.pos(Vec3f(0, 0, 20)).faceToward(Vec3f(0, 0, 0), Vec3f(0, 1, 0));
    view.fovy(45).near(0.1).far(1000);
    view.viewport(0, 0, fbWidth(), fbHeight());
  }

  virtual void preOnAnimate(double dt) override {
      nav.step();
  }

  virtual void preOnDraw() override {

  }

};

}

#endif