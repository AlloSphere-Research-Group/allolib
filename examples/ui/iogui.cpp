

#include "al/core/app/al_App.hpp"
#include "al/core/graphics/al_Shapes.hpp"
#include "al/util/ui/al_ControlGUI.hpp"
#include "al/util/ui/al_ParameterMIDI.hpp"
#include "al/util/ui/al_HtmlInterfaceServer.hpp"
#include "al/core/math/al_Random.hpp"

using namespace al;

class MyApp : public App
{
public:
    MyApp() {
        nav().pos(Vec3d(0, 0, 8));

        addCone(mMesh);
        mMesh.generateNormals();

        // Disable mouse nav to avoid naving while changing gui controls.
        navControl().useMouse(false);

    }

    virtual void onCreate() override {
        ParameterGUI::initialize();
    }

    virtual void onDraw(Graphics &g) override
    {
        g.clear(0);
        //light();
        for (int i = 0; i < Number.get(); ++i) {
            g.pushMatrix();
            g.translate((i % 4) - 2, (i / 4) - 2, -5);
            g.draw(mMesh);
            g.popMatrix();
        }
        ParameterGUI::beginDraw();
        ParameterGUI::beginPanel("IO");
        ParameterGUI::drawAudioIO(&audioIO());
        ParameterGUI::drawParameterMIDI(&parameterMidi);
        ParameterGUI::endPanel();
        ParameterGUI::endDraw();
    }
    virtual void onSound(AudioIOData &io) {
        while(io()) {
            io.out(0) = rnd::uniformS() * 0.2;
        }
    }

private:
    ParameterInt Number{ "Number", "", 1, "", 0, 16 };
    ParameterMIDI parameterMidi;

    Light light;
    Mesh mMesh;
};


int main(int argc, char *argv[])
{
    MyApp app;
    app.dimensions(800, 600);
    app.title("Presets GUI");
    app.fps(30);
    app.initAudio(44100, 256, 2, 2);
    app.start();
    return 0;
}
