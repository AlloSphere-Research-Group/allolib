#version 400

// take in a point and output a triangle strip with 4 vertices (aka a "quad")
//
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat4 al_ProjectionMatrix;

in Vertex {
  vec4 color;
  float size;
}
vertex[];

out Fragment {
  vec4 color;
  vec2 textureCoordinate;
}
fragment;

void main() {
  mat4 m = al_ProjectionMatrix;   // rename to make lines shorter
  vec4 v = gl_in[0].gl_Position;  // al_ModelViewMatrix * gl_Position

  // http://ogldev.atspace.co.uk/www/tutorial27/tutorial27.html
  //
  // in the tutorial at the link above, the billboarding computations
  // are done in worldspace using the camera position, but in this
  // example we apply the modelview matrix and do the billboarding
  // computations with respect to the origin which should be the
  // eye/camera position. are we wrong?

  float r = 0.15;
  r *= vertex[0].size;

  gl_Position = m * (v + vec4(-r, -r, 0.0, 0.0));
  fragment.textureCoordinate = vec2(0.0, 0.0);
  fragment.color = vertex[0].color;
  EmitVertex();

  gl_Position = m * (v + vec4(r, -r, 0.0, 0.0));
  fragment.textureCoordinate = vec2(1.0, 0.0);
  fragment.color = vertex[0].color;
  EmitVertex();

  gl_Position = m * (v + vec4(-r, r, 0.0, 0.0));
  fragment.textureCoordinate = vec2(0.0, 1.0);
  fragment.color = vertex[0].color;
  EmitVertex();

  gl_Position = m * (v + vec4(r, r, 0.0, 0.0));
  fragment.textureCoordinate = vec2(1.0, 1.0);
  fragment.color = vertex[0].color;
  EmitVertex();

  EndPrimitive();
}
