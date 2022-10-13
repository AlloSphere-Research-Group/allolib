#version 400

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec4 vertexColor;
layout(location = 2) in vec2
    vertexSize;  // as a 2D texture cordinate, but we ignore the y

uniform mat4 al_ModelViewMatrix;
uniform mat4 al_ProjectionMatrix;

out Vertex {
  vec4 color;
  float size;
}
vertex;

void main() {
  gl_Position = al_ModelViewMatrix * vec4(vertexPosition, 1.0);
  vertex.color = vertexColor;
  vertex.size = vertexSize.x;
}