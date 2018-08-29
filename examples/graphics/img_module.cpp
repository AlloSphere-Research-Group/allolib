/*

    Image loading with img module.
    The advatage of img module is that it does not have dependencies
    img_module::LoadImage takes const char* filepath and
    returns struct img_module::ImageData
    
    struct ImageData {
        int width = 0;
        int height = 0;
        std::vector<uint8_t> data;
    };

    [!] before running this example make sure any img of your choice is
        in ${folder_of_this_file}/bin/data
        (allolib's cmakefile puts working directory at bin folder)

*/

#include "al/core.hpp"
// TODO: figure out how to handle include dirs for modules
#include "../modules/img/loadImage.hpp"

#include <iostream>

using namespace al;
using namespace std;

struct MyApp : App {
    Texture tex;

    void onCreate() override {
        auto imageData = img_module::loadImage("data/hubble.jpg");
        if (imageData.data.size() == 0) {
            cout << "failed to load image" << endl;
        }
        cout << "loaded image size: " << imageData.width << ", " << imageData.height << endl;

        tex.create2D(imageData.width, imageData.height);
        tex.submit(imageData.data.data(), GL_RGBA, GL_UNSIGNED_BYTE);
    }

    void onDraw(Graphics& g) override {
        g.clear(0);
        g.quadViewport(tex, -0.5, -0.5, 1, 1);
    }
};

int main () {
    MyApp app;
    app.start();
}
