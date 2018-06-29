#include "al/core.hpp"
#include "al/util/al_Toml.hpp"

using namespace al;
using namespace std;

// in order to use global function based toml loading,
// al.toml file needs to be in data folder (in the `bin/data` folder)
// if not, TOML::get* function will throw runtime exception

struct MyApp : App
{
  void onCreate() override {
    cout << TOML::getd("my_float") << endl;
    cout << TOML::geti("nested", "my_int") << endl;
    cout << TOML::gets("nested", "my_str") << endl;
  }

  void onDraw(Graphics& g) override {
    g.clear(0.3f, 0.3f, 0.7f);
  }

};

int main()
{
  MyApp app;
  app.start();
}

