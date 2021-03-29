/*

Example: Instanced Rendering

Description: Draw a mesh multiple time efficiently
             while modifying certain attribute
             such as position. Instancing can be efficient
             since it can reduce the number of draw calls.
             But it will involve writing a custom shader for
             one's purpose.

Tags: Instanced Rendering

Author: Keehong Youn, 2018

*/

#include "al/app/al_App.hpp"

#include "al/graphics/al_Shapes.hpp"
#include "al/math/al_Random.hpp"

#include <iostream>
#include <string>
#include <vector>

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
layout (location = 1) in vec3 offset;

out float t;

void main()
{
  // to multiply position vector with matrices,
  // 4th component must be 1 (homogeneous coord)
  vec4 p = vec4(scale * position + offset, 1.0);
  gl_Position = al_ProjectionMatrix * al_ModelViewMatrix * p;
  // we also have access to index of instance,
  // for example, when drawing 100 instances,
  // `gl_InstanceID` goes 0 to 99
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

// Not only al::Vec, any user defined data type can be used
// Here a custom 3D vector is used
struct Float3 {
  float data[3] = {};
  void set(float x, float y, float z) {
    data[0] = x;
    data[1] = y;
    data[2] = z;
  }
  float operator[](size_t i) const { return data[i]; }
  float& operator[](size_t i) { return data[i]; }
};

struct MyApp : App {
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
  vector<Float3> positions;
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

    buffer.bufferType(GL_ARRAY_BUFFER);  // array buffer is used for attributes
    buffer.usage(GL_DYNAMIC_DRAW);       // dynamic: changes very frequently
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
    vao.attribPointer(1, buffer, 3, GL_FLOAT, GL_FALSE, 0, 0);
    // and specified to have attibute divisor 1
    glVertexAttribDivisor(1, 1);  // should be called with target vao bound
  }

  void onDraw(Graphics& g) override {
    static float time = 0;

    if (!PAUSE) {
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
        positions[i].set(x, y, z);
      }
      // send per-instance data to buffer
      // buffer was attached aas attribute in onCreate
      // so if buffer gets updated vao will use it automatically
      buffer.bind();
      buffer.data(positions.size() * 3 * sizeof(float), positions.data());
    }

    g.clear(0);
    g.polygonLine();
    g.depthTesting(false);
    g.blending(true);
    g.blendAdd();

    if (USE_INSTANCING) {
      g.shader(shader_instancing);
      g.shader().uniform("scale", scale);
      // Graphics::update: send al_ModelViewMatrix, al_ProjectionMatrix to
      // shader in normal cases Graphics::draw calls Graphics::update inside but
      // here Graphics::draw is not used and raw OpenGL calls are used so
      // Graphics::update needs to be called explicitly
      g.update();
      // just like above, when using Graphics::draw, given vao gets bound
      // but here it needs to be done manually
      mesh.vao().bind();
      if (mesh.indices().size()) {
        // when using index buffer, remember to bind it too
        mesh.indexBuffer().bind();
        glDrawElementsInstanced(GL_TRIANGLES, mesh.indices().size(),
                                GL_UNSIGNED_INT, 0, positions.size());
      } else {
        glDrawArraysInstanced(GL_TRIANGLES, 0, mesh.vertices().size(),
                              positions.size());
      }
    } else {
      // Note that we call draw as many times as
      // number of times we want to draw the mesh,
      // compared to one draw call in instancing case
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
  }

  bool onKeyDown(const Keyboard& k) override {
    if (k.key() == ' ') {
      USE_INSTANCING = !USE_INSTANCING;
      cout << "instancing: " << USE_INSTANCING << endl;
    }
    if (k.key() == 'p') {
      PAUSE = !PAUSE;
    }
    return true;
  }
};

int main() {
  MyApp app;
  app.start();
}
