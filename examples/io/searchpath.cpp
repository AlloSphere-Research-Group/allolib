/*
Allocore Example: searchpath

Description:
*/

#include <iostream>
#include "al/io/al_File.hpp"

using namespace al;
using namespace std;

int main() {
  SearchPaths searchpaths;
  searchpaths.addAppPaths();
  searchpaths.print();

  cout << "-----" << endl;
  searchpaths.addRelativePath("../../graphics");
  searchpaths.print();

  cout << "-----" << endl;
  File file1(searchpaths.find("2D.cpp"), "r", true);
  File file2(searchpaths.find("fbo.cpp"), "r", true);
  printf("file1 %s\n", file1.path().c_str());
  printf("file2 %s\n", file2.path().c_str());
}
