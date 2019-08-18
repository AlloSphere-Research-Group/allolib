#include "al/app/al_App.hpp"
#include "al/graphics/al_DefaultShaderString.hpp"
#include "al/graphics/al_Shapes.hpp"
#include <iostream>

struct MyApp : al::App {

  al::VAOMesh mesh;
  al::ShaderProgram shaders[6];
  int uCol;
  int modelView, projection;

  void onCreate () override {
    al::addSphere(mesh);
    mesh.generateNormals();
    mesh.update();

    al::ShaderSources un = al::defaultShaderUniformColor(false, false, false);
    std::cout << un.vert.str << std::endl;
    std::cout << un.frag.str << std::endl;
    shaders[0].compile(un.vert.str, un.frag.str);
    uCol = shaders[0].getUniformLocation("uColor");
    modelView = shaders[0].getUniformLocation("alModelViewMatrix");
    projection = shaders[0].getUniformLocation("alProjectionMatrix");
  }

  void onDraw (al::Graphics& g) override {
    al::Mat4f mv = view().viewMatrix();
    al::Mat4f pr = view().projMatrix(float(width()), float(height()));

    al::gl::clearColor(0.0f, 0.0f, 0.0f, 1.0f);
    shaders[0].use();
    shaders[0].uniform4f(uCol, 1.0f, 1.0f, 0.0f, 1.0f);
    shaders[0].uniformMat4f(modelView, mv.elems());
    shaders[0].uniformMat4f(projection, pr.elems());
    mesh.draw();
  }

};

int main () {
  MyApp app;
  app.start();
}
