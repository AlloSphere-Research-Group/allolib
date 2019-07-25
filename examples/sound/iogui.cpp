

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_ParameterMIDI.hpp"
#include "al/ui/al_HtmlInterfaceServer.hpp"
#include "al/math/al_Random.hpp"

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
        parameterMidi.connectControl(Number, 7, 1);

    }

    virtual void onCreate() override {
        imguiInit();
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
        imguiBeginFrame();
        ImGui::Begin("test");
        ParameterGUI::beginPanel("IO");
        ParameterGUI::drawAudioIO(&audioIO());
        ParameterGUI::drawParameterMIDI(&parameterMidi);
        ParameterGUI::endPanel();
        ImGui::End();
        imguiEndFrame();
        imguiDraw();
    }
    virtual void onSound(AudioIOData &io) override {
        while(io()) {
            io.out(0) = rnd::uniformS() * 0.2;
        }
    }

private:
    Parameter Number{ "Number", "", 1, "", 0, 16 };
    ParameterMIDI parameterMidi;

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
