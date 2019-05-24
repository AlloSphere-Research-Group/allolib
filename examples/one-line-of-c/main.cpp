#include "al/core.hpp"
#include "al/util/imgui/al_Imgui.hpp"
using namespace al;

using std::cout;
using std::endl;

#include "libtcc.h"

const char* starterCode = R"(
char foo(int t) {
  static int v = 0;
  return (v=(v>>1)+(v>>4)+t*(((t>>16)|(t>>6))&(69&(t>>9))));
  // return (t*((t>>12|t>>8)&63&t>>4));
  // return (t*((t>>5|t>>8)>>(t>>16)));
  // return (t*((t>>9|t>>13)&25&t>>6));
  // return (t*(t>>11&t>>8&123*t>>3));
  // return (t*(t>>8*(t>>15|t>>8)&(20|(t>>19)*5>>t|t>>3)));
  // return (((-t&4095)*(255&t*(t&t>>13))>>12)+(127&t*(234&t>>8&t>>3)>>(3&t>>14)));
  // return (t*(t>>((t>>9|t>>8))&63&t>>4));
  // return ((t>>6|t|t>>(t>>16))*10+((t>>11)&7));
  // return ((t|(t>>9|t>>7))*t&(t>>11|t>>9));
  // return (t*5&(t>>7)|t*3&(t*4>>10));
  // return ((t>>7|t|t>>6)*10+4*(t&t>>13|t>>6));
  // return (((t&4096)?((t*(t^t%255)|(t>>4))>>1):(t>>3)|((t&8192)?t<<2:t)));
  // return (((t*(t>>8|t>>9)&46&t>>8))^(t&t>>13|t>>6));
}
)";

//
struct TCC {
  TCCState* instance = nullptr;
  char (*process)(int) = nullptr;
  unsigned t = 0;

  void destroy() {
    if (instance) {
      cout << "Destroying Compiler..." << endl;
      tcc_delete(instance);
    }
  }

  bool compile(std::string source) {
    destroy();
    cout << "Creating Compiler..." << endl;
    instance = tcc_new();
    assert(instance != nullptr);
    tcc_set_output_type(instance, TCC_OUTPUT_MEMORY);
    //

    if (tcc_compile_string(instance, source.c_str()) == -1) {
      //
      return false;
    }

    if (tcc_relocate(instance, TCC_RELOCATE_AUTO) < 0) {
      // fail
      return false;
    }

    char (*foo)(int) = (char (*)(int))(tcc_get_symbol(instance, "foo"));
    if (foo == nullptr) {
      // fail
      return false;
    }

    cout << "Compile Succeeded..." << endl;

    process = foo;
    // t = 0;
    return true;
  }

  float operator()() {
    if (process == nullptr) return 0;
    char c = process(t);
    t = 1 + t;
    return c / 128.0f;
  }
};

struct Appp : App {
  TCC tcc[2];
  unsigned active = 0;
  bool shouldSwap = false;
  char buffer[10000];
  bool show_gui = true;

  Appp() {
    // start out with some code
    strcpy(buffer, starterCode);
  }

  void onExit() override { shutdownIMGUI(); }
  void onCreate() override {
    initIMGUI();
    tcc[active].compile(buffer);
  }

  void onAnimate(double dt) override {
    beginIMGUI();
    auto& io = ImGui::GetIO();
    bool using_gui =
        io.WantCaptureMouse | io.WantCaptureKeyboard | io.WantTextInput;

    navControl().active(!using_gui);
  }

  void onDraw(Graphics& g) override {
    g.clear(0.1);
    if (ImGui::InputTextMultiline("", buffer, sizeof(buffer),
                                  ImVec2(640, 480))) {
      if (tcc[1 - active].compile(buffer)) {
        shouldSwap = true;
      }
    }
    endIMGUI();
    //
  }

  void onSound(AudioIOData& io) override {
    //
    while (io()) {
      float s = 0;

      if (shouldSwap) {
        active = 1 - active;  // the only spot we change this
        shouldSwap = false;
      }

      s = tcc[active]();
      io.out(0) = s;
      io.out(1) = s;
    }
  }
};

int main() {
  Appp a;
  a.initAudio();
  a.start();
}
