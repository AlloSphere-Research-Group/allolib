
// Graphics
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
  // use the first 3 components of the color (xyz is rgb), but take the alpha value from the texture
  //
  fragmentColor = vec4(fragment.color.xyz, texture(alphaTexture, fragment.textureCoordinate));
}
)";
const char* geometry = R"(
#version 400

// take in a point and output a triangle strip with 4 vertices (aka a "quad")
//
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 al_ProjectionMatrix;

// this uniform is *not* passed in automatically by AlloLib; do it manually
//
uniform float halfSize;

in Vertex {
  vec4 color;
} vertex[];

out Fragment {
  vec4 color;
  vec2 textureCoordinate;
} fragment;

void main() {
  mat4 m = al_ProjectionMatrix; // rename to make lines shorter
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
