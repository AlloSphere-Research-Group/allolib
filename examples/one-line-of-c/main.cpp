#include "al/core.hpp"
#include "al/util/imgui/al_Imgui.hpp"
using namespace al;

using std::cout;
using std::endl;

#include "libtcc.h"

inline float mtof(float m) { return 8.175799f * powf(2.0f, m / 12.0f); }
inline float dbtoa(float db) { return 1.0f * powf(10.0f, db / 20.0f); }

const char* starterCode = R"(
char foo(int t) {
  return t; // sawtooth wav

  // static int v = 0;
  // return (v=(v>>1)+(v>>4)+t*(((t>>16)|(t>>6))&(69&(t>>9))));
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

// Fabrice Bellard's Tiny C Compiler can compile simple C programs quickly and
// "in memory". Given a string, we create a callable function that generates a
// sequence of audio samples.
void tcc_error_handler(void* tcc, const char* msg);
struct TCC {
  using FunctionPointer = char (*)(int);
  FunctionPointer process = nullptr;
  TCCState* instance = nullptr;
  std::string error;

  void destroy() {
    if (instance) {
      tcc_delete(instance);
    }
  }

  bool compile(std::string source) {
    destroy();
    instance = tcc_new();
    assert(instance != nullptr);

    // set up the compiler
    tcc_set_options(instance, "-nostdinc -nostdlib -Wall -Werror");
    tcc_set_error_func(instance, this, tcc_error_handler);
    tcc_set_output_type(instance, TCC_OUTPUT_MEMORY);
    //

    if (tcc_compile_string(instance, source.c_str()) == -1) {
      //
      // error string is set by the TCC handler
      return false;
    }

    if (tcc_relocate(instance, TCC_RELOCATE_AUTO) < 0) {
      error = "failed to relocate code";
      return false;
    }

    FunctionPointer foo = (FunctionPointer)(tcc_get_symbol(instance, "foo"));
    if (foo == nullptr) {
      error = "could not find the symbol 'foo'";
      return false;
    }

    // maybe we should go a step further and try a few calls to see if it
    // crashes

    error = "";
    process = foo;
    return true;
  }

  float operator()(int t) {
    if (process == nullptr) return 0;
    char c = process(t);
    return c / 128.0f;
  }
};
void tcc_error_handler(void* tcc, const char* msg) {
  ((TCC*)tcc)->error = msg;
  // TODO:
  // - remove file name prefix which is "<string>"
  // - correct line number which is off by about 20
}

struct Appp : App {
  TCC tcc[2];
  unsigned active = 0;
  bool shouldSwap = false;
  char buffer[10000];
  char error[10000];
  float gain = 0;
  int t = 0;

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

    navControl().active(false);
  }

  void onDraw(Graphics& g) override {
    g.clear(0.1);

    static float db = -20;
    ImGui::SliderFloat(" ", &db, -60.0f, 0.0f);
    gain = dbtoa(db);

    ImGui::Separator();

    ImGui::InputInt("t", &t);

    ImGui::Separator();

    bool update =
        ImGui::InputTextMultiline("", buffer, sizeof(buffer), ImVec2(640, 480));

    if (update) {
      if (tcc[1 - active].compile(buffer)) {
        shouldSwap = true;
      }
    }

    ImGui::Separator();

    ImGui::Text(tcc[1 - active].error.c_str());

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

      s = gain * tcc[active](t);
      io.out(0) = s;
      io.out(1) = s;
      t++;
    }
  }
};

int main() {
  Appp a;
  a.dimensions(1200, 800);
  a.initAudio();
  a.start();
}
