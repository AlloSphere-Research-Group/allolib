#ifndef INCLUDE_OMNIRENDERAPP_HPP
#define INCLUDE_OMNIRENDERAPP_HPP

#include "al/sphere/al_Perprojection.hpp"
#include "al/sphere/al_SphereUtils.hpp"

#include "al/core/app/al_WindowApp.hpp"
#include "al/core/graphics/al_Graphics.hpp"
#include "al/core/graphics/al_GLFW.hpp"

#include <iostream>

namespace al {

struct OmniRendererer : WindowApp
{
    Graphics mGraphics;
    PerProjectionRender pp_render;

    OmniRendererer()
        : WindowApp() // appends standard window controls and itself as event handler
    {

    }

    // virtual void pre_process() {}
    virtual void onDraw(Graphics& g) {}
    // virtual void post_process() {}
    // virtual void overlay_process() {}

    // void sample_capture() {}
    // void render_capture() {}

    void open() override
    {
        glfw::init();
        if (sphere::is_renderer()) {
            int width, height;
            sphere::get_fullscreen_dimension(&width, &height);
            if (width != 0 && height != 0) {
                // std::cout << "width: " << width << " height: " << height << std::endl;
                dimensions(0, 0, width, height);
                decorated(false);
            }
            else {
                std::cout << "[!] in sphere renderer but calculated width and/or height are/is zero!" << std::endl;
            }
        }
        create();
        cursorHide(true);
        
        pp_render.load_calibration_data(
            sphere::config_directory("data").c_str(),   // path
            sphere::renderer_hostname("config").c_str() // hostname
        ); // parameters will be used to look for file ${path}/${hostname}.txt

        pp_render.init(1024);
        mGraphics.init();
        onCreate();
    }

    void onCreate() override {}

    virtual void onAnimate(double dt) {}

    void onDraw() override
    {
        onAnimate(dt() / 1000000.0f); // millis for dt

        // pre_process(); // preparation before capturing
        
        mGraphics.omni(true);
        pp_render.begin(mGraphics);
        for (int eye = 0; eye < pp_render.num_eyes(); eye += 1) { // 2 for stereo (TODO!)
            // pp_render.set_eye(eye);
            pp_render.set_eye(-1);
            for (int i = 0; i < pp_render.num_projections(); i++) {
                pp_render.set_projection(i);
                onDraw(mGraphics);
            }
        }
        pp_render.end();
        mGraphics.omni(false);
        
        mGraphics.viewport(0, 0, fbWidth(), fbHeight());
        mGraphics.clearColor(0, 0, 0);
        mGraphics.clearDepth(1);
        pp_render.composite(mGraphics);

        /*
        sample_capture();
        post_process(); // post process sampled result
        render_capture();
        overlay_process(); // overlay something on top of result?
        */
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

struct MyOmniRendererApp : OmniRendererer
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
