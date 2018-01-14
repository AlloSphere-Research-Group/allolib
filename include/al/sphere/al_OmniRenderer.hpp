#ifndef INCLUDE_OMNIRENDER_HPP
#define INCLUDE_OMNIRENDER_HPP

#include "al/sphere/al_Perprojection.hpp"
#include "al/sphere/al_SphereUtils.hpp"

#include "al/core/app/al_WindowApp.hpp"
#include "al/core/graphics/al_Graphics.hpp"
#include "al/core/graphics/al_GLFW.hpp"

#include <iostream>

namespace al {

struct OmniRenderer : WindowApp
{
    Graphics mGraphics;
    PerProjectionRender pp_render;
    bool render_stereo = true;
    bool running_in_sphere_renderer = false;
    bool window_is_stereo_buffered = false;
    int eye_to_render = -1; // -1 for mono, 0: left, 1: right

    OmniRenderer()
        : WindowApp() // appends std window controls and itself as event handler
    {}

    void stereo(bool b) { render_stereo = b; }
    void toggleStereo() { render_stereo = !render_stereo; }

    void omniResolution(int res) { pp_render.update_resolution(res); }
    void sphereRadius(float radius) { pp_render.sphereRadius(radius); }

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

    Lens& lens() { return pp_render.lens_; }
    Lens const& lens() const { return pp_render.lens_; }

    void pose(Pose const& p) { pp_render.pose(p); }
    Pose& pose() { return pp_render.pose(); }
    Pose const& pose() const { return pp_render.pose(); }

    void open() override
    {
        glfw::init();
        if (sphere::is_renderer()) running_in_sphere_renderer = true;

        if (running_in_sphere_renderer) {
            int width, height;
            sphere::get_fullscreen_dimension(&width, &height);
            if (width != 0 && height != 0) {
                dimensions(0, 0, width, height);
                decorated(false);
            }
            else {
                std::cout << "[!] in sphere renderer but calculated "
                          << "width and/or height are/is zero!" << std::endl;
            }
        }
        create();
        cursorHide(true);
        window_is_stereo_buffered = Window::displayMode() & Window::STEREO_BUF;

        if (running_in_sphere_renderer) {
            // need to be called before pp_render.init
            pp_render.load_calibration_data(
                sphere::config_directory("data").c_str(),   // path
                sphere::renderer_hostname("config").c_str() // hostname
            ); // parameters will be used to look for file ${path}/${hostname}.txt
            pp_render.init(1024);
        }
        else {
            // load fake projection data for desktop rendering
            pp_render.load_and_init_as_desktop_config();
        }
        mGraphics.init();
        onCreate();
    }

    // for user to override
    void onCreate() override {} 
    virtual void onAnimate(double dt) {}
    virtual void onDraw(Graphics& g) {}


    void onDraw() override
    {
        onAnimate(dt() / 1000000.0f); // millis for dt

        // start drawing to perprojection fbos
        mGraphics.omni(true);
        pp_render.begin(mGraphics);
        glDrawBuffer(GL_COLOR_ATTACHMENT0); // for fbo's output
        if (window_is_stereo_buffered && render_stereo) {
            for (int eye = 0; eye < 2; eye += 1) {
                pp_render.set_eye(eye);
                for (int i = 0; i < pp_render.num_projections(); i++) {
                    pp_render.set_projection(i);
                    onDraw(mGraphics);
                }
            }
        }
        else {
            pp_render.set_eye(eye_to_render);
            for (int i = 0; i < pp_render.num_projections(); i++) {
                pp_render.set_projection(i);
                onDraw(mGraphics);
            }
        }
        pp_render.end();
        mGraphics.omni(false);

        // now sample the results
        mGraphics.viewport(0, 0, fbWidth(), fbHeight());
        mGraphics.pushViewport();

        // no stereo when actually displaying sampled result
        mGraphics.eye(Graphics::MONO_EYE);

        if (running_in_sphere_renderer) {
            if (window_is_stereo_buffered && render_stereo) {
            // rendering stereo in sphere
                glDrawBuffer(GL_BACK_LEFT);
                mGraphics.clearColor(0, 0, 0);
                mGraphics.clearDepth(1);
                pp_render.composite(mGraphics, 0);
                glDrawBuffer(GL_BACK_RIGHT);
                mGraphics.clearColor(0, 0, 0);
                mGraphics.clearDepth(1);
                pp_render.composite(mGraphics, 1);
            }
            else { // rendering mono in sphere
                glDrawBuffer(GL_BACK_LEFT);
                mGraphics.clearColor(0, 0, 0);
                mGraphics.clearDepth(1);
                pp_render.composite(mGraphics, 0);
            }
        }
        else {
            if (window_is_stereo_buffered && render_stereo) {
            // rendering stereo on display other than sphere
                glDrawBuffer(GL_BACK_LEFT);
                mGraphics.clearColor(0, 0, 0);
                mGraphics.clearDepth(1);
                pp_render.composite_desktop(mGraphics, 0); // texture[0]: left
                glDrawBuffer(GL_BACK_RIGHT);
                mGraphics.clearColor(0, 0, 0);
                mGraphics.clearDepth(1);
                pp_render.composite_desktop(mGraphics, 1); // texture[1]: right
            }
            else { // rendering mono on display other than sphere
                glDrawBuffer(GL_BACK_LEFT);
                mGraphics.clearColor(0.2, 0.2, 0.2);
                mGraphics.clearDepth(1);
                pp_render.composite_desktop(mGraphics,
                    (eye_to_render == 1)? 1:0 // mono and left eye is
                                              // rendered on texture[0],
                                              // right eye is on texture[1]
                );
            }
        }

        mGraphics.popViewport();
        // put back default drawbuffer
        glDrawBuffer(GL_BACK);
    }

    void onKeyDown(Keyboard const& k) override {}
    void onKeyUp(Keyboard const& k) override {}
    void onMouseDown(Mouse const& m) override {}
    void onMouseUp(Mouse const& m) override {}
    void onMouseDrag(Mouse const& m) override {}
    void onMouseMove(Mouse const& m) override {}
    void onResize(int w, int h) override {}
    void onVisibility(bool v) override {}
};

}

#endif

/* EXAMPLE

using namespace std;
using namespace al;

struct MyOmniRendererApp : OmniRenderer
{
    VAOMesh mesh;
    
    // wouldn't need when receiving pose from simulator
    Nav mNav;
    Viewpoint mView {mNav.transformed()};
    NavInputControl mNavControl {mNav};

    void onCreate() override
    {
        append(mNavControl);

        addIcosahedron(mesh);
        int num_verts = mesh.vertices().size();
        for (int i = 0; i < num_verts; i++) {
          mesh.color(i / float(num_verts), (num_verts - i) / float(num_verts), 0.0);
        }
        mesh.update();
    }

    void onAnimate(double dt) override
    {
        mNav.step();
        pp_render.pose(mView.pose()); // should not be in onDraw
    }

    void onDraw(Graphics& g) override
    {
        g.clear(0);
        // g.meshColor();

        for(int aa = -5; aa <= 5; aa++)
        for(int bb = -5; bb <= 5; bb++)
        for(int cc = -5; cc <= 5; cc++)  {
            if(aa == 0 && bb == 0 && cc == 0) continue;
            g.pushMatrix();
            g.translate(aa * 2, bb * 2, cc * 2);
            g.rotate(sin(2 * sec()), 0, 0, 1);
            g.rotate(sin(3 * sec()), 0, 1, 0);
            g.scale(0.3, 0.3, 0.3);
            g.color((aa + 5)/10.0, (bb + 5)/10.0, (cc + 5)/10.0);
            g.draw(mesh);
            g.popMatrix();
        }
    }
};

int main()
{
    MyOmniRendererApp app;
    app.start();
}

*/
