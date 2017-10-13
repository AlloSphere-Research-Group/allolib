#ifndef INCLUDE_AL_APP_HPP
#define INCLUDE_AL_APP_HPP

/*  Keehong Youn, 2017, younkeehong@gmail.com
*/

#include "al/core/app/al_WindowApp.hpp"
#include "al/core/app/al_AudioApp.hpp"
#include "al/core/protocol/al_OSC.hpp"

#include "al/core/gl/al_GLFW.hpp"
#include "al/core/gl/al_Graphics.hpp"
#include "al/core/gl/al_Viewpoint.hpp"
#include "al/core/spatial/al_Pose.hpp"
#include "al/core/io/al_ControlNav.hpp"
#include "al/core/gl/al_DefaultShaders.hpp"
#include "al/core/types/al_Color.hpp"

#include <iostream>

namespace al {

class AppGraphics : public Graphics
{
public:
    ShaderProgram mesh_shader;
    ShaderProgram color_shader;
    ShaderProgram tex_shader;
    int color_location = 0;
    int color_tint_location = 0;
    int tex_tint_location = 0;
    int mesh_tint_location = 0;
    int tint_location = 0;
    al::Color tint_color {1.0f, 1.0f, 1.0f, 1.0f};
    al::Texture* texPtr = nullptr;
    
    AppGraphics (Window* win) : Graphics {win} {}

    void init ()
    {
        compileDefaultShader(mesh_shader, ShaderType::MESH);
        compileDefaultShader(color_shader, ShaderType::COLOR);
        compileDefaultShader(tex_shader, ShaderType::TEXTURE);
        color_location = color_shader.getUniformLocation("col0");
        color_tint_location = color_shader.getUniformLocation("tint");
        tex_tint_location = tex_shader.getUniformLocation("tint");
        mesh_tint_location = mesh_shader.getUniformLocation("tint");
        tex_shader.begin();
        tex_shader.uniform("tex0", 0);
        tex_shader.uniform(tex_tint_location, 1, 1, 1, 1);
        tex_shader.end();
        color_shader.begin();
        color_shader.uniform(color_tint_location, 1, 1, 1, 1);
        color_shader.end();
        mesh_shader.begin();
        mesh_shader.uniform(mesh_tint_location, 1, 1, 1, 1);
        mesh_shader.end();
        shader(color_shader);
        tint_location = color_tint_location;
    }

    void tint(float r, float g, float b, float a = 1.0f) {
        tint_color.set(r, g, b, a);
        shader().uniform4v(tint_location, tint_color.components);
    }

    void tint(float grayscale, float a = 1.0f) {
        tint(grayscale, grayscale, grayscale, a);
    }

    void color (float r, float g, float b, float a = 1.0f)
    {
        if (shader().id() != color_shader.id()) {
            shader(color_shader);
            tint_location = color_tint_location;
        }
        shader().uniform(color_location, r, g, b, a);
        shader().uniform4v(color_tint_location, tint_color.components);
    }

    void color (float k, float a = 1.0f) { color(k, k, k, a); }
    void color (Color const& c) { color(c.r, c.g, c.b, c.a); }

    void bind(Texture& t) {
        if (shader().id() != tex_shader.id()) {
            shader(tex_shader);
            tint_location = tex_tint_location;
        }
        shader().uniform4v(tex_tint_location, tint_color.components);
        t.bind(0);
        texPtr = &t;
    }
    void unbind() {
        texPtr->unbind(0);
        texPtr = nullptr;
    }

    void meshColor() {
        if (shader().id() != mesh_shader.id()) {
            shader(mesh_shader);
            tint_location = mesh_tint_location;
        }
        shader().uniform4v(mesh_tint_location, tint_color.components);
    }

    void quad(Texture& tex, float x, float y, float w, float h)
    {
        static Mesh m = [] () {
            Mesh m {Mesh::TRIANGLE_STRIP};
            m.vertex(0, 0, 0); m.vertex(0, 0, 0);
            m.vertex(0, 0, 0); m.vertex(0, 0, 0);
            m.texCoord(0, 0);
            m.texCoord(1, 0);
            m.texCoord(0, 1);
            m.texCoord(1, 1);
            return m;
        }();

        auto& verts = m.vertices();
        verts[0].set(x, y, 0);
        verts[1].set(x + w, y, 0);
        verts[2].set(x, y + h, 0);
        verts[3].set(x + w, y + h, 0);

        bind(tex);
        draw(m);
        unbind();
    }

};

// single window, audioIO, and single port osc recv & send
class App: public WindowApp, public AudioApp, public osc::PacketHandler {
public:

    class AppEventHandler : public WindowEventHandler {
    public:
        App* app;
        AppEventHandler(App* a): app(a) {}
        bool resize (int dw, int dh) override {
            app->view().viewport(0, 0, app->width(), app->height());
            return true;
        }
    };

    AppGraphics mGraphics {this};
    AppEventHandler eventHandler {this};
    Viewpoint mView;
    NavInputControl mNavControl;

    Viewpoint& view() { return mView; }
    const Viewpoint& view() const { return mView; }

    Nav& nav() { return mNavControl.nav(); }
    const Nav& nav() const { return mNavControl.nav(); }

    virtual void onInit() {}
    virtual void onAnimate(double dt) {}
    virtual void onExit() {}

    virtual void preOnCreate() {
        mNavControl.target(mView);
        append(mNavControl);
        mGraphics.init();
        mView.viewport(0, 0, width(), height());
    }

    virtual void preOnAnimate(double dt) {
        mNavControl.step();
    }

    virtual void preOnDraw() {
        mGraphics.framebuffer(FBO::DEFAULT);
        mGraphics.camera(mView);
        mGraphics.loadIdentity();
        mGraphics.pushMatrix();
    }

    virtual void onDraw (AppGraphics& g) {}

    virtual void postOnDraw() {
        mGraphics.popMatrix();
    }

    void onDraw () override {
        onDraw(mGraphics);
    }

    // from WindowApp
    void open() override {
        glfw::init();
        onInit();
        create();
        preOnCreate();
        onCreate();
    }

    // from WindowApp
    void loop() override {
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
        while (!shouldQuit()) {
            // to quit, call WindowApp::quit() or click close button of window,
            // or press ctrl + q
            preOnAnimate(dt() / 1000000.0f);
            onAnimate(dt() / 1000000.0f); // millis for dt
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