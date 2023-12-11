#ifndef INCLUDE_OMNIRENDER_HPP
#define INCLUDE_OMNIRENDER_HPP

#include <iostream>

#include "al/app/al_OpenGLGraphicsDomain.hpp"
#include "al/graphics/al_Graphics.hpp"
#include "al/sphere/al_PerProjection.hpp"
#include "al/sphere/al_SphereUtils.hpp"

namespace al {

/**
 * @brief GLFWOpenGLOmniRendererDomain class
 * @ingroup App
 *
 * This domain can only be instantiated as a subdomain of OpenGLGraphicsDomain
 */
class GLFWOpenGLOmniRendererDomain : public SynchronousDomain {
public:
  GLFWOpenGLOmniRendererDomain();
  // Domain functions
  bool init(ComputationDomain *parent = nullptr) override;

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
  void stereo(bool b);

  void toggleStereo() { stereo(!render_stereo); }
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
  int eye_to_render = -1; // -1 for mono, 0: left, 1: right

  /// Set this to false to render a regular view instead of omni
  bool drawOmni{true};
  Lens mLens;
  //  Pose mPose;

private:
  std::unique_ptr<Window> mWindow;

  std::unique_ptr<Graphics> mGraphics;

  OpenGLGraphicsDomain *mParent;

  Nav mNav; // is a Pose itself and also handles manipulation of pose
  Viewpoint mView{mNav.transformed()}; // Pose with Lens and acts as camera
  NavInputControl mNavControl{mNav};   // interaction with keyboard and mouse
};
} // namespace al

#endif
