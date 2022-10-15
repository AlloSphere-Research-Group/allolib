#include <cfloat>

#include "al/sphere/al_Meter.hpp"
#include "al/math/al_Random.hpp"
#include <fstream>

using namespace std;

Vec3f spin;
float vspin[60];
ShaderProgram pointShader;
Texture pointTexture;
FBO renderTarget;
Texture rendered;

void updateFBO() {
  rendered.create2D(1280, 640);
  renderTarget.bind();
  renderTarget.attachTexture2D(rendered);
  renderTarget.unbind();
}

string slurp(string fileName)
{
  fstream file(fileName);
  string returnValue = "";
  while (file.good())
  {
    string line;
    getline(file, line);
    returnValue += line + "\n";
  }
  return returnValue;
}

void onResize() {
  updateFBO();
  //
}

void Meter::init(const Speakers &sl)
{
  mMesh.vertex(0, 0, 0);
  mMesh.primitive(Mesh::POINTS);
  addCube(mMesh);

  mMesh.color(HSV(0.67, 10., 10.));
  mSl = sl;
  // Texture & Shader
  pointTexture.create2D(256, 256, Texture::R8, Texture::RED, Texture::SHORT);
  int Nx = pointTexture.width();
  int Ny = pointTexture.height();
  std::vector<short> alpha;
  alpha.resize(Nx * Ny);
  for (int j = 0; j < Ny; ++j)
  {
    float y = float(j) / (Ny - 1) * 2 - 1;
    for (int i = 0; i < Nx; ++i)
    {
      float x = float(i) / (Nx - 1) * 2 - 1;
      float m = exp(-13 * (x * x + y * y));
      m *= pow(2, 15) - 1; // scale by the largest positive short int
      alpha[j * Nx + i] = m;
    }
  }
  pointTexture.submit(&alpha[0]);
  pointShader.compile(slurp("point-vertex.glsl"),
                      slurp("point-fragment.glsl"),
                      slurp("point-geometry.glsl"));
  updateFBO();
}

void Meter::processSound(AudioIOData &io)
{

  if (tempValues.size() != io.channelsOut())
  {
    tempValues.resize(io.channelsOut());
    values.resize(io.channelsOut());
    std::cout << "Resizing Meter buffers" << std::endl;
  }
  for (int i = 0; i < io.channelsOut(); i++)
  {
    tempValues[i] = FLT_MIN;
    auto *outBuf = io.outBuffer(i);
    auto fpb = io.framesPerBuffer();
    for (int samp = 0; samp < fpb; samp++)
    {
      float val = fabs(*outBuf);
      if (tempValues[i] < val)
      {
        tempValues[i] = val;
      }
      outBuf++;
    }
    if (tempValues[i] == 0)
    {
      tempValues[i] = 0.01;
    }
    else
    {
      float db = 20.0 * log10(tempValues[i]);
      if (db < -60)
      {
        tempValues[i] = 0.01;
      }
      else
      {
        tempValues[i] = 0.01 + 0.005 * (60 + db);
      }
    }
    if (values[i] > tempValues[i])
    {
      values[i] = values[i] - 0.05 * (values[i] - tempValues[i]);
    }
    else
    {
      values[i] = tempValues[i];
    }
  }
}

void Meter::draw(Graphics &g)
{
  int index = 0;
  auto spkrIt = mSl.begin();
  // g.lighting(true);
  g.depthTesting(true);
  g.blending(true);
  g.blendAdd();
  g.blendTrans();
  g.meshColor();
  for (const auto &v : values)
  {
    if (spkrIt != mSl.end())
    {
      // FIXME assumes speakers are sorted by device channel index
      // Should sort inside init()
      if (spkrIt->deviceChannel == index)
      {
        g.pushMatrix();
        // pointTexture.bind();
        // g.shader(pointShader);
        g.translate(spkrIt->vecGraphics());
        g.pointSize(5. +  120* v);
        g.color(HSV(0.67 + v, 0.2 + v*20, 0.2 + v*30));
        g.scale(1+ 10*v, 1 + 100* v, 1 + 10*v);
        g.draw(mMesh);
        // pointTexture.unbind();
        g.popMatrix();
        spkrIt++;
      }
    }
    else
    {
      spkrIt = mSl.begin();
    }
    index++;
  }
}

void Meter::setMeterValues(float *newValues, size_t count)
{
  if (tempValues.size() != count)
  {
    tempValues.resize(count);
    values.resize(count);
    std::cout << "Resizing Meter buffers" << std::endl;
  }
  count = values.size();
  for (int i = 0; i < count; i++)
  {
    values[i] = *newValues;
    newValues++;
  }
}
