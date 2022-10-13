#version 400

in Fragment {
  vec4 color;
  vec2 textureCoordinate;
}
fragment;

uniform sampler2D alphaTexture;

layout(location = 0) out vec4 fragmentColor;

void main() {
  float a = texture(alphaTexture, fragment.textureCoordinate).r;
  // if (a < 0.05) discard;
  fragmentColor = vec4(fragment.color.xyz, a);
}