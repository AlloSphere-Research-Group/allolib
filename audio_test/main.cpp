#include "al/core.hpp"
#include <iostream>
#include <string>
#include <array>

using namespace al;
using namespace std;

class MyApp : public WindowApp {
public:
  ShaderProgram shader;
  VAOMesh mesh;
  
  void onCreate() {
    string const vert_source = R"(
      #version 330

      uniform mat4 m;

      layout (location = 0) in vec4 position;
      layout (location = 1) in vec4 color;
      layout (location = 2) in vec2 texcoord;

      out vec4 _color;

      void main() {
        gl_Position = m * position;
        _color = color;
      }
    )";

    string const frag_source = R"(
      #version 330

      in vec4 _color;

      out vec4 frag_color;

      void main() {
        frag_color = _color;
      }
    )";

    shader.compile(vert_source, frag_source);

    mesh.primitive(TRIANGLES);
    mesh.vertex(-0.5, -0.5, 0);
    mesh.color(1.0, 0.0, 0.0);
    mesh.vertex(0.5, -0.5, 0);
    mesh.color(0.0, 1.0, 0.0);
    mesh.vertex(-0.5, 0.5, 0);
    mesh.color(0.0, 0.0, 1.0);
    mesh.vertex(-0.5, 0.5, 0);
    mesh.color(0.0, 0.0, 1.0);
    mesh.vertex(0.5, -0.5, 0);
    mesh.color(0.0, 1.0, 0.0);
    mesh.vertex(0.5, 0.5, 0);
    mesh.color(0.0, 1.0, 1.0);
    mesh.update();
  }

  void onDraw() {
    glViewport(0, 0, fbWidth(), fbHeight());
    GLfloat const clear_color[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, clear_color);

    float w = width();
    float h = height();
    Matrix4f mat = Matrix4f::rotate(sec(), 0, 0, 1);
    mat = Matrix4f::scaling(h / w, 1.0f, 1.0f) * mat;
    
    shader.begin();
    shader.uniform("m", mat);
    mesh.draw();
    shader.end();
  }
};

class AudioApp {
public:
  AudioIO mAudioIO;
  AudioIO& audioIO(){ return mAudioIO; }
  const AudioIO& audioIO() const { return mAudioIO; }
  void initAudio(
    double audioRate=44100, int audioBlockSize=128,
    int audioOutputs=-1, int audioInputs=-1
  );
  virtual void onSound(AudioIOData& io){
    static double phase {0};
    // Set the base frequency to 55 Hz
    double freq = 55/io.framesPerSecond();

    while(io()){
      // Update the oscillators' phase
      phase += freq;
      if(phase > 1) phase -= 1;

      // Generate two sine waves at the 5th and 4th harmonics
      float out1 = cos(5*phase * 2*M_PI);
      float out2 = sin(4*phase * 2*M_PI);

      // Send scaled waveforms to output...
      io.out(0) = out1*0.2;
      io.out(1) = out2*0.2;
    }
  }
  bool usingAudio() const;
  void start(){
    if(usingAudio()) mAudioIO.start();
  }
  void exit(){
    audioIO().close();
  }
};

static void AppAudioCB(AudioIOData& io){
  AudioApp& app = io.user<AudioApp>();
  io.frame(0);
  app.onSound(app.audioIO());
}

bool AudioApp::usingAudio() const {
  return audioIO().callback == AppAudioCB;
}

void AudioApp::initAudio(
  double audioRate, int audioBlockSize,
  int audioOutputs, int audioInputs
) {
  mAudioIO.callback = AppAudioCB;
  mAudioIO.user(this);
  mAudioIO.framesPerSecond(audioRate);
  mAudioIO.framesPerBuffer(audioBlockSize);
  mAudioIO.channelsOut(audioOutputs);
  mAudioIO.channelsIn(audioInputs);
}

int main(int argc, char* argv[]) {

  AudioApp audioApp;
  audioApp.initAudio();
  audioApp.start();

  MyApp app;
  app.dimensions(1000, 500);
  app.title("mesh test");
  app.start();

  audioApp.exit();
  return 0;
}