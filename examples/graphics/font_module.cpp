#include "al/util/al_FontModule.hpp"
#include "al/core.hpp"

// This file needs an asset stored in git lfs. Make sure you have git lfs.

#include <iostream>
using namespace al;
using namespace std;

struct MyApp : App {
    FontModule font;

    void onCreate() override {
        font.load("data/VeraMono.ttf", 128);
        font.align(TEXT_ALIGN::RIGHT);
    }

    void onDraw(Graphics& g) override {
        g.clear(0.5);
        g.blending(true);
        g.blendModeTrans();
        g.translate(0, 0, -12);
        font.render(g, "Gtesting", 1);
    }
};

int main() {
    MyApp app;
    app.start();
}


