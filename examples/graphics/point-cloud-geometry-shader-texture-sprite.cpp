/**

Example: Point Cloud using Geometry Shader and Texture Sprite

Description: The goal of this example is to show how to make a "point cloud", a
common request from users. It is also an example of how to use a texture to
render each point in the mesh, a scheme knowns as "point sprites". We do not use
the old OpenGL "fixed pipeline"; Instead, adapted shaders from the _OpenGL 4.0
Shading Language Cookbook_ so this is also an example of one way to use a
"geometry shader". Additionally, this provides an example of how to color data
through the geometry shader using GLSL structs.

Tags: point cloud, texture sprite, geometry shader, GLSL version 400

Author: Karl Yerkes, November 2018

*/

#include "al/core.hpp"
using namespace al;

#define N (1000)
#define CLOUD_WIDTH (5.0)

const char* vertex = R"(
#version 400

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec4 vertexColor;

uniform mat4 al_ModelViewMatrix;
uniform mat4 al_ProjectionMatrix;

out Vertex {
  vec4 color;
} vertex;

void main() {
  gl_Position = al_ModelViewMatrix * vec4(vertexPosition, 1.0);
  vertex.color = vertexColor;
}
)";

const char* fragment = R"(
#version 400

in Fragment {
  vec4 color;
  vec2 textureCoordinate;
} fragment;

uniform sampler2D alphaTexture;

layout (location = 0) out vec4 fragmentColor;

void main() {
  // use the first 3 components of the color (xyz is rgb) with the texture value for alpha
  fragmentColor = vec4(fragment.color.xyz, texture(alphaTexture, fragment.textureCoordinate));
}
)";

const char* geometry = R"(
#version 400

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform float halfSize;  // Half the width of the quad
uniform mat4 al_ProjectionMatrix;

in Vertex {
  vec4 color;
} vertex[];

out Fragment {
  vec4 color;
  vec2 textureCoordinate;
} fragment;

void main() {
  mat4 m = al_ProjectionMatrix;
  vec4 v = gl_in[0].gl_Position; // al_ModelViewMatrix * gl_Position

  gl_Position = m * (v + vec4(-halfSize, -halfSize, 0.0, 0.0));
  fragment.textureCoordinate = vec2(0.0, 0.0);
  fragment.color = vertex[0].color;
  EmitVertex();

  gl_Position = m * (v + vec4(halfSize, -halfSize, 0.0, 0.0));
  fragment.textureCoordinate = vec2(1.0, 0.0);
  fragment.color = vertex[0].color;
  EmitVertex();

  gl_Position = m * (v + vec4(-halfSize, halfSize, 0.0, 0.0));
  fragment.textureCoordinate = vec2(0.0, 1.0);
  fragment.color = vertex[0].color;
  EmitVertex();

  gl_Position = m * (v + vec4(halfSize, halfSize, 0.0, 0.0));
  fragment.textureCoordinate = vec2(1.0, 1.0);
  fragment.color = vertex[0].color;
  EmitVertex();

  EndPrimitive();
}
)";

struct AlloApp : App {
  ShaderProgram shader;
  Texture texture;
  Mesh pointMesh;

  void onCreate() override {
    // use a texture to control the alpha channel of each particle
    //
    texture.create2D(256, 256, Texture::R8, Texture::RED, Texture::SHORT);
    int Nx = texture.width();
    int Ny = texture.height();
    std::vector<short> alpha;
    alpha.resize(Nx * Ny);
    for (int j = 0; j < Ny; ++j) {
      float y = float(j) / (Ny - 1) * 2 - 1;
      for (int i = 0; i < Nx; ++i) {
        float x = float(i) / (Nx - 1) * 2 - 1;
        float m = exp(-13 * (x * x + y * y));
        alpha[j * Nx + i] = m * 32000;
      }
    }
    texture.submit(&alpha[0]);

    // compile and link the three shaders
    //
    shader.compile(vertex, fragment, geometry);

    // create a mesh of points scattered randomly
    //
    pointMesh.primitive(Mesh::POINTS);
    for (int i = 0; i < N; i++) {
      pointMesh.vertex(
          Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()) *
          CLOUD_WIDTH);
      pointMesh.color(HSV(rnd::uniform(), 1.0, 1.0));
    }
  }

  float phase = 0;
  void onAnimate(double dt) override {
    phase += dt;
    if (phase > 3) phase -= 3;
  }

  void onDraw(Graphics& g) override {
    g.clear(0.23);

    g.depthTesting(false);
    g.blending(true);
    g.blendModeTrans();

    texture.bind();
    g.shader(shader);
    float halfSize = 0.2 * phase / 3;
    halfSize = halfSize < 0.05 ? 0.05 : halfSize;
    g.shader().uniform("halfSize", halfSize);
    g.draw(pointMesh);
    texture.unbind();
  }
};

int main() { AlloApp().start(); }
