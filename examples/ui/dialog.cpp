
#include "al/app/al_App.hpp"
#include "al/ui/al_Dialog.hpp"
//#include "al/graphics/al_Font.hpp"

using namespace al;
using namespace std;

class MyApp : public App {

    void onCreate() override {
      // FIXME load some default
//        font.load("arial.ttf");
    }

    void onDraw(Graphics &g) override {
        g.clear(mBrightness);
        g.translate(-1, 0, -4);
        g.blendAdd();
        g.blending(true);

//        font.render(g, "Press space bar to show dialog");
    }

    void onKeyDown(const Keyboard &k) override {
        if (k.key() == ' ') {
            std::cout << "space" << std::endl;
            Dialog d("Question", "How are you?",
                     Dialog::Buttons::AL_DIALOG_BUTTON_OK | Dialog::Buttons::AL_DIALOG_BUTTON_CANCEL );
            auto ret = d.exec();
            if (ret == Dialog::DialogResult::AL_DIALOG_OK) {
                std::cout << "Dialog returned OK " << std::endl;
            } else if (ret == Dialog::DialogResult::AL_DIALOG_CANCEL) {
                std::cout << "Dialog returned CANCEL " << std::endl;
            }
        }
    }

private:
    float mBrightness {0.5};
//    Font font;
};

int main(int argc, char *argv[])
{
    MyApp app;
    app.start();
    return 0;
}
