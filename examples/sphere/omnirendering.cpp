#include "al/core.hpp"
#include "al/sphere/al_OmniRenderer.hpp"

using namespace std;
using namespace al;

struct MyOmniRendererApp : OmniRenderer
{
    VAOMesh mesh;
    
    Nav mNav;
    Viewpoint mView {mNav.transformed()};
    NavInputControl mNavControl {mNav};

    void onCreate() override {
        append(mNavControl);

        addIcosahedron(mesh);
        mesh.update();
    }

    void onAnimate(double dt) override {
        mNav.step();
        pose(mView.pose()); // should not be in onDraw
    }

    void onDraw(Graphics& g) override {
        g.clear(0);

        for(int aa = -5; aa <= 5; aa++)
        for(int bb = -5; bb <= 5; bb++)
        for(int cc = -5; cc <= 5; cc++)  {
            if(aa == 0 && bb == 0 && cc == 0) continue;
            g.pushMatrix();
            g.translate(aa * 2, bb * 2, cc * 2);
            g.rotate(sin(2 * sec()), 0, 0, 1);
            g.rotate(sin(3 * sec()), 0, 1, 0);
            g.scale(0.3, 0.3, 0.3);
            g.color((aa + 5)/10.0, (bb + 5)/10.0, (cc + 5)/10.0);
            g.draw(mesh);
            g.popMatrix();
        }
    }

};

int main()
{
    MyOmniRendererApp app;
    app.start();
}