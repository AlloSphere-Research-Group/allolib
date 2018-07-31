
#include "al/core/app/al_App.hpp"
#include "al/util/ui/al_Dialog.hpp"

using namespace al;
using namespace std;

class MyApp : public App {

    void onDraw(Graphics &g) override {
        g.clear(mBrightness);
    }

    void onKeyDown(const Keyboard &k) override {
        if (k.key() == ' ') {
            std::cout << "space" << std::endl;
            Dialog d;
            auto ret = d.exec();
            std::cout << "Dialog returned : " << (int) ret << std::endl;
        }
    }

private:
    float mBrightness {0.5}; 

};

int main(int argc, char *argv[])
{
    MyApp app;

    app.start();
    return 0;
}