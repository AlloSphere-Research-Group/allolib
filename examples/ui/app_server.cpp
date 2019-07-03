
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"

#include "al/util/ui/al_Parameter.hpp"
#include "al/util/ui/al_ParameterServer.hpp"
#include "al/util/ui/al_ControlGUI.hpp"

using namespace al;

class MyApp : public App {
public:

    Parameter X {"X", "", 0.0f, "", -1.0f, 1.0f};
    Parameter Y {"Y", "", 0.0f, "", -1.0f, 1.0f};

    MyApp() {
        navControl().disable(); // Disable Keyboard and mouse navigaion
    }

    void onCreate() override {
        // Add Cone shape to mesh
        addCone(mesh);
        mesh.primitive(Mesh::LINE_STRIP);
        // Register parameters with GUI
        gui << X << Y;
        // Initialize control GUI
        gui.init();
        // Register parameters with App's internal parameter server
        parameterServer() << X << Y;
        parameterServer().print();
        // You can change the port and listening address of the parameter server at runtime
        parameterServer().listen(9011, "localhost");
        parameterServer().print();
    }

    void onDraw(Graphics &g) override {
        g.clear();
        g.pushMatrix();
        g.translate(X, Y, -8.0);
        g.draw(mesh);
        g.popMatrix();
        gui.draw(g);
    }

    void onKeyDown(Keyboard const& k) override {
        // Use the 'd' and 'a' keys to change the X value
        // This sends an OSC message that will be receieved by the
        // parameter server
        if (k.key() == 'd') {
            osc::Send sender(parameterServer().serverPort());
            sender.send("/X", X + 0.1f);
        }

        if (k.key() == 'a') {
            osc::Send sender(parameterServer().serverPort());
            sender.send("/X", X - 0.1f);
        }

    }

private:
    ControlGUI gui;

    Mesh mesh;
};


int main()
{
    MyApp app;
    app.start();

    return 0;
}
