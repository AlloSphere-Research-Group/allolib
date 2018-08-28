/*
Allocore Example: modelshader

Description:
This demonstrates loading an OBJ and applying a shader

Author:
Graham Wakefield 2011
*/

#include "al/core.hpp"
#include "al/util/al_Asset.hpp"
#include "al/util/al_Image.hpp"
#include <algorithm> // max
#include <vector>
#include <cstdint> // uint8_t

using namespace al;
using namespace std;

SearchPaths searchpaths;
Scene* ascene = nullptr;
Vec3f scene_min, scene_max, scene_center;
Texture tex;
vector<Mesh> meshes;
float a = 0.f;  // current rotation angle

struct MyWindow : App {

  void onCreate() {
    // load in a "scene"
    FilePath path = searchpaths.find("ducky.obj");
    printf("reading %s\n", path.filepath().c_str());

    ascene = Scene::import(path.filepath());
    if (ascene == 0) {
      printf("error reading %s\n", path.filepath().c_str());
      return;
    } else {
      ascene->getBounds(scene_min, scene_max);
      scene_center = (scene_min + scene_max) / 2.f;
      ascene->print();
    }

    Image img(searchpaths.find("hubble.jpg").filepath());
    img.sendToTexture(tex);
    // tex.create2D(img.width(), img.height(), Texture::RGB8, Texture::RGB, Texture::UBYTE);
    // tex.submit(img.pixels<uint8_t>(), Texture::RGB, Texture::UBYTE);

    // extract meshes from scene
	meshes.resize(ascene->meshes());
    for (int i = 0; i < ascene->meshes(); i += 1) {
    	ascene->mesh(i, meshes[i]);
    }
  }

  void onDraw(Graphics& g) {
    g.clear(0.1);

    g.projMatrix(Matrix4f::perspective(45, aspect(), 0.1, 100));
    g.viewMatrix(Matrix4f::lookAt(Vec3d(0, 0, 4), Vec3d(0, 0, 0), Vec3d(0, 1, 0)));

    g.depthTesting(true);
    g.lighting(true);
    //g.light().dir(1.f, 1.f, 2.f);

    g.pushMatrix();

    // rotate it around the y axis
    g.rotate(a, 0.f, 1.f, 0.f);
    a += 0.5;

    // scale the whole asset to fit into our view frustum
    float tmp = scene_max[0] - scene_min[0];
    tmp = std::max(scene_max[1] - scene_min[1], tmp);
    tmp = std::max(scene_max[2] - scene_min[2], tmp);
    tmp = 2.f / tmp;
    g.scale(tmp);

    // center the model
    g.translate(-scene_center);

    tex.bind(0);
    g.texture(); // use texture to color the mesh
    // draw all the meshes in the scene
    for (auto& m : meshes) {
    	g.draw(m);
    }
    tex.unbind(0);

    g.popMatrix();
  }
};

int main(int argc, char* const argv[]) {
  searchpaths.addAppPaths(argc, argv);
  searchpaths.addSearchPath(searchpaths.appPath() + "data");
  searchpaths.print();

  MyWindow win;
  win.dimensions(640, 480);
  win.start();
}
