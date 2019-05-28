/*    Gamma - Generic processing library
    See COPYRIGHT file for authors and license information

    Example:
    Description:
*/

#include <cstdio>               // for printing to stdout
#define GAMMA_H_INC_ALL         // define this to include all header files
#define GAMMA_H_NO_IO           // define this to avoid bringing AudioIO from Gamma

#include "Gamma/Gamma.h"

#include "al/core/io/al_AudioIO.hpp"
#include "al/util/scene/al_DistributedScene.hpp"
#include "al/core/app/al_DistributedApp.hpp"
#include "al/core/graphics/al_Shapes.hpp"

using namespace gam;
using namespace al;

// This example shows how the DistributedScene class can be used
// to propagate changes in a DynamicScene across the network.
// Using DistributedApp together with DistributedScene, allows
// multiple instances of this same application binary to take
// on different roles. Try running two instances of this application
// on the same machine. The first will be the "primary" application
// controlling changes, and the second one will be a "replica"
// mirroring all the changes.

// See the examples for DynamicScene (dynamic_scene.cpp and
// avSequencer.cpp) for information on how it works.


// The Scene will contain "SimpleVoice" agents
class SimpleVoice : public PositionedVoice {
public:

  virtual void update(double dt) override {
    pose().vec().x = pose().vec().x * 0.993;
  }

  virtual void onProcess(Graphics &g) override {
    auto *mesh = (Mesh *) userData();
    g.color(fabs(pose().vec().x));
    g.draw(*mesh);
    if (fabs(pose().vec().x) < 0.1) {free();}
  }

};

class MyApp : public DistributedApp<>
{
public:

  virtual void onCreate() override {

    scene1.registerSynthClass<SimpleVoice>();
    scene2.registerSynthClass<SimpleVoice>();
    registerDynamicScene(scene1); // scene1 is broadcast from primary

    // Now connect scene2 so that it is broadcast from replica
    // If distributed scene, connect according to this app's role
    if (!isPrimary()) {
      scene2.registerNotifier(parameterServer());
      parameterServer().addListener("localhost", 9010);
    } else {
      parameterServer().registerOSCConsumer(
            &scene2, scene2.name());
    }

    addDisc(mMesh1, 0.5);
    mMesh1.primitive(Mesh::LINE_STRIP);
    mMesh1.update();
    scene1.setDefaultUserData((void *) &mMesh1);

    addTetrahedron(mMesh2);
    mMesh2.primitive(Mesh::LINE_STRIP);
    mMesh2.update();
    scene2.setDefaultUserData((void *) &mMesh2);

    if (isPrimary()) {
      title("Primary");
    } else {
      title("Replica");
    }
    parameterServer().print();
    parameterServer().verbose(true);
  }

  virtual void onAnimate(double dt) override {
    counter++;
    if (counter == 30) {
      // Regularly add voices to scene depending on app's role
      counter = 0;
      if (isPrimary()) {
//        std::cout << "Added voice for scene 1" << std::endl;
        // Only primary node triggers voice
        auto voice = scene1.getVoice<SimpleVoice>();
        voice->pose().pos() = {1.0, 0.0, -3.0};
        scene1.triggerOn(voice);
      } else {

//        std::cout << "Added voice for scene 2" << std::endl;
        // Only replica node triggers voice for scene2
        auto voice = scene2.getVoice<SimpleVoice>();
        voice->pose().pos() = {-1.0, 0.0, -3.0};
        scene2.triggerOn(voice);
      }
    }
    scene1.update(dt);
    scene2.update(dt);
  }

  virtual void onDraw(Graphics &g) override {
    g.clear();
    scene1.render(g); // Render graphics
    scene2.render(g); // Render graphics
  }

  DistributedScene scene1 {"scene1"};
  DistributedScene scene2 {"scene2"};

  VAOMesh mMesh1;
  VAOMesh mMesh2;

  int counter {0};
};

int main(){
  // Create app instance
  MyApp app;
  app.start();
}
