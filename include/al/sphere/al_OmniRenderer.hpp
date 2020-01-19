#ifndef INCLUDE_OMNIRENDER_HPP
#define INCLUDE_OMNIRENDER_HPP

#include <iostream>

#include "al/app/al_WindowApp.hpp"
#include "al/sphere/al_PerProjection.hpp"
#include "al/sphere/al_SphereUtils.hpp"
//#include "al/graphics/al_GLFW.hpp"
#include "al/graphics/al_Graphics.hpp"

namespace al {

struct OmniRenderer : WindowApp {
  PerProjectionRender pp_render;
  bool render_stereo = true;
  bool running_in_sphere_renderer = false;
  bool window_is_stereo_buffered = false;
  int eye_to_render = -1;  // -1 for mono, 0: left, 1: right
  Lens mLens;
  Pose mPose;

  OmniRenderer() : WindowApp() {}
  // `WindowApp()` appends std window controls and itself as event handler

  // omni-stereo related functions
  void stereo(bool b) { render_stereo = b; }
  void toggleStereo() { render_stereo = !render_stereo; }
  void omniResolution(int res) { pp_render.update_resolution(res); }
  int omniResolution() { return pp_render.res_; }
  // void sphereRadius(float radius) { pp_render.sphereRadius(radius); }

  // only for testing with desktop mode, loops (mono -> left -> right)
  void loopEyeForDesktopMode() {
    eye_to_render += 1;
    if (eye_to_render > 1) eye_to_render = -1;
  }

  // only for testing with desktop mode, -1 for mono, 0: left, 1: right
  void setEyeToRenderForDesktopMode(int eye) {
    eye_to_render = eye;
    if (eye_to_render < -1 || eye_to_render > 1) eye_to_render = -1;
  }

  // getters trying to match al::App interface
  virtual Graphics &graphics() { return mGraphics; }
  virtual const Graphics &graphics() const { return mGraphics; }
  virtual Lens &lens() { return mLens; }
  virtual const Lens &lens() const { return mLens; }
  virtual void pose(Pose const &p) { mPose = p; }
  virtual Pose &pose() { return mPose; }
  virtual Pose const &pose() const { return mPose; }

  // for user to override
  void onInit() override {}
  void onCreate() override {}
  void onAnimate(double dt) override {}
  void onDraw(Graphics &g) override {}
  void onExit() override {}
  //  void onKeyDown(Keyboard const &k) override {}
  //  void onKeyUp(Keyboard const &k) override {}
  //  void onMouseDown(Mouse const &m) override {}
  //  void onMouseUp(Mouse const &m) override {}
  //  void onMouseDrag(Mouse const &m) override {}
  //  void onMouseMove(Mouse const &m) override {}
  void onResize(int w, int h) override {}
  void onVisibility(bool v) override {}

  // run app
  void start() override;

  // omni functionality, impl below out-of-line
  void check_if_in_sphere_and_setup_window_dimensions();
  void load_perprojection_configuration();
  void draw_using_perprojection_capture();

  // input events in case on desktop mode
};

inline void OmniRenderer::start() {
  initializeWindowManager();
  onInit();
  check_if_in_sphere_and_setup_window_dimensions();
  Window::create(is_verbose);
  window_is_stereo_buffered = Window::displayMode() & Window::STEREO_BUF;
  mGraphics.init();
  load_perprojection_configuration();
  cursorHide(true);
  onCreate();
  FPS::startFPS();
  while (!shouldQuit()) {
    onAnimate(dt_sec());  // millis for dt
    draw_using_perprojection_capture();
    Window::refresh();
    FPS::tickFPS();
  }
  onExit();
  Window::destroy();
  terminateWindowManager();
}

inline void OmniRenderer::check_if_in_sphere_and_setup_window_dimensions() {
  if (sphere::is_renderer()) running_in_sphere_renderer = true;

  if (running_in_sphere_renderer) {
    int width, height;
    sphere::get_fullscreen_dimension(&width, &height);
    if (width != 0 && height != 0) {
      dimensions(0, 0, width, height);
      decorated(false);
    } else {
      std::cout << "[!] in sphere renderer but calculated "
                << "width and/or height are/is zero!" << std::endl;
    }
  }
}

inline void OmniRenderer::load_perprojection_configuration() {
  if (running_in_sphere_renderer) {
    // need to be called before pp_render.init
    pp_render.load_calibration_data(
        sphere::config_directory("data").c_str(),    // path
        sphere::renderer_hostname("config").c_str()  // hostname
    );  // parameters will be used to look for file ${path}/${hostname}.txt
    pp_render.init(lens());
  } else {
    // load fake projection data for desktop rendering
    pp_render.load_and_init_as_desktop_config(lens());
  }
}

inline void OmniRenderer::draw_using_perprojection_capture() {
  // start drawing to perprojection fbos

  mGraphics.omni(true);  // set true to use omni default shaders when drawing
  // lens and pose for rendering is set in PerProjectionRender::begin
  // so updating those in onDraw will not have effect in rendering
  // will setting up omni rendering,
  // begin also pushes fbo, viewport, viewmat, projmat, lens, shader
  pp_render.begin(mGraphics, lens(), pose());
  glDrawBuffer(GL_COLOR_ATTACHMENT0);  // for fbo's output
  if (render_stereo) {
    for (int eye = 0; eye < 2; eye += 1) {
      pp_render.set_eye(eye);
      for (int i = 0; i < pp_render.num_projections(); i++) {
        pp_render.set_projection(i);
        gl::depthTesting(true);
        gl::depthMask(true);
        gl::blending(false);
        onDraw(mGraphics);
      }
    }
  } else {
    // std::cout << "rendering eye " << eye_to_render << std::endl;
    pp_render.set_eye(eye_to_render);
    for (int i = 0; i < pp_render.num_projections(); i++) {
      pp_render.set_projection(i);
      gl::depthTesting(true);
      gl::depthMask(true);
      gl::blending(false);
      onDraw(mGraphics);
    }
  }
  pp_render.end();  // pops everything pushed before

  /* Settings for warp and blend composition sampling
   */
  mGraphics.omni(false);  // warp and blend composition done on flat rendering
  mGraphics.eye(Graphics::MONO_EYE);  // stereo handled at capture stage
  gl::polygonMode(GL_FILL);           // to draw viewport filling quad
  gl::blending(false);                // blending already done when capturing
  gl::depthTesting(false);  // no depth testing when drawing viewport slab
  mGraphics.pushViewport(0, 0, fbWidth(),
                         fbHeight());  // filling the whole window

  // now sample the results
  if (running_in_sphere_renderer) {
    if (window_is_stereo_buffered) {
      // rendering stereo in sphere
      glDrawBuffer(GL_BACK_LEFT);
      gl::clearColor(0, 0, 0);
      gl::clearDepth(1);
      pp_render.composite(mGraphics, 0);
      glDrawBuffer(GL_BACK_RIGHT);
      gl::clearColor(0, 0, 0);
      gl::clearDepth(1);
      pp_render.composite(mGraphics, 1);
    } else {  // rendering mono in sphere
      // std::cout << "sampling mono in sphere setup" << std::endl;
      glDrawBuffer(GL_BACK_LEFT);
      gl::clearColor(1, 0, 0);
      gl::clearDepth(1);
      pp_render.composite(mGraphics, (eye_to_render == 1) ? 1 : 0);
    }
  } else {
    if (window_is_stereo_buffered) {
      // rendering stereo on display other than sphere
      glDrawBuffer(GL_BACK_LEFT);
      gl::clearColor(0, 0, 0);
      gl::clearDepth(1);
      pp_render.composite_desktop(mGraphics, 0);  // texture[0]: left
      glDrawBuffer(GL_BACK_RIGHT);
      gl::clearColor(0, 0, 0);
      gl::clearDepth(1);
      pp_render.composite_desktop(mGraphics, 1);  // texture[1]: right
    } else {  // rendering mono on display other than sphere
      // std::cout << "sampling mono on flat display" << std::endl;
      glDrawBuffer(GL_BACK_LEFT);
      gl::clearColor(0.2, 0.2, 0.2);
      gl::clearDepth(1);
      pp_render.composite_desktop(
          mGraphics,
          (eye_to_render == 1) ? 1 : 0  // mono and left eye is
                                        // rendered on texture[0],
                                        // right eye is on texture[1]
      );
    }
  }
  mGraphics.popViewport();
  // put back default drawbuffer
  glDrawBuffer(GL_BACK_LEFT);
}

}  // namespace al

#endif
