#include <sstream>
#include <string>

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"

using namespace al;
using namespace std;

Matrix4f getLookAt(const Vec3f &ux, const Vec3f &uy, const Vec3f &uz,
                   const Vec3f &p) {
  return Matrix4f(ux[0], ux[1], ux[2], -(ux.dot(p)), uy[0], uy[1], uy[2],
                  -(uy.dot(p)), uz[0], uz[1], uz[2], -(uz.dot(p)), 0, 0, 0, 1);
}

Matrix4f getLookAt(const Vec3f &eyePos, const Vec3f &at, const Vec3f &up) {
  Vec3f z = (eyePos - at).normalize();
  Vec3f x = cross(up, z).normalize();
  Vec3f y = cross(z, x).normalize();
  return getLookAt(x, y, z, eyePos);
}

string mat_to_string(const Matrix4f &m) {
  stringstream s;
  s << to_string(m[0]) << to_string(m[4]) << to_string(m[8]) << to_string(m[12])
    << '\n'
    << to_string(m[1]) << to_string(m[5]) << to_string(m[9]) << to_string(m[13])
    << '\n'
    << to_string(m[2]) << to_string(m[6]) << to_string(m[10])
    << to_string(m[14]) << '\n'
    << to_string(m[3]) << to_string(m[7]) << to_string(m[11])
    << to_string(m[15]) << '\n';
  return s.str();
}

struct MyApp : App {
  Mesh mesh;
  Mesh axis;
  Light light;
  double angle1, angle2;

  void onCreate() override {
    angle1 = angle2 = 0;
    int Nv = addIcosahedron(mesh);
    for (int i = 0; i < Nv; ++i) {
      float f = float(i) / Nv;
      mesh.color(HSV(f, 1, 1));
    }
    mesh.decompress();
    mesh.generateNormals();

    int num_verts_added;
    Mat4f transform;

    // x
    num_verts_added = addCube(axis);
    transform.setIdentity();
    transform *= Matrix4f::rotation(M_PI / 2, 2, 0); // rotate from z to x
    transform *= Matrix4f::translation(0, 0, 0.5);
    transform *= Matrix4f::scaling(0.1, 0.1, 3);
    axis.transform(transform, axis.vertices().size() - num_verts_added);
    for (int i = 0; i < num_verts_added; i += 1) {
      axis.color(1, 0, 0);
    }

    // y
    num_verts_added = addCube(axis);
    transform.setIdentity();
    transform *= Matrix4f::rotation(M_PI / 2, 2, 1); // rotate from z to y
    transform *= Matrix4f::translation(0, 0, 0.5);
    transform *= Matrix4f::scaling(0.1, 0.1, 3);
    axis.transform(transform, axis.vertices().size() - num_verts_added);
    for (int i = 0; i < num_verts_added; i += 1) {
      axis.color(0, 1, 0);
    }

    // z
    num_verts_added = addCube(axis);
    transform.setIdentity();
    transform *= Matrix4f::translation(0, 0, 0.5);
    transform *= Matrix4f::scaling(0.1, 0.1, 3);
    axis.transform(transform, axis.vertices().size() - num_verts_added);
    for (int i = 0; i < num_verts_added; i += 1) {
      axis.color(0, 0, 1);
    }

    nav().pos(0, 0, 10).faceToward({0, 0, 0}, {0, 1, 0});

    light.pos(0, 10, 0);
  }

  void onAnimate(double /*dt*/) override {
    angle1 += 0.02;
    angle2 += 0.1;
  }

  void onDraw(Graphics &g) override {
    g.clear(0, 0, 0);

    g.depthTesting(true);
    g.light(light);

    auto draw_mesh = [this, &g]() {
      g.lighting(false);
      g.meshColor();
      g.polygonFill();
      g.draw(axis);

      g.rotate(angle1, 0, 1, 0);
      g.rotate(angle2, 1, 0, 0);

      g.lighting(true);
      g.meshColor();
      g.polygonFill();
      g.draw(mesh);

      g.scale(1.01);
      g.color(0);
      g.polygonLine();
      g.draw(mesh);
    };

    int w = fbWidth() / 2;
    int h = fbHeight() / 2;
    g.scissorTest(true);

    {
      g.viewport(0, 0, w, h);
      g.scissorArea(0, 0, w, h);
      g.clear(0.9, 0.9, 0.5);
      g.pushMatrix();
      g.pushViewMatrix();
      g.viewMatrix(getLookAt({8, 0, 0}, {0, 0, 0}, {0, 1, 0}));
      draw_mesh();
      g.popViewMatrix();
      g.popMatrix();
    }

    {
      g.viewport(w, 0, w, h);
      g.scissorArea(w, 0, w, h);
      g.clear(0.5, 0.9, 0.5);
      g.pushMatrix();
      g.pushViewMatrix();
      g.viewMatrix(getLookAt({0, 8, 0}, {0, 0, 0}, {0, 0, 1}));
      draw_mesh();
      g.popViewMatrix();
      g.popMatrix();
    }

    {
      g.viewport(0, h, w, h);
      g.scissorArea(0, h, w, h);
      g.clear(0.5, 0.9, 0.9);
      g.pushMatrix();
      g.pushViewMatrix();
      g.viewMatrix(getLookAt({0, 0, 8}, {0, 0, 0}, {1, 0, 0}));
      draw_mesh();
      g.popViewMatrix();
      g.popMatrix();
    }

    {
      g.viewport(w, h, w, h);
      g.scissorArea(w, h, w, h);
      g.clear(0.9, 0.5, 0.9);
      g.pushMatrix();
      g.pushViewMatrix();
      g.viewMatrix(getLookAt({8, 8, 8}, {0, 0, 0}, {0, 1, 0}));
      draw_mesh();
      g.popViewMatrix();
      g.popMatrix();
    }

    g.viewport(0, 0, fbWidth(), fbHeight());
    g.scissorTest(false);
    g.clearDepth(1);

    g.pushMatrix();
    draw_mesh();
    g.popMatrix();
  }
};

int main() {
  MyApp app;
  app.start();
}
