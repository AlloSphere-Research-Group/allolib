#include "al/core.hpp"
using namespace al;

#include "libtcc.h"

const char* sourceCode = R"(

char foo(int t) {
  // return (t*((t>>12|t>>8)&63&t>>4));
  // return (t*((t>>5|t>>8)>>(t>>16)));
  // return (t*((t>>9|t>>13)&25&t>>6));
  // return (t*(t>>11&t>>8&123*t>>3));
  // return (t*(t>>8*(t>>15|t>>8)&(20|(t>>19)*5>>t|t>>3)));
  return (((-t&4095)*(255&t*(t&t>>13))>>12)+(127&t*(234&t>>8&t>>3)>>(3&t>>14)));
  // return (t*(t>>((t>>9|t>>8))&63&t>>4));
  // return ((t>>6|t|t>>(t>>16))*10+((t>>11)&7));
  // return ((t|(t>>9|t>>7))*t&(t>>11|t>>9));
  // return (t*5&(t>>7)|t*3&(t*4>>10));
  // return ((t>>7|t|t>>6)*10+4*(t&t>>13|t>>6));
  // return (((t&4096)?((t*(t^t%255)|(t>>4))>>1):(t>>3)|((t&8192)?t<<2:t)));
  // return (((t*(t>>8|t>>9)&46&t>>8))^(t&t>>13|t>>6));
  //static int v = 0; return (v=(v>>1)+(v>>4)+t*(((t>>16)|(t>>6))&(69&(t>>9))));
}

)";

struct Appp : App {
  TCCState* instance;
  char (*process)(int) = nullptr;
  bool clean = false;
  float data[8000];
  int t = 0;

  void onCreate() override {
    instance = tcc_new();
    assert(instance != nullptr);
    tcc_set_output_type(instance, TCC_OUTPUT_MEMORY);
    //
  }

  void recompileMaybe() {
    if (clean)  //
      return;
    if (tcc_compile_string(instance, sourceCode) == -1) {
      // show compile error
      //
      return;
    }
    if (tcc_relocate(instance, TCC_RELOCATE_AUTO) < 0) {
      // fail
      return;
    }
    char (*foo)(int);
    foo = (char (*)(int))(tcc_get_symbol(instance, "foo"));
    if (foo == nullptr) {
      // fail
      return;
    }

    // if we got here ...
    // for (int t = 0; t < 8000; t++) data[t] = foo(t) / 181.046676f;

    clean = true;
    process = foo;
    t = 0;
    // for (int t = 0; t < 8000; t++) printf("%f\n", data[t]);
  }

  void onAnimate(double dt) override {
    recompileMaybe();
    //
  }

  void onSound(AudioIOData& io) override {
    //
    while (io()) {
      float s = 0;
      if (process != nullptr) s = process(t++);
      io.out(0) = s;
      io.out(1) = s;
    }
  }

  void onDraw(Graphics& g) override {
    g.clear(0.1);
    //
  }
};

int main() {
  Appp a;
  a.initAudio();
  a.start();
}
