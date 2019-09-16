
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/ui/al_ParameterGUI.hpp"

using namespace al;


// You can also create Parameter GUIs without the ControlGUI
// manager to use ImGUI more directly. This gives you better
// control over layout and style, but requires a bit more
// knowledge of how ImGUI works

class MyApp : public App
{
public:
    Parameter x{"X", "", 0, "", -2.0, 2.0};
    Parameter y{"Y", "", 0, "", -2.0, 2.0};
    Parameter z{"Z", "", 0, "", -2.0, 2.0};

    ParameterColor color {"Color"};

    MyApp()
    {
        nav() = Vec3d(0, 0, 2);

        addSphere(mMesh, 0.1);
        mMesh.primitive(Mesh::LINES);

        // Disable mouse nav to avoid naving while changing gui controls.
        navControl().useMouse(false);
    }

    void onCreate() override
    {
        // We must initialize ImGUI ourselves:
        imguiInit();
    }

    void onExit() override {
        imguiShutdown();
    }

    virtual void onDraw(Graphics &g) override {
        g.clear(0);
        g.pushMatrix();
        g.translate(x, y, z);
        g.color(color);
        g.draw(mMesh);
        g.popMatrix();


        // You are responsible for wrapping all your ImGUI code
        // between beginIMGUI() and endIMGUI()
        // Don't forget this or this will crash or not work!

        // Each begin()/end() pair creates a separate "window"
        ParameterGUI::beginPanel("Position Control");
        ParameterGUI::drawParameterMeta(&x);
        ParameterGUI::drawParameterMeta(&y);
        ParameterGUI::drawParameterMeta(&z);
        ParameterGUI::endPanel();

        // Specifying position and/or width makes the position/width fixed
        // A value of -1 in width or height sets automatic width
        ParameterGUI::beginPanel("Control Color", 5, 100, 150, -1);

        ParameterGUI::drawParameterMeta(&color);
        ParameterGUI::endPanel();

        imguiDraw();
    }

private:
    Mesh mMesh;
};


int main(int argc, char *argv[])
{
    MyApp app;
    app.dimensions(800, 600);
    app.title("Parameter GUI");
    app.fps(30);
    app.start();
}

