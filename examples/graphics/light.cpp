#include "al/core.hpp"
#include <iostream>
#include <cmath>

using namespace al;
using namespace std;

struct MyApp : App
{
    Light light;
    Material material;
    Mesh mesh;
    Texture tex;

    void onCreate()
    {
        // prepare test texture
        tex.create2D(64, 64);
        vector<unsigned char> pixels(4 * 64 * 64);
        for(int j=0; j<64; ++j){ float y = float(j) / (64 - 1);
        for(int i=0; i<64; ++i){ float x = float(i) / (64 - 1);
            int idx = j * 64 + i;
            pixels[4 * idx + 0] = x * 255;
            pixels[4 * idx + 1] = y * 255;
            pixels[4 * idx + 2] = 0;
            pixels[4 * idx + 3] = 255;
        }}
        tex.submit(pixels.data());

        // Light::globalAmbient(1, 1, 1);

        // light.dir(0, -1, 0);
        light.ambient({0.1, 0.1, 0.5});
        light.diffuse({1, 0, 0});
        // light.specular(1, 1, 0.5);
        // light.attenuation(1, 1, 0);

        // material.ambient(1, 1, 1);
        material.diffuse({1, 1, 0});
        // material.specular(1, 1, 1);
        // material.shininess(50);
        // material.emission(1, 1, 1);

        addSphereWithTexcoords(mesh, 2);
        nav().pos(0, 10, 50).faceToward({0, 0, 0}, {0, 1, 0});
    }

    void draw(Mesh& m, float x, float y, float z)
    {
        auto& g = graphics();
        g.pushMatrix();
        g.translate(x, y, z);
        g.draw(mesh);
        g.popMatrix();
    }

    void onAnimate(double dt)
    {
        light.pos(8 * cos(2 * sec()), 10, 2 + 5 * sin(2 * sec()));
    }

    void onDraw(Graphics& g)
    {
        g.clear(1, 1, 1);
        g.depthTesting(true);

        // will be used when Graphics::texture() is called
        tex.bind();

        // enable lighting
        g.lighting(true);

        g.light(light);

        // light + material
        // g.material(material);
        // draw(mesh, -12, 0, 0);

        // light + uniform color
        g.color(0.5, 0.5, 0.5);
        draw(mesh, -6, 0, 0);

        // light + mesh color
        // g.meshColor();
        // draw(mesh, 0, 0, 0);

        // light + texture
        // g.texture();
        // draw(mesh, 6, 0, 0);

        // no lighting
        g.lighting(false);

        g.color(1, 1, 0);
        draw(mesh, 12, 0, 0);

        tex.unbind();
    }

};

int main () {
    MyApp app;
    app.start();
}
