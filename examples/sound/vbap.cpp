#include "al/app/al_App.hpp"
#include "al/sound/al_Vbap.hpp"
#include "al/sound/al_Speaker.hpp"
#include "al/math/al_StdRandom.hpp"
#include "al/graphics/al_Shapes.hpp"

#include <memory>
#include <atomic>
#include <vector>

#define BLOCK_SIZE (2048)

static al::SpeakerLayout StereoLayout (unsigned int deviceChannelStart=0, float angle=30.f, float distance=1.f, float gain=1.f)
{
  al::Speaker mLeft(deviceChannelStart, angle, 0, 0, distance, gain);
  al::Speaker mRight(deviceChannelStart + 1, -angle, 0, 0, distance, gain);

  al::SpeakerLayout layout;
  layout.addSpeaker(mLeft);
  layout.addSpeaker(mRight);
  return layout;
}

static al::SpeakerLayout ringLayout (unsigned int N, unsigned int deviceChannelStart=0, float phase=0.f, float radius=1.f, float gain=1.f)
{
  al::SpeakerLayout layout;
	layout.speakers().reserve(N);
  for(unsigned int i=0; i<N; ++i)
    layout.addSpeaker(al::Speaker(i+deviceChannelStart, 360.f/N*i + phase, 0.f, 0, radius, gain));
  return layout;
}

using namespace al;

struct MyApp : public App
{
  Mesh mSpeakerMesh;
  std::vector<Mesh> mVec;
  std::vector<int> sChannels;
  SpeakerLayout speakerLayout;
  std::unique_ptr<Vbap> panner;
  std::unique_ptr<std::atomic<float>[]> mPeaks;
  float speedMult = 0.03f;
  Vec3d srcpos {0.0,0.0,0.0};

  void onInit () override {

    speakerLayout = ringLayout(16);

    panner = std::make_unique<Vbap>(speakerLayout, true);
    panner->set3D(false);
    panner->makePhantomChannel(12, {0,1,2,3,4,5,6,7,8,9,10,11}); // assigned speakers top
    panner->makePhantomChannel(13, {0,1,2,3,4,5,6,7,8,9,10,11}); // assigned speakers bottom
    panner->compile();
    panner->print();

    //Determine number of output channels
    Speakers allSpeakers = speakerLayout.speakers();
    int highestChannel = 0;
    for(Speaker s:allSpeakers){
      if((int) s.deviceChannel > highestChannel){
          highestChannel = s.deviceChannel;
      }
    }

    audioIO().close();
    audioIO().channelsOut(highestChannel + 1);
    audioIO().open();

    panner->prepare(audioIO());
    mPeaks = std::make_unique<std::atomic<float>[]>(speakerLayout.speakers().size());
    for (size_t i = 0; i < speakerLayout.speakers().size(); i += 1) {
      mPeaks[i].store(0.0f);
    }

    std::vector<SpeakerTriple> sts = panner->triplets();

    //Setup the triangles
    for(int j = 0; j < sts.size();++j){
      SpeakerTriple &s = sts[j];
      Mesh mesh;
      mesh.primitive(Mesh::TRIANGLES);

      Color c(rnd::uniform(1.f,0.1f), rnd::uniform(), rnd::uniform(),0.4f);
      for(int i = 0;i < 3;++i){
        mesh.vertex(float(-s.vec[i][1]), float(s.vec[i][2]), float(-s.vec[i][0]));
        mesh.color(c);
      }
      mesh.decompress();
      mesh.generateNormals();
      mVec.push_back(mesh);
    }

    addSphere(mSpeakerMesh, 1.0, 5, 5);
  }

  void onCreate () override {
    nav().pos(0, 1, 20);
  }

  void onAnimate (double dt) override {
    nav().faceToward(Vec3f(0,0,0)); // Always face origin
  }

  void onDraw (Graphics& g) override {
    g.clear(0);
    g.blending(true);
    g.blendModeAdd();

    //Draw the triangles
    g.meshColor();
    g.polygonFill();
    for(int i = 0; i < mVec.size(); ++i){
      g.draw(mVec[i]);
    }

    //Draw the source
    g.pushMatrix();
    g.translate(srcpos);
    g.scale(0.3f);
    g.color(0.4f,0.4f, 0.4f, 0.5f);
    g.draw(mSpeakerMesh);
    g.popMatrix();

    // Draw line
    Mesh lineMesh;
    lineMesh.vertex(0.0f,0.0f, 0.0f);
    lineMesh.vertex(float(srcpos.x),0.0f, float(srcpos.z));
    lineMesh.vertex(srcpos);
    lineMesh.index(0);
    lineMesh.index(1);
    lineMesh.index(1);
    lineMesh.index(2);
    lineMesh.index(2);
    lineMesh.index(0);
    lineMesh.primitive(Mesh::LINES);
    g.color(1);
    g.draw(lineMesh);

    //Draw the speakers
    for(int i = 0; i < speakerLayout.numSpeakers(); ++i){
      g.pushMatrix();
      g.translate(speakerLayout.speakers()[i].vecGraphics());
      float peak = mPeaks[i].load();
      g.scale(0.02f + peak * 6.0f);
      g.polygonLine();
      g.color(1.0f);
      g.draw(mSpeakerMesh);
      g.popMatrix();
    }
  }

  void onSound(AudioIOData &io) override {
    static unsigned int t = 0;
    double sec;
    float srcBuffer[BLOCK_SIZE];

    while (io()) {
      int i = io.frame();
      float env = (22050 - (t % 22050))/22050.0f;
      sec = (t / io.fps());
      // Signal is computed every sample
      srcBuffer[i] = 0.5f * rnd::uniform() * env;
      ++t;
    }
    // But the positions can be computed once per buffer
    float tta = float(sec*speedMult*M_2PI + M_2PI);
    float x = 6.0f*cos(tta);
    float y = 6.0f*sin(2.8f * tta);
    float z = 6.0f*sin(tta);

    srcpos = Vec3d(x,y,z);

    panner->renderBuffer(io, srcpos, srcBuffer, BLOCK_SIZE);

    // Now compute RMS to display the signal level for each speaker
    Speakers &speakers = speakerLayout.speakers();
    for (int i = 0; i < speakers.size(); i++) {
      mPeaks[i].store(0);
    }
    for (int speaker = 0; speaker < speakers.size(); speaker++) {
      float rms = 0;
      for (unsigned int i = 0; i < io.framesPerBuffer(); i++) {
        unsigned int deviceChannel = speakers[speaker].deviceChannel;
        if(deviceChannel < io.channelsOut()) {
          float sample = io.out(speakers[speaker].deviceChannel, i);
          rms += sample * sample;
        }
      }
      rms = sqrt(rms/io.framesPerBuffer());
      mPeaks[speaker].store(rms);
    }
  }
};

int main ()
{
  MyApp app;

  AudioDevice::printAll();
  app.initAudio(44100, BLOCK_SIZE, 2, 0);
  app.start();

  return 0;
}



