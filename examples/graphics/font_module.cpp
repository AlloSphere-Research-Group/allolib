#include "al/util/al_FontModule.hpp"
#include "al/core.hpp"

// remember to put test font file in bin/data folder!

#include <iostream>
using namespace al;
using namespace std;

struct MyApp : App {
    Texture tex0;
    FontModule font;

    void onCreate() override {
        font.load("data/font.otf", 128);
        font.align(TEXT_ALIGN::RIGHT);
    }

    void onDraw(Graphics& g) override {
        g.clear(0.5);
        g.blending(true);
        g.blendModeTrans();
        g.translate(0, 0, -10);
        font.render(g, "Gtesting", 1);
    }
};

int main() {
    MyApp app;
    app.start();
}


