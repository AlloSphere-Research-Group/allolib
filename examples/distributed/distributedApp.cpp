/*
Example: DistributedApp

Description:
Demontration of basic usage of the DistributedApp class

Author:
Andres Cabrera 2/2018
*/

// To run this example on a distributed cluster, you first need to copy the
// built binary to the cluster's shared file system, and then run it with mpirun.
// You can use a command like:
// mpirun -n 4 -f host_file ./distributedApp
// -n 4 determines the number of precesses launched, and -f host_file
// lists the machines where things will run. It should look like:
//
// Look at http://mpitutorial.com/tutorials/mpi-hello-world/ for more details on
// running MPI applications.


#include <stdio.h>

#include "al/app/al_DistributedApp.hpp"
#include "al/math/al_Random.hpp"

using namespace al;

struct SharedState {
  float value1;
  int value2;
};

class DistributedExampleApp : public DistributedApp<SharedState> {
public:

  Parameter value {"value", "", 0.1f};
  Mesh sphere;

  void onInit() override {
    std::cout << "I am '" << name() << "' role: " << roleName() << std::endl;
    parameterServer() << value;
    if (hasRole(ROLE_RENDERER)) {
      value.registerChangeCallback([](float newValue) { std::cout << "Got value " << newValue << std::endl;});
    }

    addSphere(sphere);
    sphere.primitive(Mesh::LINES);
  }

  // The simulate function is only run for the simulator
  // So there is no need to check role
  // Updating state will only work if cuttlebone is available
  // but parameters will always be synchronized
  virtual void simulate(double dt) override {
    state().value1 = rnd::uniform();
    state().value2 = int(state().value1 * 1000.f);
    std::cout << "Updating state ... " << state().value1 << std::endl;
  }

  virtual void onDraw(Graphics &g) override {
    if (hasRole(ROLE_RENDERER)) {
      if (newStates()> 0) {
        // Renderer will recieve state from simulator
        std::cout << " Renderer got state : " << state().value1 << std::endl;
      }
    }
    g.clear(0);
    g.pushMatrix();
    g. translate(0,0, -4);
    g.scale(value);
    g.draw(sphere);
    g.popMatrix();
  }

  virtual void onSound(AudioIOData &io) override {
    if (hasRole(ROLE_AUDIO) || hasRole(ROLE_DESKTOP)) {
      // Audio will recieve state from simulator
      std::cout << " Audio got : " << state().value1 << std::endl;
    }
  }

  void onKeyDown(Keyboard const &k) override {
    value = value + 0.1f;
    std::cout << "value set to " << value << std::endl;
  }

};

int main(){
  DistributedExampleApp app;
  app.fps(1); // Only call simulate and draw once per second
  app.startFPS();
  app.print();
  for (int i = 0; i < 10; i++) {
    if (app.isPrimary()) {
      std::cout << "   Call simulate " << i << " ---------------" <<std::endl;
      app.simulate(0);
    }
  }
  app.start();
  return 0;
}
