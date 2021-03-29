/*
  Allocore Example: CIE Color Spaces

  Description:
  This program demonstrates how to use the HCLab and HCLuv cylindrical color
  spaces.

  Author:
  Owen Campbell, 5/2014 (owen.campbell at gmail dot com)
  Adapted from Lance Putnam's 2D Drawing example code
*/

#include "al/app/al_App.hpp"
#include "al/math/al_Random.hpp"

using namespace al;
using namespace std;

enum PROGRAM_MODE {
  VARYING_HUE,
  VARYING_CHROMA,
  VARYING_LUMINANCE,
  FIXED_HUE,
  FIXED_CHROMA,
  FIXED_LUMINANCE,
  NUM_MODES
};

PROGRAM_MODE MODE = VARYING_LUMINANCE;

enum COLOR_TYPE { HCLAB, HCLUV, NUM_TYPES };

COLOR_TYPE TYPE = HCLAB;

struct CieColor : public App {
  int numRows = 4, numCols = 4;

  Mesh verts{Mesh::TRIANGLES};
  // rnd::Random<rnd::MulLinCon> rng;
  rnd::Random<rnd::LinCon> rng;
  // rnd::Random<rnd::Tausworthe> rng;
  bool updateScene = true;

  void printInstructions() {
    // print controls to terminal
    //
    cout
        << "***********************************************************" << endl
        << "****               RGB SWATCH GENERATOR:               ****" << endl
        << "***********************************************************" << endl
        << "****                                                   ****" << endl
        << "****  PERCEPTUALLY LINEAR HUE GRADIENTS           '1'  ****" << endl
        << "****  PERCEPTUALLY LINEAR CHROMA GRADIENTS        '2'  ****" << endl
        << "****  PERCEPTUALLY LINEAR LUMINANCE GRADIENTS     '3'  ****" << endl
        << "****  FIXED HUE                                   '4'  ****" << endl
        << "****  FIXED CHROMA                                '5'  ****" << endl
        << "****  FIXED LUMINANCE                             '6'  ****" << endl
        << "****                                                   ****" << endl
        << "****  DECREASE NUMBER OF ROWS                     '-'  ****" << endl
        << "****  INCREASE NUMBER OF ROWS                     '+'  ****" << endl
        << "****  DECREASE NUMBER OF COLUMNS                  '<'  ****" << endl
        << "****  INCREASE NUMBER OF COLUMNS                  '>'  ****" << endl
        << "****                                                   ****" << endl
        << "****  USE CIE L*ab / HCL(ab) COLOR SPACE          'b'  ****" << endl
        << "****  USE CIE L*uv / HCL(uv) COLOR SPACE          'v'  ****" << endl
        << "****                                                   ****" << endl
        << "****      (RGB VALUES [0, 255] PRINT TO CONSOLE)       ****" << endl
        << "****                                                   ****" << endl
        << "***********************************************************" << endl
        << "***********************************************************" << endl
        << "***********************************************************"
        << endl;
  }

  void onCreate() override { printInstructions(); }

  bool onKeyDown(const Keyboard &k) override {
    // cout << "KEY PRESSED: " << k.key() << endl;
    updateScene = true;
    // change gradient type
    if (k.key() >= '1' && k.key() - 49 <= NUM_MODES - 1) {
      MODE = (PROGRAM_MODE)(k.key() - 49);
    }
    // change number of rows
    else if (k.key() == '-' || k.key() == '_' || k.key() == '+' ||
             k.key() == '=') {
      numRows += (k.key() == '-' || k.key() == '_') ? -1 : 1;
      if (numRows < 1)
        numRows = 1;
      if (numRows > 10)
        numRows = 10;
    }
    // change number of columns
    else if (k.key() == '<' || k.key() == ',' || k.key() == '>' ||
             k.key() == '.') {
      numCols += (k.key() == '<' || k.key() == ',') ? -1 : 1;
      if (numCols < 1)
        numCols = 1;
      if (numCols > 10)
        numCols = 10;
    }
    // change color space
    else if (k.key() == 'b' || k.key() == 'v') {
      TYPE = (k.key() == 'b') ? HCLAB : HCLUV;
    } else {
      updateScene = false;
    }
    return true;
  }

  void onDraw(Graphics &g) override {
    if (updateScene) {
      // pick new swatches

      // clear polygons and re-seed random number generator
      verts.reset();
      rng.seed(rnd::seed());
      float variedParam1, variedParam2, fixedParam1, fixedParam2;
      RGB swatch;
      float width = 1.0f / numCols / 2;
      float height = 1.0f / numRows / 2;
      float widthOffset = width / 2.0f;
      float heightOffset = height / 2.0f;
      if (MODE > 2) {
        // randomly select two of the three parameters
        // these will stay constant across the whole
        // grid for modes 3 - 6
        fixedParam1 = rng.uniform();
        fixedParam2 = rng.uniform();
      }
      // Create grid
      for (int i = 1; i <= numCols * 2; i += 2) {
        // NOTE: uniform distribution produces reliable gradients
        //      but is more likely to produce duplicates.
        //      deviating from uniform distribution produces fewer
        //      duplicate gradients but exposes holes in gamut

        if (MODE < 3) {
          // randomly select two of the three parameters
          // these will change for each gradient for modes 3 -6
          fixedParam1 = rng.uniform();
          fixedParam2 = rng.uniform();
        }
        // print header for rgb values
        cout << "Column " << i / 2 + 1 << ":" << endl;
        for (int j = 1; j <= numRows * 2; j += 2) {
          // linearly interpolate two of the three parameters
          //(only one will be used for modes 1 - 3)
          variedParam1 = (float)j / (numRows * 2);
          variedParam2 = (float)i / (numCols * 2);

          // choose gradient type based on current mode
          switch (MODE) {
          case VARYING_HUE:
            swatch = (TYPE == HCLAB)
                         ? RGB(HCLab(variedParam1, fixedParam1, fixedParam2))
                         : RGB(HCLuv(variedParam1, fixedParam1, fixedParam2));
            break;
          case VARYING_CHROMA:
            swatch = (TYPE == HCLAB)
                         ? RGB(HCLab(fixedParam1, variedParam1, fixedParam2))
                         : RGB(HCLuv(fixedParam1, variedParam1, fixedParam2));
            break;
          case VARYING_LUMINANCE:
            swatch = (TYPE == HCLAB)
                         ? RGB(HCLab(fixedParam1, fixedParam2, variedParam1))
                         : RGB(HCLuv(fixedParam1, fixedParam2, variedParam1));
            break;
          case FIXED_HUE:
            swatch = (TYPE == HCLAB)
                         ? RGB(HCLab(fixedParam1, variedParam2, variedParam1))
                         : RGB(HCLuv(fixedParam1, variedParam2, variedParam1));
            break;
          case FIXED_CHROMA:
            swatch = (TYPE == HCLAB)
                         ? RGB(HCLab(variedParam2, fixedParam1, variedParam1))
                         : RGB(HCLuv(variedParam2, fixedParam1, variedParam1));
            break;
          case FIXED_LUMINANCE:
            swatch = (TYPE == HCLAB)
                         ? RGB(HCLab(variedParam2, variedParam1, fixedParam1))
                         : RGB(HCLuv(variedParam2, variedParam1, fixedParam1));
            break;
          default:
            break;
          }

          // draw rectangles

          // triangle 1
          verts.vertex(i * width - widthOffset,
                       j * height - heightOffset); // bottom left
          verts.color(swatch);
          verts.vertex(i * width + width - widthOffset,
                       j * height - heightOffset); // bottom right
          verts.color(swatch);
          verts.vertex(i * width - widthOffset,
                       j * height + height - heightOffset); // top left
          verts.color(swatch);

          // triangle 2
          verts.vertex(i * width + width - widthOffset,
                       j * height - heightOffset); // bottom right
          verts.color(swatch);
          verts.vertex(i * width - widthOffset,
                       j * height + height - heightOffset); // top left
          verts.color(swatch);
          verts.vertex(i * width + width - widthOffset,
                       j * height + height - heightOffset); // top right
          verts.color(swatch);

          // print RGB values [0, 255] for each swatch
          cout << "{" << (int)(swatch.r * 255) << ", " << (int)(swatch.g * 255)
               << ", " << (int)(swatch.b * 255) << "}" << endl;
        }
        cout << endl;
      }
      // reset flag
      updateScene = false;
      // print instructions again for convenience
      printInstructions();
    }

    g.clear(0);

    // Set up 2D orthographic projection coordinates
    g.projMatrix(Matrix4f::ortho2D(0, 1, 0, 1)); // left, right, bottom, top
    g.viewMatrix(Matrix4f::identity());
    g.loadIdentity(); // model matrix

    g.meshColor(); // use mesh's color array
    g.draw(verts);
  }
};

int main() {
  CieColor().start();
  return 0;
}
