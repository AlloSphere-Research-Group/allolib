#include "al/core.hpp"
#include <iostream>
#include <string>
#include <array>

using namespace al;
using namespace std;

class MyApp : public WindowApp {
public:
  ShaderProgram shader;
  Texture texture;
  FBO fbo;
  Texture color;
  RBO depth;
  GLuint vao;

  void beforeCreate() {
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primary);
    dimensions(500, 500);
  }

  void onCreate() {
    std::cout << "MyApp onCreate" << std::endl;

    string const vert_source = R"(
      #version 330

      out vec2 _texcoord;

      void main() {
        const vec4[] vertex = vec4[4](
          vec4(-0.5, -0.5, 0.0, 1.0),
          vec4(0.5, -0.5, 0.0, 1.0),
          vec4(-0.5, 0.5, 0.0, 1.0),
          vec4(0.5, 0.5, 0.0, 1.0)
        );
        gl_Position = vertex[gl_VertexID];
        _texcoord = vec2(vertex[gl_VertexID].xy) + 0.5;
      }
    )";

    string const frag_source = R"(
      #version 330

      uniform sampler2D tex;

      in vec2 _texcoord;

      out vec4 frag_color;

      void main() {
        frag_color = texture(tex, _texcoord);
        // frag_color = vec4(_texcoord, 0.0, 1.0);
        // frag_color = vec4(0.0, 1.0, 1.0, 1.0);
      }
    )";

    shader.compile(vert_source, frag_source);
    shader.begin();
    shader.uniform1("tex", 0);
    shader.end();
    
    int w = 256;
    int h = 256;
    int internal = GL_RGBA8; // GL_RGBA16/32F/16F,
                             // GL_DEPTH_COMPONENT32F/24/16, ...
    int format = GL_RGBA; // GL_RGB, GL_DEPTH_COMPONENT, ...
    int type = GL_FLOAT; // GL_UNSIGNED_BYTE (what data type will we give?)
    texture.create2D(w, h, internal, format, type);

    array<float, 256 * 256 * 4> arr;
    for (int i = 0; i < 256; i++) {
      for (int j = 0; j < 256; j++) {
        int idx = i + 256 * j;
        arr[4 * idx + 0] = i / 255.0f;
        arr[4 * idx + 1] = j / 255.0f;
        arr[4 * idx + 2] = 1.0f;
        arr[4 * idx + 3] = 1.0f;
      }
    }
    texture.submit(arr.data()); // give raw pointer
    texture.mipmap(true); // tuen on only if needed

    color.create2D(128, 128);
    depth.create(128, 128);
    fbo.attachTexture2D(color);
    fbo.attachRBO(depth);
    printf("fbo status %s\n", fbo.statusString());

    fbo.begin();
    GLfloat const clear_color[] = {0.0f, 1.0f, 0.0f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, clear_color);
    fbo.end();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
  }

  void onDraw() {
    static int i = 0;
    GLfloat const clear_color[] = {1.0f, 0.0f, 0.0f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, clear_color);

    shader.begin();

    texture.bind();
    glViewport(0, 0, width() / 2, height());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    texture.unbind();

    color.bind();
    glViewport(width() / 2, 0, width() / 2, height());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    color.unbind();

    shader.end();

    if (i >= 6000) {
      // run for 600 frames and print average fps when done
      cout << (6000 / sec()) << endl;
      quit();
    }
    i += 1;
  }

  void onKeyUp(Keyboard const& k) {
    std::cout << "onKeyUp" << char(k.key()) << std::endl;
    dimensions(Window::Dim(100, 100, 200, 100));
  }

};

int main(int argc, char* argv[]) {
  MyApp app;
  app.fps(60);
  app.start();
  return 0;
}