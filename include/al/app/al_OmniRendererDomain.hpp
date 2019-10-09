#ifndef INCLUDE_OMNIRENDER_HPP
#define INCLUDE_OMNIRENDER_HPP

#include "al/sphere/al_Perprojection.hpp"
#include "al/sphere/al_SphereUtils.hpp"

#include "al/app/al_OpenGLGraphicsDomain.hpp"
//#include "al/graphics/al_GLFW.hpp"
#include "al/graphics/al_Graphics.hpp"

#include <iostream>

namespace al {

/**
 * @brief GLFWOpenGLOmniRendererDomain class
 * @ingroup App
 */
class GLFWOpenGLOmniRendererDomain : public SynchronousDomain {
 public:
  GLFWOpenGLOmniRendererDomain();
  // Domain functions
  bool initialize(ComputationDomain *parent = nullptr) override;

  bool tick() override;

  bool cleanup(ComputationDomain *parent = nullptr) override;

  std::function<void()> preOnDraw = [this]() {
    mGraphics->framebuffer(FBO::DEFAULT);
    mGraphics->viewport(0, 0, mWindow->fbWidth(), mWindow->fbHeight());
    mGraphics->resetMatrixStack();
    mGraphics->camera(mView);
    mGraphics->color(1, 1, 1);
  };

  virtual void onNewFrame() {
    mNav.smooth(std::pow(0.0001, mTimeDrift));
    mNav.step(mTimeDrift * mParent->fps());
  }

  std::function<void(Graphics &)> onDraw = [](Graphics &) {};

  std::function<void()> postOnDraw = []() {};

  Viewpoint &view() { return mView; }
  const Viewpoint &view() const { return mView; }

  Nav &nav() { return mNav; }
  // Nav& nav() { return mNav; }
  const Nav &nav() const { return mNav; }

  NavInputControl &navControl() { return mNavControl; }

  Window &window() { return *mWindow; }

  Graphics &graphics() { return *mGraphics; }

  // omni-stereo related functions
  void stereo(bool b) { render_stereo = b; }
  void toggleStereo() { render_stereo = !render_stereo; }
  void omniResolution(int res) { pp_render.update_resolution(res); }
  int omniResolution() { return pp_render.res_; }

  // only for testing with desktop mode, loops (mono -> left -> right)
  void loopEyeForDesktopMode();

  // only for testing with desktop mode, -1 for mono, 0: left, 1: right
  void setEyeToRenderForDesktopMode(int eye);

  // omni functionality, impl below out-of-line
  void spanAllDesktop();
  void loadPerProjectionConfiguration(bool desktop = true);
  void drawUsingPerProjectionCapture();

  // OmniRenderer
  PerProjectionRender pp_render;
  bool render_stereo = true;
  bool running_in_sphere_renderer = false;
  bool window_is_stereo_buffered = false;
  int eye_to_render = -1;  // -1 for mono, 0: left, 1: right

  bool drawOmni{true};
  Lens mLens;
  //  Pose mPose;

 private:
  std::unique_ptr<Window> mWindow;

  std::unique_ptr<Graphics> mGraphics;

  OpenGLGraphicsDomain *mParent;

  Nav mNav;  // is a Pose itself and also handles manipulation of pose
  Viewpoint mView{mNav.transformed()};  // Pose with Lens and acts as camera
  NavInputControl mNavControl{mNav};    // interaction with keyboard and mouse
};

// inline void GLFWOpenGLOmniRendererDomain::start() {
//  initializeWindowManager();
//  onInit();
//  check_if_in_sphere_and_setup_window_dimensions();
//  Window::create(is_verbose);
//  window_is_stereo_buffered = Window::displayMode() & Window::STEREO_BUF;
//  mGraphics->init();
//  load_perprojection_configuration();
//  cursorHide(true);
//  onCreate();
//  FPS::startFPS();
//  while (!shouldQuit()) {
//    onAnimate(dt_sec()); // millis for dt
//    draw_using_perprojection_capture();
//    Window::refresh();
//    FPS::tickFPS();
//  }
//  onExit();
//  Window::destroy();
//  terminateWindowManager();
//}

// GLFWOpenGLOmniRendererDomain::GLFWOpenGLOmniRendererDomain() {
//  mWindow = std::make_unique<Window>();

//  mWindow->append(navControl());
//  mGraphics = std::make_unique<Graphics>();
//}

// bool GLFWOpenGLOmniRendererDomain::initialize(ComputationDomain *parent) {
//  //  if (strcmp(typeid(*parent).name(), typeid(OpenGLGraphicsDomain).name())
//  ==
//  //  0) {
//  //    mGraphics = &static_cast<OpenGLGraphicsDomain *>(parent)->graphics();
//  //  }
//  assert(strcmp(typeid(*parent).name(), typeid(OpenGLGraphicsDomain).name())
//  ==
//         0);
//  mParent = static_cast<OpenGLGraphicsDomain *>(parent);
//  if (!mWindow) {
//    mWindow = std::make_unique<Window>();
//  }
//  if (!mGraphics) {
//    mGraphics = std::make_unique<Graphics>();
//  }
//  if (!mWindow->created()) {
//    bool ret = mWindow->create();
//    if (ret) {
//        mGraphics->init();
//    }
//  }

//  if (sphere::is_renderer()) {
//    spanAllDesktop();
//    loadPerProjectionConfiguration(false);
//    running_in_sphere_renderer = true;
//  } else {
//    loadPerProjectionConfiguration();
//    running_in_sphere_renderer = false;
//  }
//  return true;
//}

// bool GLFWOpenGLOmniRendererDomain::tick() {
//  if (mWindow->shouldClose()) {
//    return false;
//  }
//  /* Make the window's context current */
//  onNewFrame();
//  mWindow->makeCurrent();
//  preOnDraw();
//  drawUsingPerProjectionCapture();
//  postOnDraw();
//  mWindow->refresh();
//  return true;
//}

// bool GLFWOpenGLOmniRendererDomain::cleanup(ComputationDomain *parent) {
//  if (mWindow) {
//    mWindow->destroy();
//    mWindow = nullptr;
//  }
//  if (mGraphics) {
//    mGraphics = nullptr;
//  }
//  return true;
//}

// inline void GLFWOpenGLOmniRendererDomain::spanAllDesktop() {
//    int width, height;
//    sphere::get_fullscreen_dimension(&width, &height);
//    if (width != 0 && height != 0) {
//      mWindow->dimensions(0, 0, width, height);
//      mWindow->decorated(false);
//    } else {
//      std::cout << "[!] in sphere renderer but calculated "
//                << "width and/or height are/is zero!" << std::endl;
//    }
//}

// inline void GLFWOpenGLOmniRendererDomain::loadPerProjectionConfiguration(bool
// desktop) {
//  if (!desktop) {
//    // need to be called before pp_render.init
//    pp_render.load_calibration_data(
//        sphere::config_directory("data").c_str(),   // path
//        sphere::renderer_hostname("config").c_str() // hostname
//    ); // parameters will be used to look for file ${path}/${hostname}.txt
//    pp_render.init(mGraphics->lens());
//  } else {
//    // load fake projection data for desktop rendering
//    pp_render.load_and_init_as_desktop_config(mGraphics->lens());
//  }
//}

// inline void GLFWOpenGLOmniRendererDomain::drawUsingPerProjectionCapture() {

//  // start drawing to perprojection fbos
////  std::cout << "hhh" << std::endl;

//  mGraphics->omni(true); // set true to use omni default shaders when drawing
//  // lens and pose for rendering is set in PerProjectionRender::begin
//  // so updating those in onDraw will not have effect in rendering
//  // will setting up omni rendering,
//  // begin also pushes fbo, viewport, viewmat, projmat, lens, shader
//  pp_render.begin(*mGraphics, mGraphics->lens(), mPose);
//  glDrawBuffer(GL_COLOR_ATTACHMENT0); // for fbo's output
//  if (render_stereo) {
//    for (int eye = 0; eye < 2; eye += 1) {
//      pp_render.set_eye(eye);
//      for (int i = 0; i < pp_render.num_projections(); i++) {
//        pp_render.set_projection(i);
//        mGraphics->depthTesting(true);
//        mGraphics->depthMask(true);
//        mGraphics->blending(false);
//        onDraw(*mGraphics);
//      }
//    }
//  } else {
//    // std::cout << "rendering eye " << eye_to_render << std::endl;
//    pp_render.set_eye(eye_to_render);
//    for (int i = 0; i < pp_render.num_projections(); i++) {
//      pp_render.set_projection(i);
//      mGraphics->depthTesting(true);
//      mGraphics->depthMask(true);
//      mGraphics->blending(false);
//      onDraw(*mGraphics);
//    }
//  }
//  pp_render.end(); // pops everything pushed before

//  /* Settings for warp and blend composition sampling
//  */
//  mGraphics->omni(false); // warp and blend composition done on flat rendering
//  mGraphics->eye(Graphics::MONO_EYE);     // stereo handled at capture stage
//  mGraphics->polygonMode(Graphics::FILL); // to draw viewport filling quad
//  mGraphics->blending(false);     // blending already done when capturing
//  mGraphics->depthTesting(false); // no depth testing when drawing viewport
//  slab mGraphics->pushViewport(0, 0, mWindow->fbWidth(), mWindow->fbHeight());
//  // filling the whole window

//  // now sample the results
//  if (running_in_sphere_renderer) {
//    if (window_is_stereo_buffered) {
//      // rendering stereo in sphere
//      glDrawBuffer(GL_BACK_LEFT);
//      mGraphics->clearColor(0, 0, 0);
//      mGraphics->clearDepth(1);
//      pp_render.composite(*mGraphics, 0);
//      glDrawBuffer(GL_BACK_RIGHT);
//      mGraphics->clearColor(0, 0, 0);
//      mGraphics->clearDepth(1);
//      pp_render.composite(*mGraphics, 1);
//    } else { // rendering mono in sphere
//      // std::cout << "sampling mono in sphere setup" << std::endl;
//      glDrawBuffer(GL_BACK_LEFT);
//      mGraphics->clearColor(1, 0, 0);
//      mGraphics->clearDepth(1);
//      pp_render.composite(*mGraphics, (eye_to_render == 1) ? 1 : 0);
//    }
//  } else {
//    if (window_is_stereo_buffered) {
//      // rendering stereo on display other than sphere
//      glDrawBuffer(GL_BACK_LEFT);
//      mGraphics->clearColor(0, 0, 0);
//      mGraphics->clearDepth(1);
//      pp_render.composite_desktop(*mGraphics, 0); // texture[0]: left
//      glDrawBuffer(GL_BACK_RIGHT);
//      mGraphics->clearColor(0, 0, 0);
//      mGraphics->clearDepth(1);
//      pp_render.composite_desktop(*mGraphics, 1); // texture[1]: right
//    } else { // rendering mono on display other than sphere
//      // std::cout << "sampling mono on flat display" << std::endl;
//      glDrawBuffer(GL_BACK_LEFT);
//      mGraphics->clearColor(0.2, 0.2, 0.2);
//      mGraphics->clearDepth(1);
//      pp_render.composite_desktop(
//          *mGraphics,
//          (eye_to_render == 1) ? 1 : 0 // mono and left eye is
//                                       // rendered on texture[0],
//                                       // right eye is on texture[1]
//      );
//    }
//  }
//  mGraphics->popViewport();
//  // put back default drawbuffer
//  glDrawBuffer(GL_BACK_LEFT);
//}
//=======

//};
//>>>>>>> Stashed changes

}  // namespace al

#endif
