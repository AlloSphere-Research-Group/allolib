#include <iostream>
#include "al/app/al_App.hpp"
#include "al/graphics/al_DefaultShaderString.hpp"
#include "al/graphics/al_Shapes.hpp"

struct ShaderWithInterface {
  al::ShaderProgram shader;
  // -1 means the uniform does not exist
  int modelviewLocation = -1;
  int projectionLocation = -1;
  int normalMatLocation = -1;
  int uniformColorLocation = -1;
  int lightPositionLocation = -1;

  void compile(const char* vert, const char* frag) {
    if (!shader.compile(vert, frag)) {
      std::cerr << "failed to compile shader program\n";
      return;
    }
    GLuint id = (GLuint)shader.id();
    modelviewLocation = glGetUniformLocation(id, "alModelViewMatrix");
    projectionLocation = glGetUniformLocation(id, "alProjectionMatrix");
    normalMatLocation = glGetUniformLocation(id, "alNormalMatrix");
    uniformColorLocation = glGetUniformLocation(id, "uColor");
    lightPositionLocation = glGetUniformLocation(id, "lightPositionEyeCoord");
  }

  void use() { shader.use(); }

  // when -1 is given for the location, OpenGL silently ignores the call
  void modelView(float* data) { shader.uniformMat4f(modelviewLocation, data); }
  void projection(float* data) {
    shader.uniformMat4f(projectionLocation, data);
  }
  void normalMat(float* data) { shader.uniformMat4f(normalMatLocation, data); }
  void uniformColor(float r, float g, float b, float a) {
    shader.uniform4f(uniformColorLocation, r, g, b, a);
  }
  void lightPos(float x, float y, float z, float w) {
    shader.uniform4f(lightPositionLocation, x, y, z, w);
  }
};

struct MyApp : al::App {
  al::VAOMesh mesh;
  ShaderWithInterface shaders[6];
  double t;

  void onCreate() override {
    al::addTetrahedron(mesh);
    mesh.generateNormals();
    mesh.update();

    al::ShaderSources un = al::defaultShaderUniformColor(false, false, false);
    shaders[0].compile(un.vert.c_str(), un.frag.c_str());

    al::ShaderSources ul = al::defaultShaderUniformColor(false, false, true);
    shaders[1].compile(ul.vert.c_str(), ul.frag.c_str());

    t = 0.0;
  }

  void onDraw(al::Graphics& g) override {
    t += 0.01;

    // view matrix, projection matrix, light position
    al::Mat4f vw = view().viewMatrix();
    al::Mat4f pr = view().projMatrix(float(width()), float(height()));
    al::Vec4f lp{4 * std::sin(float(t)), 4 * std::cos(float(t)), 0.0f, 1.0f};

    al::gl::clearColor(1.0f, 1.0f, 1.0f, 1.0f);
    al::gl::clearDepth(1.0f);
    al::gl::depthTesting(true);

    // uniform color, no lighting
    al::Mat4f md0 = al::Mat4f::translation(1.0f, 0.0f, 0.0f) *
                    al::Mat4f::rotation(M_PI_4, 0, 1);
    shaders[0].use();
    shaders[0].modelView((vw * md0).elems());
    shaders[0].projection(pr.elems());
    shaders[0].uniformColor(1.0f, 1.0f, 0.0f, 1.0f);
    mesh.draw();

    // uniform color, lighting
    al::Mat4f md1 = al::Mat4f::translation(-1.0f, 0.0f, 0.0f) *
                    al::Mat4f::rotation(M_PI_2, 0, 1);
    al::Vec4f lp1 = vw * lp;  // light position in eye coord
    shaders[1].use();
    shaders[1].modelView((vw * md1).elems());
    shaders[1].projection(pr.elems());
    shaders[1].uniformColor(0.0f, 1.0f, 1.0f, 1.0f);
    shaders[1].normalMat(vw.elems());  // no scaling: normal mat = view mat
    shaders[1].lightPos(lp1.x, lp1.y, lp1.z, lp1.w);
    mesh.draw();
  }
};

int main() {
  MyApp app;
  app.start();
}
