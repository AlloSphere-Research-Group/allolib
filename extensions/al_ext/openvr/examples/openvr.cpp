#include "al/core.hpp"

// The AL_EXT_OPENVR macro is set if OpenVR is found.
#ifdef AL_EXT_OPENVR
#include "module/openvr/al_OpenVRWrapper.hpp"
#endif

using namespace al;

class MyApp : public App {
public:

    void onCreate() override {

#ifdef AL_EXT_OPENVR
        // Initialize openVR in onCreate. A graphics context is needed.
        if(!mOpenVR.init()) {
            std::cerr << "ERROR: OpenVR init returned error" << std::endl;
        }
#else
        std::cerr << "Not building wiht OpenVR support" << std::endl;
#endif
        addCube(mCube);
        mCube.primitive(Mesh::LINE_STRIP);
        mCube.update();  
    }

    void onAnimate(double dt) override {
#ifdef AL_EXT_OPENVR
        // Update traking and controller data;
        mOpenVR.update();

        //openVR draw.
        // Draw in onAnimate, to make sure drawing happens only once per frame
        // Pass a function that takes Graphics &g argument
        mOpenVR.draw(std::bind(&MyApp::drawScene, this, std::placeholders::_1), mGraphics);
#endif
    }

    void drawScene(Graphics &g) {

#ifdef AL_EXT_OPENVR
      g.clear();
  // Draw a cube in the scene
      g.draw(mCube);
    // Draw markers for the controllers
    // The openVR object is available in the VRRenderer class to query the controllers
        g.pushMatrix();
            g.translate(mOpenVR.LeftController.pos);
            g.rotate(mOpenVR.LeftController.quat);
            g.scale(0.1);
            g.color(1);
            g.polygonMode(Graphics::LINE);
            g.draw(mCube);
        g.popMatrix();

            //right hand
        g.pushMatrix();
            g.translate(mOpenVR.RightController.pos);
            // std::cout << openVR->RightController.pos.x << openVR->RightController.pos.y << openVR->RightController.pos.z << std::endl;
            g.rotate(mOpenVR.RightController.quat);
            g.scale(0.1);
            g.color(1);
            g.polygonMode(Graphics::LINE);
            g.draw(mCube);
        g.popMatrix();
#else

      g.clear(1.0, 0.0, 0.0); // Draw red if not building for OpenVR
#endif
    }

    void onDraw(Graphics &g) override {
        // This onDraw determines what is seen on the Desktop window.
        // In this case, the desktop nav is independent to the HMD nav
        drawScene(g);
    }

    void onExit() override {
#ifdef AL_EXT_OPENVR
        mOpenVR.close();
#endif
    }

private:
    VAOMesh mCube;
#ifdef AL_EXT_OPENVR
    OpenVRWrapper mOpenVR;
#endif
};

int main(int argc, char *argv[]) {
    MyApp app;

    app.start();
    return 0;
}
