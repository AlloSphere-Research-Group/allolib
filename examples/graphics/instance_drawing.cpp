#include "al/core.hpp"
#include <iostream>
#include <vector>
#include <string>

using namespace al;
using namespace std;

const string instancing_vert = R"(
#version 330

uniform mat4 al_ModelViewMatrix;
uniform mat4 al_ProjectionMatrix;
uniform float scale;

layout (location = 0) in vec3 position;
// attibute at location 1 will be set to have
// "attribute divisor = 1" which means for given buffer for attibute 1,
// index for reading that buffer will increase per-instance, not per-vertex
// divisor = 0 is default value and means per-vertex increase
layout (location = 1) in vec4 offset; // has w = 1

out float t;

void main()
{
  // to multiply position vector with matrices,
  // 4th component must be 1 (homogeneous coord)
  vec4 p = vec4(scale * position, 0.0) + offset;
  gl_Position = al_ProjectionMatrix * al_ModelViewMatrix * p;
  // we also have access to index of instance,
  // for example, when drawing 100 instances,
  // `gl_InstanceID goes 0 to 99
  t = float(gl_InstanceID) / (100000.0 - 1.0);
}
)";

const string instancing_frag = R"(
#version 330
in float t;
layout (location = 0) out vec4 frag_out0;
void main()
{
  frag_out0 = vec4(1.0, t, 0.0, 0.01);
}
)";

const char* gl_error_string() {
  unsigned int e = glGetError();
  switch (e) {
  case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
  case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
  case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
  case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
  case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
  case GL_NO_ERROR: return "GL_NO_ERROR";
  default: return "UNKNOWN_ERROR";
  }
}

struct Float4
{
  float data[4] = {};
  void set(float x, float y, float z, float w) {
    data[0] = x; data[1] = y; data[2] = z; data[3] = w;
  }
  float operator[](size_t i) const { return data[i]; }
  float& operator[](size_t i) { return data[i]; }
};

struct MyApp : App
{
  const size_t num_instances = 100000;
  float wx = 0;
  float px = 0;
  float wy = 0;
  float py = 0;
  float wz = 0;
  float pz = 0;
  float loops = 0;
  float scale = 0;
  bool USE_INSTANCING = false;
  bool PAUSE = false;
  VAOMesh mesh;
  BufferObject buffer;
  vector<Float4> positions;
  ShaderProgram shader_instancing;

  void randomize() {
    wx = rnd::uniform(1.0f, 5.0f);
    px = rnd::uniform(0.0f, 3.141592f * 2.0f);
    wy = rnd::uniform(1.0f, 5.0f);
    py = rnd::uniform(0.0f, 3.141592f * 2.0f);
    wz = rnd::uniform(1.0f, 5.0f);
    pz = rnd::uniform(0.0f, 3.141592f * 2.0f);
    loops = rnd::uniform(3.141592f * 5.0f, 3.141592f * 20.0f);
    scale = rnd::uniform(0.1, 2.0);
  }

  void onCreate() override {
    nav().pos(Vec3f{0, 0, 50}).faceToward({0, 0, 0}, {0, 1, 0});
    lens().near(10).far(1000);
    addTetrahedron(mesh);
    mesh.update();

    buffer.bufferType(GL_ARRAY_BUFFER); // array buffer is used for attributes
    buffer.usage(GL_DYNAMIC_DRAW); // dynamic: changes very frequently
                                   // draw: used for drawing
    buffer.create();
    positions.resize(num_instances);
    shader_instancing.compile(instancing_vert, instancing_frag);

    randomize();
    
    // attribute 0 will be the vertex positions buffer in VAOMesh object
    // so per-instance data will be attached at attribute 1
    auto& vao = mesh.vao();
    vao.bind();
    vao.enableAttrib(1);
    vao.attribPointer(1, buffer, 4, GL_FLOAT, GL_FALSE, 0, 0);
    // and specified to have attibute divisor 1
    glVertexAttribDivisor(1, 1); // should be called with target vao bound
  }

  void onDraw(Graphics& g) override {
    static float time = 0;

    if (PAUSE) goto LABEL_DRAW;

    time += 0.01;

    if (rnd::uniform() > 0.99) {
      time = 0;
      randomize();
    }

    float t, x, y, z;
    for (size_t i = 0; i < positions.size(); i += 1) {
      t = i / float(positions.size() - 1);
      t *= loops;
      x = 20 * sin(wx * t + px + time);
      y = 20 * sin(wy * t + py + time);
      z = 20 * sin(wz * t + pz + time);
      positions[i].set(x, y, z, 1);
    }
    // send per-instance data to buffer
    // buffer was attached aas attribute in onCreate
    // so if buffer gets updated vao will use it automatically
    buffer.bind();
    buffer.data(positions.size() * 4 * sizeof(float), positions.data());
  
    LABEL_DRAW:
    g.clear(0);
    g.polygonMode(Graphics::LINE);
    g.depthTesting(false);
    g.blending(true);
    g.blendModeAdd();

    if (USE_INSTANCING) {
      g.shader(shader_instancing);
      g.shader().uniform("scale", scale);
      // Graphics::update: send al_ModelViewMatrix, al_ProjectionMatrix to shader
      // in normal cases Graphics::draw calls Graphics::update inside
      // but here Graphics::draw is not used and raw OpenGL calls are used
      // so Graphics::update needs to be called explicitly
      g.update();
      // just like above, when using Graphics::draw, given vao gets bound
      // but here it needs to be done manually
      mesh.vao().bind();
      if (mesh.indices().size()){
        // when using index buffer, remember to bind it too
        mesh.indexBuffer().bind();
        glDrawElementsInstanced(GL_TRIANGLES, mesh.indices().size(),
                                GL_UNSIGNED_INT, 0, positions.size());
      }
      else {
        glDrawArraysInstanced(GL_TRIANGLES, 0, mesh.vertices().size(),
                              positions.size());
      }
    }
    else {
      g.color(1);
      for (size_t i = 0; i < positions.size(); i += 1) {
        const auto& p = positions[i];
        g.pushMatrix();
        g.translate(p[0], p[1], p[2]);
        g.scale(scale);
        g.draw(mesh);
        g.popMatrix();
      }
    }

  } // onDraw

  void onKeyDown(const Keyboard& k) override {
    if (k.key() == ' ') {
      USE_INSTANCING = !USE_INSTANCING;
      cout << "instancing: " << USE_INSTANCING << endl;
    }
    if (k.key() == 'p') {
      PAUSE = !PAUSE;
    }
  }

};

int main()
{
  MyApp app;
  app.start();
}
