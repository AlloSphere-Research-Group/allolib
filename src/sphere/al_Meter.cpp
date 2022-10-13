#include <cfloat>

#include "al/sphere/al_Meter.hpp"
#include "al/math/al_Random.hpp"
#include "shader.hpp"

Vec3f spin;
float vspin[60];
ShaderProgram shader;
Texture texture;

void Meter::init(const Speakers &sl)
{
  mMesh.primitive(Mesh::POINTS);
  mMesh.color(HSV(0.67, 1., 1.));
  mSl = sl;
  // Texture & Shader 
  texture.create2D(500, 500, Texture::R8, Texture::RED, Texture::SHORT);
  int Nx = texture.width();
  int Ny = texture.height();
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
  texture.submit(&alpha[0]);
  shader.compile(vertex, fragment, geometry);
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
  // g.polygonLine();
  int index = 0;
  auto spkrIt = mSl.begin();
  g.color(1);
  g.lighting(true);
  for (const auto &v : values)
  {
    if (spkrIt != mSl.end())
    {
      // FIXME assumes speakers are sorted by device channel index
      // Should sort inside init()
      if (spkrIt->deviceChannel == index)
      {
        g.pushMatrix();
        // texture.bind();
        g.shader(shader);
        g.shader().uniform("halfSize", 5.5);
        // g.scale(1 / 5.0f);
        g.translate(spkrIt->vecGraphics());
        g.draw(mMesh);
        // texture.unbind();
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
