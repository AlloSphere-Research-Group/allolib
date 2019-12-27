/*
  This example shows how to use Image, Array and Texture to read a .jpg file,
display it as an OpenGL texture and print the pixel values on the command line.
Notice that while the intput image has only 4 pixels, the rendered texture is
smooth.  This is because interpolation is done on the GPU.

  Karl Yerkes and Matt Wright (2011/10/10)
*/

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

#include "al/app/al_App.hpp"
#include "al/graphics/al_Image.hpp"

using namespace al;
using namespace std;

class MyApp : public App {
 public:
  Texture texture;

  void onCreate() {
    // Load a .jpg file
    //
    const char* filename = "data/hubble.jpg";

    auto imageData = Image(filename);

    if (imageData.array().size() == 0) {
      cout << "failed to load image" << endl;
    }
    cout << "loaded image size: " << imageData.width() << ", "
         << imageData.height() << endl;

    texture.create2D(imageData.width(), imageData.height());
    texture.submit(imageData.array().data(), GL_RGBA, GL_UNSIGNED_BYTE);

    texture.filter(Texture::LINEAR);
  }

  void onDraw(Graphics& g) {
    g.clear(0.2);

    g.pushMatrix();

    // Push the texture/quad back 5 units (away from the camera)
    //
    g.translate(0, 0, -5);

    g.quad(texture, -1, -1, 2, 2);

    g.popMatrix();
  }
};

int main() {
  MyApp app;
  app.dimensions(600, 400);
  app.title("imageTexture");
  app.start();
}
