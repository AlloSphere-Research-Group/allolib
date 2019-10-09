#include <iostream>
#include "al/io/al_File.hpp"

using namespace al;
using namespace std;

int main(int argc, char* argv[]) {
  auto list = itemListInDir(".");
  for (int i = 0; i < list.count(); i += 1) {
    auto const& name = list[i].file();
    if (File::isDirectory(name)) {
      cout << list[i].file() << '/' << endl;
    } else {
      cout << list[i].file() << endl;
    }
  }
}
