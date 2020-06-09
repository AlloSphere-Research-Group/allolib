#include <iostream>
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/math/al_Random.hpp"

// how to use WindowEventHandler interface to implement interaction behaviour

using namespace al;
using namespace std;

struct ObjectTransformHandler : WindowEventHandler {
  float scale = 1;
  float scale_multiplier = 1;
  Quatf quat{Quatf::identity()};
  Vec3f spin;
  Vec3f pos{0.0f, 0.0f, 0.0f};
  Vec3f vel{0.0f, 0.0f, 0.0f};

  void reset() {
    scale = 1;
    quat = Quatf::identity();
    pos.set(0, 0, 0);
  }

  void step() {
    scale *= scale_multiplier;
    quat = Quatf().fromEuler(spin) * quat;
    pos += vel;
  }

  Matrix4f mat() {
    Matrix4f rot_mat;
    quat.toMatrix(rot_mat.elems());
    return Matrix4f::translation(pos) * rot_mat * Matrix4f::scaling(scale);
  }

  bool keyDown(const Keyboard& k) override {
    switch (k.key()) {
      case 'a':
        vel[0] = -1;
        return false;
      case 'd':
        vel[0] = 1;
        return false;
      case 'e':
        vel[1] = 1;
        return false;
      case 'c':
        vel[1] = -1;
        return false;
      case 'w':
        vel[2] = -1;
        return false;
      case 'x':
        vel[2] = 1;
        return false;
      case Keyboard::UP:
        spin[1] = -0.1f;
        return false;
      case Keyboard::DOWN:
        spin[1] = 0.1f;
        return false;
      case Keyboard::RIGHT:
        spin[0] = 0.1f;
        return false;
      case Keyboard::LEFT:
        spin[0] = -0.1f;
        return false;
      case 'q':
        spin[2] = 0.1f;
        return false;
      case 'z':
        spin[2] = -0.1f;
        return false;
      case '[':
        scale_multiplier = 0.99f;
        return false;
      case ']':
        scale_multiplier = 1.01f;
        return false;
      case 's':
        reset();
        return false;
    }
    return true;
  }

  bool keyUp(const Keyboard& k) override {
    switch (k.key()) {
      case 'a':
      case 'd':
        vel[0] = 0;
        return false;
      case 'e':
      case 'c':
        vel[1] = 0;
        return false;
      case 'w':
      case 'x':
        vel[2] = 0;
        return false;
      case Keyboard::UP:
      case Keyboard::DOWN:
        spin[1] = 0;
        return false;
      case Keyboard::RIGHT:
      case Keyboard::LEFT:
        spin[0] = 0;
        return false;
      case 'q':
      case 'z':
        spin[2] = 0;
        return false;
      case '[':
      case ']':
        scale_multiplier = 1;
        return false;
    }
    return true;
  }
};

struct MyApp : App {
  ObjectTransformHandler object_transform;
  Mesh m;

  void onCreate() override {
    // remove windows's default camera control
    defaultWindow().remove(navControl());
    // use object control for model matrix
    defaultWindow().append(object_transform);

    addCube(m, false, 50);
    for (size_t i = 0; i < m.vertices().size(); i += 1) {
      m.color(rnd::uniform(), rnd::uniform(), rnd::uniform());
    }
  }

  void onAnimate(double /*dt*/) override { object_transform.step(); }

  void onDraw(Graphics& g) override {
    // interactive model transform
    g.modelMatrix(object_transform.mat());

    // fixed camera pose
    // look from (0, 0, 501), (-) is there because camera looks in -z direction
    g.viewMatrix(Matrix4f::translation(0, 0, -501));

    // ortho lens
    // make window center (0, 0, 0) and set near: z=-500, and far: z = 500
    g.projMatrix(Matrix4f::ortho(-width() / 2, width() / 2, -height() / 2,
                                 height() / 2, 1, 1000));

    g.clear(0);
    g.polygonFill();
    g.depthTesting(true);
    g.meshColor();

    // then individual transform can be done
    g.pushMatrix();
    g.rotate(al_steady_time());
    g.draw(m);
    g.popMatrix();

    g.pushMatrix();
    g.translate(150, 0, 0);
    g.draw(m);
    g.popMatrix();

    g.pushMatrix();
    g.translate(-150, 0, 0);
    g.draw(m);
    g.popMatrix();

    g.pushMatrix();
    g.translate(-150, 0, 200);
    g.draw(m);
    g.popMatrix();

    g.pushMatrix();
    g.translate(-150, 0, -200);
    g.draw(m);
    g.popMatrix();
  }
};

int main() {
  MyApp app;
  app.start();
}
