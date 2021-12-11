/*    Gamma - Generic processing library
    See COPYRIGHT file for authors and license information

    Example:
    Description:
*/

#include <cstdio>       // for printing to stdout
#define GAMMA_H_INC_ALL // define this to include all header files
#define GAMMA_H_NO_IO   // define this to avoid bringing AudioIO from Gamma

#include "Gamma/Gamma.h"
#include "al/app/al_DistributedApp.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/io/al_AudioIO.hpp"
#include "al/scene/al_DistributedScene.hpp"

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
  virtual void init() override {
    // Pose and size are not transmitted by default, so they must be explicitly
    // shared
    registerParameter(parameterPose());
    registerParameter(parameterSize());
  }

  virtual void update(double dt) override {
    if (!mIsReplica) {
      Pose p = pose();
      p.vec().x = p.vec().x * 0.993;
      setPose(p);
      setSize(size() * 0.997);
      if (fabs(pose().vec().x) < 0.01) {
        free();
      }
    }
  }

  virtual void onProcess(Graphics &g) override {
    auto *mesh = (Mesh *)userData();
    g.color(fabs(pose().vec().x));
    g.draw(*mesh);
  }
};

class MyApp : public DistributedApp {
public:
  ParameterPose navParameter{"nav"};

  virtual void onCreate() override {
    scene1.registerSynthClass<SimpleVoice>();
    scene2.registerSynthClass<SimpleVoice>();

    registerDynamicScene(scene1); // scene1 is shared from primary node

    {
      // Now connect scene2 so that it is shared from replica.
      // This is what registerDynamicScene does behind the scenes,
      // But we do it in reverse for scene2
      if (isPrimary()) {
        parameterServer().registerOSCConsumer(&scene2, scene2.name());
        scene1.allNotesOff(); // To turn off any events that might remain in a
                              // replica scene
      } else {
        scene2.registerNotifier(parameterServer());
        parameterServer().addListener("localhost", 9010);
        scene2.allNotesOff(); // To turn off any events that might remain in a
                              // replica scene
      }
    }

    // Scene 1 will display a disc
    addDisc(mMesh1, 0.5);
    mMesh1.primitive(Mesh::LINE_STRIP);
    mMesh1.update();
    scene1.setDefaultUserData(&mMesh1); // Share the mesh between all voices

    // Scene 2 displays a tetrahedron
    addTetrahedron(mMesh2);
    mMesh2.primitive(Mesh::LINE_STRIP);
    mMesh2.update();
    scene2.setDefaultUserData(&mMesh2); // Share the mesh between all voices

    title("Two Scenes");
    parameterServer() << navParameter; // Synchronize nav
    parameterServer().print();
    //    parameterServer().verbose(true);
  }

  virtual void onAnimate(double dt) override {
    counter++;
    if (counter >= 30) {
      // Regularly add voices to scene depending on app's role
      counter = 0;
      if (isPrimary()) {
        //        std::cout << "Added voice for scene 1" << std::endl;
        // Only primary node triggers voice
        auto voice = scene1.getVoice<SimpleVoice>();
        voice->setPose({Vec3d(1.0, 0.0, -3.0), Quatd()});
        voice->setSize(1.0f);
        scene1.triggerOn(voice);
      } else {
        //        std::cout << "Added voice for scene 2" << std::endl;
        // Only replica node triggers voice for scene2
        auto voice = scene2.getVoice<SimpleVoice>();
        voice->setPose({Vec3d(-1.0, 0.0, -3.0), Quatd()});
        voice->setSize(1.0f);
        scene2.triggerOn(voice);
      }
    }
    scene1.update(dt);
    scene2.update(dt);
    if (isPrimary()) {
      navParameter.set(nav());
    } else {
      pose() = navParameter.get();
    }
  }

  virtual void onDraw(Graphics &g) override {
    g.clear(0);
    scene1.render(g); // Render graphics
    scene2.render(g); // Render graphics
  }

  virtual void onExit() override {
    if (isPrimary()) {
      scene1.allNotesOff(); // To turn off any events that might remain in a
                            // replica scene
    } else {
      scene2.allNotesOff(); // To turn off any events that might remain in a
                            // replica scene
    }
  }

  DistributedScene scene1{"scene1", 0, TimeMasterMode::TIME_MASTER_GRAPHICS};
  DistributedScene scene2{"scene2", 0, TimeMasterMode::TIME_MASTER_GRAPHICS};

  VAOMesh mMesh1;
  VAOMesh mMesh2;

  int counter{0};
};

int main() {
  MyApp app;
  app.fps(30);
  app.start();
  return 0;
}
