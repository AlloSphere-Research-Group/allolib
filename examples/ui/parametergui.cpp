
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
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

    MyApp()
    {
        // Add parameters to GUI
        // You can pipe any Parameter type (ParameterColor, ParameterVec3f, ParameterBool, etc.) and the
        // GUI will generate the appropriate controls
        mParameterGUI << x << y << z << color;
        nav() = Vec3d(0, 0, 2);
        // Add parameters to OSC server
        parameterServer() << x << y << z << color;
        parameterServer().print();

        // Expose parameter server in html interface
        //mInterfaceServer << mServer;

        addSphere(mMesh, 0.1);
        mMesh.primitive(Mesh::LINES);

        // Connect MIDI CC #1 on channel 1 to parameter x
        mParameterMIDI.connectControl(x, 1, 1);

        // Broadcast parameter changes to localhost por 9011
        parameterServer().addListener("localhost", 9011);

        // Disable mouse nav to avoid naving while changing gui controls.
        navControl().useMouse(false);
    }

    void onCreate()
    {
        Light::globalAmbient({ 0.2, 1, 0.2 });
        mParameterGUI.init();
    }

    virtual void onDraw(Graphics &g) {
        g.clear(0);
        g.pushMatrix();
        g.translate(x, y, z);
        g.color(color);
        g.draw(mMesh);
        g.popMatrix();
        mParameterGUI.draw(g);
    }

private:
    ControlGUI mParameterGUI;
    ParameterMIDI mParameterMIDI;

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

