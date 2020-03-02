#include "al/app/al_OmniRendererDomain.hpp"

#include <cstring>

using namespace al;

GLFWOpenGLOmniRendererDomain::GLFWOpenGLOmniRendererDomain() {
  mWindow = std::make_unique<Window>();

  mWindow->append(navControl());
  mGraphics = std::make_unique<Graphics>();
}

bool GLFWOpenGLOmniRendererDomain::init(al::ComputationDomain *parent) {
  //  if (strcmp(typeid(*parent).name(), typeid(OpenGLGraphicsDomain).name()) ==
  //  0) {
  //    mGraphics = &static_cast<OpenGLGraphicsDomain *>(parent)->graphics();
  //  }
  bool ret = true;
  ret &= initializeSubdomains(true);
  assert(strcmp(typeid(*parent).name(), typeid(OpenGLGraphicsDomain).name()) ==
         0);
  mParent = static_cast<OpenGLGraphicsDomain *>(parent);
  if (!mWindow) {
    mWindow = std::make_unique<Window>();
  }
  if (!mGraphics) {
    mGraphics = std::make_unique<Graphics>();
  }
  if (!mWindow->created()) {
    if (render_stereo) {
      mWindow->displayMode(mWindow->displayMode() |
                           Window::DisplayMode::STEREO_BUF);
    }
    ret = mWindow->create();
    window_is_stereo_buffered =
        mWindow->displayMode() & Window::DisplayMode::STEREO_BUF;
    if (ret) {
      mGraphics->init();
    }
  }

  if (sphere::isRendererMachine()) {
    spanAllDesktop();
    loadPerProjectionConfiguration(false);
    running_in_sphere_renderer = true;
  } else {
    loadPerProjectionConfiguration();
    running_in_sphere_renderer = false;
  }
  ret &= initializeSubdomains(false);
  return ret;
}

bool GLFWOpenGLOmniRendererDomain::tick() {
  if (mWindow->shouldClose()) {
    return false;
  }
  /* Make the window's context current */
  onNewFrame();
  mWindow->makeCurrent();
  preOnDraw();
  if (drawOmni) {
    drawUsingPerProjectionCapture();
  } else {
    onDraw(*mGraphics);
  }
  postOnDraw();
  mWindow->refresh();
  return true;
}

bool GLFWOpenGLOmniRendererDomain::cleanup(ComputationDomain *parent) {
  if (mWindow) {
    mWindow->destroy();
    mWindow = nullptr;
  }
  if (mGraphics) {
    mGraphics = nullptr;
  }
  return true;
}

void GLFWOpenGLOmniRendererDomain::stereo(bool b) {
  if (mWindow && mWindow->enabled(Window::DisplayMode::STEREO_BUF) != b) {
    if (b) {
      mWindow->displayMode(Window::DisplayMode::DEFAULT_BUF |
                           Window::DisplayMode::STEREO_BUF);
    } else {
      mWindow->displayMode(Window::DisplayMode::DEFAULT_BUF);
    }
  }
  render_stereo = b;
}

void GLFWOpenGLOmniRendererDomain::loopEyeForDesktopMode() {
  eye_to_render += 1;
  if (eye_to_render > 1) eye_to_render = -1;
}

void GLFWOpenGLOmniRendererDomain::setEyeToRenderForDesktopMode(int eye) {
  eye_to_render = eye;
  if (eye_to_render < -1 || eye_to_render > 1) eye_to_render = -1;
}

void GLFWOpenGLOmniRendererDomain::spanAllDesktop() {
  int width, height;
  sphere::getFullscreenDimension(&width, &height);
  if (width != 0 && height != 0) {
    mWindow->dimensions(0, 0, width, height);
    mWindow->decorated(false);
  } else {
    std::cout << "[!] in sphere renderer but calculated "
              << "width and/or height are/is zero!" << std::endl;
  }
}

void GLFWOpenGLOmniRendererDomain::loadPerProjectionConfiguration(
    bool desktop) {
  if (!desktop) {
    // need to be called before pp_render.init
    pp_render.load_calibration_data(
        sphere::getCalibrationDirectory("data").c_str(),  // path
        sphere::renderer_hostname("config").c_str()       // hostname
    );  // parameters will be used to look for file ${path}/${hostname}.txt
    pp_render.init(mGraphics->lens());
  } else {
    // load fake projection data for desktop rendering
    pp_render.load_and_init_as_desktop_config(mGraphics->lens());
  }
}

void GLFWOpenGLOmniRendererDomain::drawUsingPerProjectionCapture() {
  // start drawing to perprojection fbos
  //  std::cout << "hhh" << std::endl;

  mGraphics->omni(true);  // set true to use omni default shaders when drawing
  // lens and pose for rendering is set in PerProjectionRender::begin
  // so updating those in onDraw will not have effect in rendering
  // will setting up omni rendering,
  // begin also pushes fbo, viewport, viewmat, projmat, lens, shader
  pp_render.begin(*mGraphics, mGraphics->lens(), mView.pose());
  glDrawBuffer(GL_COLOR_ATTACHMENT0);  // for fbo's output
  if (render_stereo) {
    for (int eye = 0; eye < 2; eye += 1) {
      pp_render.set_eye(eye);
      for (int i = 0; i < pp_render.num_projections(); i++) {
        pp_render.set_projection(i);
        gl::depthTesting(true);
        gl::depthMask(true);
        gl::blending(false);
        onDraw(*mGraphics);
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
      onDraw(*mGraphics);
    }
  }
  pp_render.end();  // pops everything pushed before

  /* Settings for warp and blend composition sampling
   */
  mGraphics->omni(false);  // warp and blend composition done on flat rendering
  mGraphics->eye(Graphics::MONO_EYE);  // stereo handled at capture stage
  gl::polygonMode(GL_FILL);            // to draw viewport filling quad
  gl::blending(false);                 // blending already done when capturing
  gl::depthTesting(false);  // no depth testing when drawing viewport slab
  mGraphics->pushViewport(0, 0, mWindow->fbWidth(),
                          mWindow->fbHeight());  // filling the whole window

  // now sample the results
  if (running_in_sphere_renderer) {
    if (window_is_stereo_buffered) {
      // rendering stereo in sphere
      glDrawBuffer(GL_BACK_LEFT);
      gl::clearColor(0, 0, 0);
      gl::clearDepth(1);
      pp_render.composite(*mGraphics, 0);
      glDrawBuffer(GL_BACK_RIGHT);
      gl::clearColor(0, 0, 0);
      gl::clearDepth(1);
      pp_render.composite(*mGraphics, 1);
    } else {  // rendering mono in sphere
      // std::cout << "sampling mono in sphere setup" << std::endl;
      glDrawBuffer(GL_BACK_LEFT);
      gl::clearColor(1, 0, 0);
      gl::clearDepth(1);
      pp_render.composite(*mGraphics, (eye_to_render == 1) ? 1 : 0);
    }
  } else {
    if (window_is_stereo_buffered) {
      // rendering stereo on display other than sphere
      glDrawBuffer(GL_BACK_LEFT);
      gl::clearColor(0, 0, 0);
      gl::clearDepth(1);
      pp_render.composite_desktop(*mGraphics, 0);  // texture[0]: left
      glDrawBuffer(GL_BACK_RIGHT);
      gl::clearColor(0, 0, 0);
      gl::clearDepth(1);
      pp_render.composite_desktop(*mGraphics, 1);  // texture[1]: right
    } else {  // rendering mono on display other than sphere
      // std::cout << "sampling mono on flat display" << std::endl;
      glDrawBuffer(GL_BACK_LEFT);
      gl::clearColor(0.2, 0.2, 0.2);
      gl::clearDepth(1);
      pp_render.composite_desktop(
          *mGraphics,
          (eye_to_render == 1) ? 1 : 0  // mono and left eye is
                                        // rendered on texture[0],
                                        // right eye is on texture[1]
      );
    }
  }
  mGraphics->popViewport();
  // put back default drawbuffer
  glDrawBuffer(GL_BACK_LEFT);
}
