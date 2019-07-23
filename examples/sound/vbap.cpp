#include "al/app/al_App.hpp"
#include "al/sound/al_Vbap.hpp"
#include "al/sound/al_Speaker.hpp"
#include "al/math/al_StdRandom.hpp"
#include "al/graphics/al_Shapes.hpp"

#include <memory>
#include <atomic>
#include <vector>

#define BLOCK_SIZE (2048)

/// Current arrangement of speakers in AlloSphere
al::SpeakerLayout alloSphereSpeakerLayout () {
  const int numSpeakers = 54;
  al::Speaker alloSpeakers[numSpeakers] = {
    al::Speaker(1-1, -77.660913f, 41.000000f, 4.992118f),
    al::Speaker(2-1, -45.088015f, 41.000000f, 5.571107f),
    al::Speaker(3-1, -14.797289f, 41.000000f, 5.900603f),
    al::Speaker(4-1, 14.797289f, 41.000000f, 5.900603f),
    al::Speaker(5-1, 45.088015f, 41.000000f, 5.571107f),
    al::Speaker(6-1, 77.660913f, 41.000000f, 4.992118f),
    al::Speaker(7-1, 102.339087f, 41.000000f, 4.992118f),
    al::Speaker(8-1, 134.911985f, 41.000000f, 5.571107f),
    al::Speaker(9-1, 165.202711f, 41.000000f, 5.900603f),
    al::Speaker(10-1, -165.202711f, 41.000000f, 5.900603f),
    al::Speaker(11-1, -134.911985f, 41.000000f, 5.571107f),
    al::Speaker(12-1, -102.339087f, 41.000000f, 4.992118f),
    al::Speaker(17-1, -77.660913f, 0.000000f, 4.992118f),
    al::Speaker(18-1, -65.647587f, 0.000000f, 5.218870f),
    al::Speaker(19-1, -54.081600f, 0.000000f, 5.425483f),
    al::Speaker(20-1, -42.869831f, 0.000000f, 5.604350f),
    al::Speaker(21-1, -31.928167f, 0.000000f, 5.749461f),
    al::Speaker(22-1, -21.181024f, 0.000000f, 5.856274f),
    al::Speaker(23-1, -10.559657f, 0.000000f, 5.921613f),
    al::Speaker(24-1, 0.000000f, 0.000000f, 5.943600f),
    al::Speaker(25-1, 10.559657f, 0.000000f, 5.921613f),
    al::Speaker(26-1, 21.181024f, 0.000000f, 5.856274f),
    al::Speaker(27-1, 31.928167f, 0.000000f, 5.749461f),
    al::Speaker(28-1, 42.869831f, 0.000000f, 5.604350f),
    al::Speaker(29-1, 54.081600f, 0.000000f, 5.425483f),
    al::Speaker(30-1, 65.647587f, 0.000000f, 5.218870f),
    al::Speaker(31-1, 77.660913f, 0.000000f, 4.992118f),
    al::Speaker(32-1, 102.339087f, 0.000000f, 4.992118f),
    al::Speaker(33-1, 114.352413f, 0.000000f, 5.218870f),
    al::Speaker(34-1, 125.918400f, 0.000000f, 5.425483f),
    al::Speaker(35-1, 137.130169f, 0.000000f, 5.604350f),
    al::Speaker(36-1, 148.071833f, 0.000000f, 5.749461f),
    al::Speaker(37-1, 158.818976f, 0.000000f, 5.856274f),
    al::Speaker(38-1, 169.440343f, 0.000000f, 5.921613f),
    al::Speaker(39-1, 180.000000f, 0.000000f, 5.943600f),
    al::Speaker(40-1, -169.440343f, 0.000000f, 5.921613f),
    al::Speaker(41-1, -158.818976f, 0.000000f, 5.856274f),
    al::Speaker(42-1, -148.071833f, 0.000000f, 5.749461f),
    al::Speaker(43-1, -137.130169f, 0.000000f, 5.604350f),
    al::Speaker(44-1, -125.918400f, 0.000000f, 5.425483f),
    al::Speaker(45-1, -114.352413f, 0.000000f, 5.218870f),
    al::Speaker(46-1, -102.339087f, 0.000000f, 4.992118f),
    al::Speaker(49-1, -77.660913f, -32.500000f, 4.992118f),
    al::Speaker(50-1, -45.088015f, -32.500000f, 5.571107f),
    al::Speaker(51-1, -14.797289f, -32.500000f, 5.900603f),
    al::Speaker(52-1, 14.797289f, -32.500000f, 5.900603f),
    al::Speaker(53-1, 45.088015f, -32.500000f, 5.571107f),
    al::Speaker(54-1, 77.660913f, -32.500000f, 4.992118f),
    al::Speaker(55-1, 102.339087f, -32.500000f, 4.992118f),
    al::Speaker(56-1, 134.911985f, -32.500000f, 5.571107f),
    al::Speaker(57-1, 165.202711f, -32.500000f, 5.900603f),
    al::Speaker(58-1, -165.202711f, -32.500000f, 5.900603f),
    al::Speaker(59-1, -134.911985f, -32.500000f, 5.571107f),
    al::Speaker(60-1, -102.339087f, -32.500000f, 4.992118f),
  };

  al::SpeakerLayout layout;
  layout.speakers().reserve(numSpeakers);
  for(int i=0; i<numSpeakers; ++i)
  {
    layout.addSpeaker(alloSpeakers[i]);
  }
  return layout;
}

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

    speakerLayout = alloSphereSpeakerLayout();

    panner = std::make_unique<Vbap>(speakerLayout, true);
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



