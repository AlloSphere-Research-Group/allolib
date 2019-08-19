
#include "al/app/al_App.hpp"
#include "al/app/al_OmniRendererDomain.hpp"
#include <iostream>

using namespace std;
using namespace al;

struct DistributedApp_: public App {
public:
  void start() override {
    onInit();  // onInit() can't be called in constructor as it is virtual. But it
               // is good enough here.
    graphicsDomain()->removeSubDomain(mDefaultWindowDomain);
    auto omniRendering = graphicsDomain()->newSubDomain<GLFWOpenGLOmniRendererDomain>();
    omniRendering->window().append(stdControls);
    for (auto &domain : mDomainList) {
      mRunningDomains.push(domain);
      if (!domain->start()) {
        std::cerr << "ERROR starting domain " << std::endl;
        break;
      }
    }

    while (mRunningDomains.size() > 0) {
      if (!mRunningDomains.top()->stop()) {
        std::cerr << "ERROR stopping domain " << std::endl;
      }
      mRunningDomains.pop();
    }

    onExit();
    mDefaultWindowDomain = nullptr;
    for (auto &domain : mDomainList) {
      if (!domain->cleanup()) {
        std::cerr << "ERROR cleaning up domain " << std::endl;
      }
    }
  }
};

struct MyOmniRendererApp : DistributedApp_
{
    VAOMesh mesh;
    
    Nav mNav;
    Viewpoint mView {mNav.transformed()};
    NavInputControl mNavControl {mNav};

    bool DO_BLENDING = false;
    float alpha = 0.9;

    void onCreate() override {

        addIcosahedron(mesh);
        mesh.update();
    }

    void onAnimate(double dt) override {
        mNav.step();
    }

    void onDraw(Graphics& g) override {
        g.clear(0, 0, 1);

        if (DO_BLENDING) {
            g.depthTesting(false);
            g.blending(true);
            g.blendModeAdd();
        }
        else {
            g.depthTesting(true);
            g.depthMask(true);
            g.blending(false);
        }

        for(int aa = -5; aa <= 5; aa++)
        for(int bb = -5; bb <= 5; bb++)
        for(int cc = -5; cc <= 5; cc++)  {
            if(aa == 0 && bb == 0 && cc == 0) continue;
            g.pushMatrix();
            g.translate(aa * 2, bb * 2, cc * 2);
//            g.rotate(sin(2 * al::seconds()), 0, 0, 1);
//            g.rotate(sin(3 * sec()), 0, 1, 0);
            g.scale(0.3, 0.3, 0.3);
            if (DO_BLENDING) {
                g.color((aa + 5)/10.0, (bb + 5)/10.0, (cc + 5)/10.0, alpha);
            }
            else {
                g.color((aa + 5)/10.0, (bb + 5)/10.0, (cc + 5)/10.0);
            }
            g.draw(mesh);
            g.popMatrix();
        }
    }

    bool onKeyDown(const Keyboard& k) /*override*/ {
        if (k.key() == 'b') {
            DO_BLENDING = !DO_BLENDING;
            cout << "blending: " << DO_BLENDING << endl;
        }
        if (k.key() == 'n') {
            alpha = 1 - alpha;
            cout << "alpha: " << alpha << endl;
        }
        return true;
    }
};

int main()
{
    MyOmniRendererApp app;
    app.start();
}
