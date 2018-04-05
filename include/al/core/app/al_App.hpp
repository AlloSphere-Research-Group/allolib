#ifndef INCLUDE_AL_APP_HPP
#define INCLUDE_AL_APP_HPP

/*  Keehong Youn, 2017, younkeehong@gmail.com
*/

#include "al/core/app/al_WindowApp.hpp"
#include "al/core/app/al_AudioApp.hpp"
#include "al/core/protocol/al_OSC.hpp"

#include "al/core/graphics/al_GLFW.hpp"
#include "al/core/io/al_Window.hpp"
#include "al/core/graphics/al_Graphics.hpp"
#include "al/core/io/al_ControlNav.hpp"

#include "al/util/al_DeviceServerApp.hpp"

#include <iostream>

namespace al {

// single window, audioIO, and single port osc recv & send
class App: public WindowApp, public AudioApp, /*public DeviceServerApp,*/ public osc::PacketHandler {
public:

    class AppEventHandler : public WindowEventHandler {
    public:
        App* app;
        AppEventHandler(App* a): app(a) {}
        bool resize (int dw, int dh) override {
            app->mViewport.set(0, 0, app->fbWidth(), app->fbHeight());
            return true;
        }
    };

    Graphics mGraphics;
    AppEventHandler eventHandler {this};
    Nav mNav;
    Viewpoint mView {mNav.transformed()};
    NavInputControl mNavControl {mNav};
    Viewport mViewport;

    Viewpoint& view() { return mView; }
    const Viewpoint& view() const { return mView; }

    // Nav& nav() override { return mNav; }
    Nav& nav() { return mNav; }
    const Nav& nav() const { return mNav; }

    Pose& pose() { return mNav; }
    const Pose& pose() const { return mNav; }

    NavInputControl& navControl() { return mNavControl; }
    NavInputControl const& navControl() const { return mNavControl; }

    Lens& lens() { return mView.lens(); }
    Lens const& lens() const { return mView.lens(); }

    Graphics& graphics() { return mGraphics; }

    Viewport const& viewport() { return mViewport; }

    virtual void onInit() {}
    virtual void onAnimate(double dt) {}
    virtual void onExit() {}

    virtual void preOnCreate() {
        append(mNavControl);
        append(eventHandler);
        mGraphics.init();
        mViewport.set(0, 0, fbWidth(), fbHeight());
    }

    virtual void preOnAnimate(double dt) {
        mNav.step();
    }

    virtual void preOnDraw() {
        mGraphics.framebuffer(FBO::DEFAULT);
        mGraphics.viewport(mViewport);
        mGraphics.resetMatrixStack();
        mGraphics.camera(mView);
        mGraphics.color(1, 1, 1);
    }

    virtual void onDraw (Graphics& g) {}

    virtual void postOnDraw() {}

    void onDraw () override {
        onDraw(mGraphics);
    }

    // from WindowApp
    void open() override {
        glfw::init(is_verbose);
        onInit();
        Window::create(is_verbose);
        preOnCreate();
        onCreate();
    }

    // from WindowApp
    void loop() override {
        preOnAnimate(dt_sec());
        onAnimate(dt_sec());
        preOnDraw();
        onDraw();
        postOnDraw();
        refresh(); // Window
    }

    // overrides WindowApp's start to also initiate AudioApp and etc.
    void start() override {
        open();
        beginAudio(); // only begins if `initAudio` was called before
        startFPS(); // WindowApp (FPS)
        // initDeviceServer();
        while (!shouldQuit()) {
            // to quit, call WindowApp::quit() or click close button of window,
            // or press ctrl + q
            loop(); // WindowApp (onDraw, refresh)
            tickFPS(); // WindowApp (FPS)
        }
        onExit(); // user defined
        endAudio(); // AudioApp
        closeApp(); // WindowApp
    }

    // PacketHandler
    void onMessage(osc::Message& m) override {}
};

}

#endif