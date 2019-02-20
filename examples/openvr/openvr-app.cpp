#include "al/openvr/al_VRApp.hpp"

using namespace al;

class MyApp : public VRApp {
public:
    void onCreate() override {
        
        addCube(mCube);
        mCube.primitive(Mesh::LINE_STRIP);
        mCube.update();  
    }

    // In the VRApp the draw function only draws to the HMD
    void onDraw(Graphics &g) override {
        g.clear();
    // Draw a cube in the scene
        g.draw(mCube);

    // Draw markers for the controllers
    // The openVR object is available in the VRRenderer class to query the controllers
        g.pushMatrix();
            g.translate(openVR->LeftController.pos);
            g.rotate(openVR->LeftController.quat);
            g.scale(0.1);
            g.color(1);
            g.polygonMode(Graphics::LINE);
            g.draw(mCube);
        g.popMatrix();

            //right hand
        g.pushMatrix();
            g.translate(openVR->RightController.pos);
            // std::cout << openVR->RightController.pos.x << openVR->RightController.pos.y << openVR->RightController.pos.z << std::endl;
            g.rotate(openVR->RightController.quat);
            g.scale(0.1);
            g.color(1);
            g.polygonMode(Graphics::LINE);
            g.draw(mCube);
        g.popMatrix();
    }

private:
    VAOMesh mCube;
};


int main(int argc, char *argv[]) {
    MyApp app;

    app.start();
    return 0;
}