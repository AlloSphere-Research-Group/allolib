

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_ParameterMIDI.hpp"
#include "al/ui/al_HtmlInterfaceServer.hpp"

using namespace al;

struct MyApp : public App
{
    MyApp() {
        nav().pos(Vec3d(0, 0, 8));

        presets << Number << Size << Red << Green << Blue; // Add parameters to preset handling

        // Now make control GUI
        // You can add Parameter objects using the streaming operator.
        // They will all be laid out vertically
        gui << Number << Size;
        gui << Red << Green << Blue;

        // Expose parameters to network (You can send OSC message to them
        parameterServer() << Number << Size << Red << Green << Blue;

        // Print server configuration
        parameterServer().print();

        // Adding a PresetHandler to a ControlGUI creates a multi-button interface
        // to control the presets.
        gui << presets;

        addCone(mMesh);
        mMesh.generateNormals();

        // Disable mouse nav to avoid naving while changing gui controls.
        navControl().useMouse(false);

    }

    virtual void onCreate() override {
        gui.init();
    }

    virtual void onDraw(Graphics &g) override
    {
        g.clear(0);
        //light();
        for (int i = 0; i < Number.get(); ++i) {
            g.pushMatrix();
            g.translate((i % 4) - 2, (i / 4) - 2, -5);
            g.scale(Size.get());
            g.color(Red.get(), Green.get(), Blue.get());
            g.draw(mMesh);
            g.popMatrix();
        }
        gui.draw(g);
    }

private:
    ParameterInt Number{ "Number", "", 1, "", 0, 16 };
    Parameter Size{ "Size", "", 0.3, "", 0.1, 2.0 };
    Parameter Red{ "Red", "Color", 0.5, "", 0.0, 1.0 };
    Parameter Green{ "Green", "Color", 1.0, "", 0.0, 1.0 };
    Parameter Blue{ "Blue", "Color", 0.5, "", 0.0, 1.0 };

    PresetHandler presets{ "presetsGUI" };

    ControlGUI gui;

    Light light;
    Mesh mMesh;
};


int main(int argc, char *argv[])
{
    MyApp app;
    app.dimensions(800, 600);
    app.title("Presets GUI");
    app.fps(30);
    app.start();
    return 0;
}
