
#include "al/core/app/al_App.hpp"
#include "al/core/graphics/al_Shapes.hpp"
#include "al/util/ui/al_ControlGUI.hpp"
#include "al/util/ui/al_ParameterMIDI.hpp"
#include "al/util/ui/al_HtmlInterfaceServer.hpp"

using namespace al;


class MyApp : public App
{
public:
    Parameter x{"X", "", 0, "", -2.0, 2.0};
    Parameter y{"Y", "", 0, "", -2.0, 2.0};
    Parameter z{"Z", "", 0, "", -2.0, 2.0};

    ParameterColor color {"Color"};

    void onCreate()
    {
        nav() = Vec3d(0, 0, 2);

        addSphere(mMesh, 0.1);
        mMesh.primitive(Mesh::LINES);

        initIMGUI(); // We need to call initIMGUI manually as we want two control windows
        mPositionGUI.init(5, 5, false);
        mPositionGUI << x << y << z;
        mColorGUI.init(300, 5, false);
        mColorGUI << color;
    }

    void onAnimate(double dt) override {
        navControl().active(!mPositionGUI.usingInput() && !mColorGUI.usingInput());
    }

    virtual void onDraw(Graphics &g) {
        g.clear(0);

        g.pushMatrix();
        g.translate(x, y, z);
        g.color(color);
        g.draw(mMesh);
        g.popMatrix();

        beginIMGUI();
        mPositionGUI.draw(g);
        mColorGUI.draw(g);
        endIMGUI();
    }

private:
    ControlGUI mPositionGUI;
    ControlGUI mColorGUI;

    Mesh mMesh;
};


int main(int argc, char *argv[])
{
    MyApp app;
    app.dimensions(800, 600);
    app.title("Presets GUI");
    app.fps(30);
    app.start();
}

