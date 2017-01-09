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
    std::cout << "MyApp beforeCreate" << std::endl;
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primary);
    dimensions(mode->width / 2, mode->height / 2);
    decorated(false);
  }

  void onCreate() {
    //std::cout << "MyApp onCreate" << std::endl;

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
    auto loc = glGetUniformLocation(shader.id(), "tex");
    glUniform1f(loc, 0);
    //shader.uniform1("tex", 0);
    shader.end();
    
    int w = 16;
    int h = 16  ;
    int internal = GL_RGBA8; // GL_RGBA16/32F/16F,
                             // GL_DEPTH_COMPONENT32F/24/16, ...
    int format = GL_RGBA; // GL_RGB, GL_DEPTH_COMPONENT, ...
    int type = GL_FLOAT; // GL_UNSIGNED_BYTE (what data type will we give?)
    texture.create2D(w, h, internal, format, type);

    array<float, 16 * 16 * 4> arr;
    for (int i = 0; i < 16; i++) {
      for (int j = 0; j < 16; j++) {
        int idx = i + 16 * j;
        arr[4 * idx + 0] = i / 15.0f;
        arr[4 * idx + 1] = j / 15.0f;
        arr[4 * idx + 2] = 1.0f;
        arr[4 * idx + 3] = 1.0f;
      }
    }
    texture.bind();
    texture.submit(arr.data()); // give raw pointer
    texture.mipmap(true); // turn on only if needed
    texture.update();
    texture.unbind();

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

    if (fullScreen()) {
      GLfloat const clear_color[] = {1.0f, 0.0f, 0.0f, 1.0f};
      glClearBufferfv(GL_COLOR, 0, clear_color);
    }
    else {
      GLfloat const clear_color[] = {0.0f, 0.0f, 1.0f, 1.0f};
      glClearBufferfv(GL_COLOR, 0, clear_color); 
    }

    shader.begin();

    texture.bind();
    glViewport(0, 0, fbWidth() / 2, fbHeight());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    texture.unbind();

    color.bind();
    glViewport(fbWidth() / 2, 0, fbWidth() / 2, fbHeight());
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

  void onKeyDown(Keyboard const& k) {
    std::cout << "onKeyDown" << char(k.key()) << std::endl;
    dimensions(Window::Dim(100, 100, 200, 100));
  }

  void onResize(int w, int h) {
    cout << "resize:" << endl;
    cout << w << ", " << h << endl;
    cout << width() << ", " << height() << endl;
    cout << dimensions().l << ", " << dimensions().t << endl;
  }

};

int main(int argc, char* argv[]) {
  MyApp app;
  app.fps(60);
  app.start();
  return 0;
}