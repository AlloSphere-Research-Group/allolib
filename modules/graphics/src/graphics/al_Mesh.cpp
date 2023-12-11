// #include <algorithm> // transform
#include <cctype> // tolower
#include <cstdio>
#include <map>
#include <set>
// #include <string>
#include <fstream>
#include <sstream> // stringstream
#include <utility> // swap
// #include <cstdint>
#include <cstring>
#include <stdio.h>

#include "al/graphics/al_Mesh.hpp"
#include "al/system/al_Printing.hpp"

namespace al {

Mesh::Mesh(Primitive p) : mPrimitive(p) {
  //
}

Mesh::Mesh(const Mesh &cpy)
    : mVertices(cpy.mVertices), mNormals(cpy.mNormals), mColors(cpy.mColors),
      mTexCoord1s(cpy.mTexCoord1s), mTexCoord2s(cpy.mTexCoord2s),
      mTexCoord3s(cpy.mTexCoord3s), mIndices(cpy.mIndices),
      mPrimitive(cpy.mPrimitive) {}

void Mesh::copy(Mesh const &m) {
  mVertices = m.mVertices;
  mNormals = m.mNormals;
  mColors = m.mColors;
  mTexCoord1s = m.mTexCoord1s;
  mTexCoord2s = m.mTexCoord2s;
  mTexCoord3s = m.mTexCoord3s;
  mIndices = m.mIndices;
  mPrimitive = m.mPrimitive;
}

Mesh &Mesh::reset() {
  vertices().clear();
  normals().clear();
  colors().clear();
  texCoord1s().clear();
  texCoord2s().clear();
  texCoord3s().clear();
  indices().clear();
  return *this;
}

Mesh &Mesh::points() { return primitive(Mesh::POINTS); }
Mesh &Mesh::points(float stroke) { return points().stroke(stroke); }
Mesh &Mesh::lines() { return primitive(Mesh::LINES); }
Mesh &Mesh::lines(float stroke) { return lines().stroke(stroke); }
Mesh &Mesh::lineStrip() { return primitive(Mesh::LINE_STRIP); }
Mesh &Mesh::lineStrip(float stroke) { return lineStrip().stroke(stroke); }
Mesh &Mesh::triangles() { return primitive(Mesh::TRIANGLES); }
Mesh &Mesh::triangleStrip() { return primitive(Mesh::TRIANGLE_STRIP); }

bool Mesh::isPoints() const { return primitive() == Mesh::POINTS; }
bool Mesh::isLines() const { return primitive() == Mesh::LINES; }
bool Mesh::isLineStrip() const { return primitive() == Mesh::LINE_STRIP; }
bool Mesh::isTriangles() const { return primitive() == Mesh::TRIANGLES; }
bool Mesh::isTriangleStrip() const {
  return primitive() == Mesh::TRIANGLE_STRIP;
}

Mesh &Mesh::decompress() {
  if (indices().size()) { // only makes sense for indexed mesh
    toTriangles();
    int Ni = indices().size();

#define DECOMPRESS(buf, Type)                                                  \
  {                                                                            \
    int N = (int)buf.size();                                                   \
    if (N > 1) {                                                               \
      std::vector<Type> old(N);                                                \
      std::copy(&buf[0], (&buf[0]) + N, old.begin());                          \
      buf.resize(Ni);                                                          \
      for (int i = 0; i < Ni; ++i)                                             \
        buf[i] = old[indices()[i]];                                            \
    }                                                                          \
  }
    DECOMPRESS(vertices(), Vertex)
    DECOMPRESS(colors(), Color)
    DECOMPRESS(normals(), Normal)
    DECOMPRESS(texCoord1s(), TexCoord1)
    DECOMPRESS(texCoord2s(), TexCoord2)
    DECOMPRESS(texCoord3s(), TexCoord3)
#undef DECOMPRESS

    indices().clear();
  }
  return *this;
}

Mesh &Mesh::equalizeBuffers() {
  const int Nv = (int)vertices().size();
  const int Nn = (int)normals().size();
  const int Nc = (int)colors().size();
  const int Nt1 = (int)texCoord1s().size();
  const int Nt2 = (int)texCoord2s().size();
  const int Nt3 = (int)texCoord3s().size();

  if (Nn) {
    normals().reserve(Nv);
    for (int i = Nn; i < Nv; ++i) {
      normals().push_back(normals()[Nn - 1]);
    }
  }
  if (Nc) {
    colors().reserve(Nv);
    for (int i = Nc; i < Nv; ++i) {
      colors().push_back(colors()[Nc - 1]);
    }
  }
  if (Nt1) {
    texCoord1s().reserve(Nv);
    for (int i = Nt1; i < Nv; ++i) {
      texCoord1s().push_back(texCoord1s()[Nt1 - 1]);
    }
  }
  if (Nt2) {
    texCoord2s().reserve(Nv);
    for (int i = Nt2; i < Nv; ++i) {
      texCoord2s().push_back(texCoord2s()[Nt2 - 1]);
    }
  }
  if (Nt3) {
    texCoord3s().reserve(Nv);
    for (int i = Nt3; i < Nv; ++i) {
      texCoord3s().push_back(texCoord3s()[Nt3 - 1]);
    }
  }
  return *this;
}

void Mesh::createNormalsMesh(Mesh &mesh, float length, bool perFace) {
  auto initMesh = [](Mesh &m, int n) {
    m.vertices().resize(n * 2);
    m.reset();
    m.primitive(Mesh::LINES);
  };

  if (perFace) {
    // compute vertex based normals
    if (indices().size()) {
      int Ni = (int)indices().size();
      Ni = Ni - (Ni % 3); // must be multiple of 3
      initMesh(mesh, (Ni / 3) * 2);

      for (int i = 0; i < Ni; i += 3) {
        Index i1 = indices()[i + 0];
        Index i2 = indices()[i + 1];
        Index i3 = indices()[i + 2];
        const Vertex &v1 = vertices()[i1];
        const Vertex &v2 = vertices()[i2];
        const Vertex &v3 = vertices()[i3];

        // get mean:
        const Vertex mean = (v1 + v2 + v3) / 3.f;

        // get face normal:
        Vertex facenormal = cross(v2 - v1, v3 - v1);
        facenormal.normalize();

        mesh.vertex(mean);
        mesh.vertex(mean + (facenormal * length));
      }
    } else {
      AL_WARN_ONCE("createNormalsMesh only valid for indexed meshes");
    }
  } else {
    int Ni = (int)(vertices().size() < normals().size() ? vertices().size()
                                                        : normals().size());
    initMesh(mesh, Ni * 2);

    for (int i = 0; i < Ni; ++i) {
      const Vertex &v = vertices()[i];
      mesh.vertex(v);
      mesh.vertex(v + normals()[i] * length);
    }
  }
}

Mesh &Mesh::invertNormals() {
  for (auto &v : normals())
    v = -v;
  return *this;
}

Mesh &Mesh::compress() {
  int Ni = (int)indices().size();
  int Nv = (int)vertices().size();
  if (Ni) {
    AL_WARN_ONCE("cannot compress Mesh with indices");
    return *this;
  }
  if (Nv == 0) {
    AL_WARN_ONCE("cannot compress Mesh with no vertices");
    return *this;
  }

  int Nc = (int)colors().size();
  int Nn = (int)normals().size();
  int Nt1 = (int)texCoord1s().size();
  int Nt2 = (int)texCoord2s().size();
  int Nt3 = (int)texCoord3s().size();

  // map tree to uniquely ID vertices with same values:
  typedef std::map<float, int> Zmap;
  typedef std::map<float, Zmap> Ymap;
  typedef std::map<float, Ymap> Xmap;
  Xmap xmap;

  // copy current values:
  Mesh old(*this);

  // walk backward through the vertex list
  // create a ID for each one
  for (int i = (int)vertices().size() - 1; i >= 0; i--) {
    Vertex &v = vertices()[i];
    xmap[v.x][v.y][v.z] = i;
  }

  // map of old vertex index to new vertex index:
  typedef std::map<int, int> Imap;
  Imap imap;

  // reset current mesh:
  reset();

  // walk forward, inserting if
  for (size_t i = 0; i < old.vertices().size(); i++) {
    Vertex &v = old.vertices()[i];
    int idx = xmap[v.x][v.y][v.z];
    Imap::iterator it = imap.find(idx);
    if (it != imap.end()) {
      // use existing
      index(it->second);
    } else {
      // create new
      int newidx = (int)vertices().size();
      vertex(v);
      if (Nc)
        color(old.colors()[i]);
      if (Nn)
        normal(old.normals()[i]);
      if (Nt1)
        texCoord(old.texCoord1s()[i]);
      if (Nt2)
        texCoord(old.texCoord2s()[i]);
      if (Nt3)
        texCoord(old.texCoord3s()[i]);
      // store new index:
      imap[idx] = newidx;
      // use new index:
      index(newidx);
    }
  }
  return *this;
}

Mesh &Mesh::generateNormals(bool normalize, bool equalWeightPerFace) {
  auto calcNormal = [](const Vertex &v1, const Vertex &v2, const Vertex &v3,
                       bool MWE) {
    // MWAAT (mean weighted by areas of adjacent triangles)
    auto vn = cross(v2 - v1, v3 - v1);

    // MWE (mean weighted equally)
    if (MWE)
      vn.normalize();

    // MWA (mean weighted by angle)
    // This doesn't work well with dynamic marching cubes- normals
    // pop in and out for small triangles.
    /*Vertex v12= v2-v1;
    Vertex v13= v3-v1;
    Vertex vn = cross(v12, v13).normalize();
    vn *= angle(v12, v13) / M_PI;*/

    return vn;
  };

  size_t Nv = vertices().size();

  // need at least one triangle
  if (Nv < 3)
    return *this;

  // make same number of normals as vertices
  normals().resize(Nv);

  // compute vertex based normals
  if (indices().size()) {
    for (auto &n : normals())
      n = 0.;

    size_t Ni = indices().size();

    if (isTriangles()) {
      Ni = Ni - (Ni % 3); // must be multiple of 3

      for (unsigned i = 0; i < Ni; i += 3) {
        auto i1 = indices()[i];
        auto i2 = indices()[i + 1];
        auto i3 = indices()[i + 2];

        Vertex vn = calcNormal(vertices()[i1], vertices()[i2], vertices()[i3],
                               equalWeightPerFace);

        normals()[i1] += vn;
        normals()[i2] += vn;
        normals()[i3] += vn;
      }
    } else if (isTriangleStrip()) {
      for (unsigned i = 0; i < Ni - 2; ++i) {
        // Flip every other normal due to change in winding direction
        auto odd = i & 1;

        auto i1 = indices()[i];
        auto i2 = indices()[i + 1 + odd];
        auto i3 = indices()[i + 2 - odd];

        auto vn = calcNormal(vertices()[i1], vertices()[i2], vertices()[i3],
                             equalWeightPerFace);

        normals()[i1] += vn;
        normals()[i2] += vn;
        normals()[i3] += vn;
      }
    }

    // normalize the normals
    if (normalize) {
      for (auto &n : normals()) {
        n.normalize();
      }
    }
  }

  // non-indexed case
  else {
    // compute face based normals
    if (isTriangles()) {
      unsigned N = (unsigned)(Nv - (Nv % 3));

      for (unsigned i = 0; i < N; i += 3) {
        auto i1 = i + 0;
        auto i2 = i + 1;
        auto i3 = i + 2;
        const auto &v1 = vertices()[i1];
        const auto &v2 = vertices()[i2];
        const auto &v3 = vertices()[i3];

        auto vn = cross(v2 - v1, v3 - v1);
        if (normalize)
          vn.normalize();

        normals()[i1] = vn;
        normals()[i2] = vn;
        normals()[i3] = vn;
      }
    }
    // compute vertex based normals
    else if (isTriangleStrip()) {
      for (unsigned i = 0; i < Nv; ++i)
        normals()[i].set(0, 0, 0);

      for (unsigned i = 0; i < Nv - 2; ++i) {
        // Flip every other normal due to change in winding direction
        auto odd = i & 1;

        auto vn = calcNormal(vertices()[i], vertices()[i + 1 + odd],
                             vertices()[i + 2 - odd], equalWeightPerFace);

        normals()[i] += vn;
        normals()[i + 1] += vn;
        normals()[i + 2] += vn;
      }

      // normalize the normals
      if (normalize) {
        for (auto &n : normals()) {
          n.normalize();
        }
      }
    }
  }
  return *this;
}

Mesh &Mesh::repeatLast() {
  if (indices().size()) {
    index(indices().back());
  } else {
    if (colors().size())
      colors().push_back(colors().back());
    if (vertices().size())
      vertices().push_back(vertices().back());
    if (normals().size())
      normals().push_back(normals().back());
    if (texCoord2s().size())
      texCoord2s().push_back(texCoord2s().back());
    if (texCoord3s().size())
      texCoord3s().push_back(texCoord3s().back());
    if (texCoord1s().size())
      texCoord1s().push_back(texCoord1s().back());
  }
  return *this;
}

Mesh &Mesh::ribbonize(float *widths, int widthsStride, bool faceBinormal) {
  auto frenet = [](Vertex *f, const Vertex &v0, const Vertex &v1,
                   const Vertex &v2) {
    const auto vf = v2 - v1;             // forward difference
    const auto vb = v1 - v0;             // backward difference
    const auto d1 = vf + vb;             // first difference (x 2)
    f[2] = cross(vb, vf).normalized();   // binormal
    f[1] = cross(d1, f[2]).normalized(); // normal (T x B)
    // f[0] = d1.normalized(); // tangent (not used)
  };

  const int N = (int)mVertices.size();

  if (0 == N)
    return *this;

  mVertices.resize(N * 2);
  mNormals.resize(N * 2);

  int in = faceBinormal ? 2 : 1;
  int ib = faceBinormal ? 1 : 2;

  Vertex ff[3]; // T,N,B

  // Compute second and second to last Frenet frames used later to ribbonize
  // the first and last vertices.
  frenet(ff, mVertices[0], mVertices[1], mVertices[2]);
  const auto n1 = ff[in];
  const auto b1 = ff[ib] * widths[0];
  frenet(ff, mVertices[N - 3], mVertices[N - 2], mVertices[N - 1]);
  const auto nN = ff[in];
  const auto bN = ff[ib] * widths[(N - 1) * widthsStride];

  // Store last vertex since it will be overwritten eventually
  const auto last = mVertices[N - 1];

  // Go backwards through vertices since we are processing in place
  for (int i = N - 2; i >= 1; --i) {
    int i0 = i - 1;
    int i1 = i;
    int i2 = i + 1;
    const auto &v0 = mVertices[i0];
    const auto &v1 = mVertices[i1];
    const auto &v2 = mVertices[i2];

    // Compute Frenet frame
    frenet(ff, v0, v1, v2);

    // Scale binormal by ribbon width
    ff[ib] *= widths[i1 * widthsStride];

    int i12 = i1 << 1;
    // v1 is ref, so we must write in reverse to properly handle i=0
    mVertices[i12 + 1] = v1 + ff[ib];
    mVertices[i12] = v1 - ff[ib];

    mNormals[i12].set(ff[in][0], ff[in][1], ff[in][2]);
    mNormals[i12 + 1] = mNormals[i12];
    return *this;
  }

  // Ribbonize first and last vertices
  mVertices[1] = mVertices[0] + b1;
  mVertices[0] = mVertices[0] - b1;
  mNormals[0] = mNormals[1] = n1;
  int iN = (N - 1) * 2;
  mVertices[iN + 1] = last + bN;
  mVertices[iN + 0] = last - bN;
  mNormals[iN + 0] = mNormals[iN + 1] = nN;

  if (mColors.size()) {
    // mColors.expand<2,true>();
    int old_size = (int)mColors.size();
    mColors.reserve(old_size * 2);
    for (int i = old_size; i < old_size * 2; i++) {
      mColors.push_back(mColors[old_size - 1]);
    }
  }
  return *this;
}

Mesh &Mesh::smooth(float amount, int weighting) {
  std::map<int, std::set<int>> nodes;

  int Ni = (int)indices().size();

  // Build adjacency map
  for (int i = 0; i < Ni; i += 3) {
    int i0 = indices()[i];
    int i1 = indices()[i + 1];
    int i2 = indices()[i + 2];
    nodes[i0].insert(i1);
    nodes[i0].insert(i2);
    nodes[i1].insert(i2);
    nodes[i1].insert(i0);
    nodes[i2].insert(i0);
    nodes[i2].insert(i1);
  }

  Mesh::Vertices vertsCopy(vertices());

  for (const auto &node : nodes) {
    Mesh::Vertex sum(0, 0, 0);
    const auto &adjs = node.second;

    switch (weighting) {
    case 0: { // equal weighting
      for (auto adj : adjs) {
        sum += vertsCopy[adj];
      }
      sum /= (float)adjs.size();
    } break;

    case 1: { // inverse distance weights; reduces vertex sliding
      float sumw = 0;
      for (auto adj : adjs) {
        const auto &v = vertsCopy[adj];
        const auto &c = vertsCopy[node.first];
        float dist = (v - c).mag();
        float w = 1.0f / dist;
        sumw += w;
        sum += v * w;
      }
      sum /= sumw;
    } break;
    }

    auto &orig = vertices()[node.first];
    orig = (sum - orig) * amount + orig;
  }
  return *this;
}

Mesh &Mesh::flipWinding() {
  if (isTriangles()) {
    if (mIndices.size()) {
      for (size_t i = 0; i < mIndices.size(); i += 3)
        std::swap(mIndices[i], mIndices[i + 2]);
    } else {
      for (size_t i = 0; i < mVertices.size(); i += 3)
        std::swap(mVertices[i], mVertices[i + 2]);
    }
  }
  return *this;
}

Mesh &Mesh::merge(const Mesh &src) {
  // TODO: only do merge if source and dest are well-formed
  // TODO: what to do when mixing float and integer colors? promote or demote?
  if (src.vertices().empty())
    return *this;

  // Inherit primitive if no verts yet
  if (vertices().empty()) {
    primitive(src.primitive());
  }
  // Source has indices, and I either do or don't.
  // After this block, I will have indices.
  if (src.indices().size()) {
    Index Nv = (unsigned int)vertices().size();
    Index Ni = (unsigned int)indices().size();
    // If no indices, must create
    if (0 == Ni) {
      for (Index i = 0; i < Nv; ++i)
        index(i);
    }
    // Add source indices offset by my number of vertices
    index(src.indices().data(), (int)src.indices().size(), (unsigned int)Nv);
  }

  // Source doesn't have indices, but I do
  else if (indices().size()) {
    size_t Nv = vertices().size();
    for (size_t i = Nv; i < Nv + src.vertices().size(); ++i)
      index((unsigned int)i);
  }

  // From here, everything is indice invariant

  // equalizeBuffers(); << TODO: must do this if we are using indices

  vertices().insert(vertices().end(), src.vertices().begin(),
                    src.vertices().end());
  normals().insert(normals().end(), src.normals().begin(), src.normals().end());
  colors().insert(colors().end(), src.colors().begin(), src.colors().end());
  texCoord1s().insert(texCoord1s().end(), src.texCoord1s().begin(),
                      src.texCoord1s().end());
  texCoord2s().insert(texCoord2s().end(), src.texCoord2s().begin(),
                      src.texCoord2s().end());
  texCoord3s().insert(texCoord3s().end(), src.texCoord3s().begin(),
                      src.texCoord3s().end());
  return *this;
}

void Mesh::getBounds(Vec3f &min, Vec3f &max) const {
  if (vertices().size()) {
    min = vertices()[0];
    max = min;
    for (size_t v = 1; v < vertices().size(); ++v) {
      const Vertex &vt = vertices()[v];
      for (int i = 0; i < 3; ++i) {
        min[i] = (min[i] < vt[i]) ? min[i] : vt[i];
        max[i] = (max[i] > vt[i]) ? max[i] : vt[i];
      }
    }
  }
}

Vec3f Mesh::getCenter() const {
  Vec3f min(0), max(0);
  getBounds(min, max);
  return min + (max - min) * 0.5;
}

Mesh &Mesh::fitToSphere(float radius) {
  double maxMag = 0.;
  for (auto &v : mVertices) {
    auto mm = v.dot(v);
    if (mm > maxMag)
      maxMag = mm;
  }
  if (maxMag > 0.) {
    auto nrm = radius / sqrt(maxMag);
    for (auto &v : mVertices) {
      v *= nrm;
    }
  }
  return *this;
}

Mesh &Mesh::fitToCubeTransform(Vec3f &center, Vec3f &scale, float radius,
                               bool proportional) {
  Vertex min(0), max(0);
  getBounds(min, max);
  // span of each axis:
  auto span = max - min; // positive only
                         // center of each axis:
  center = min + (span * 0.5);
  // axis scalar:
  scale = (2.f * radius) / span; // positive only

  // adjust to use scale of largest axis:
  if (proportional) {
    scale = al::min(scale);
  }
  return *this;
}

Mesh &Mesh::fitToCube(float radius, bool proportional) {
  Vec3f center, scale;
  fitToCubeTransform(center, scale, radius, proportional);
  for (auto &v : mVertices)
    v = (v - center) * scale;
  return *this;
}

Mesh &Mesh::unitize(bool proportional) { return fitToCube(1.f, proportional); }

Mesh &Mesh::translate(float x, float y, float z) {
  const Vertex xfm(x, y, z);
  for (auto &v : mVertices)
    v += xfm;
  return *this;
}

Mesh &Mesh::scale(float x, float y, float z) {
  const Vertex xfm(x, y, z);
  for (auto &v : mVertices)
    v *= xfm;
  return *this;
}

bool Mesh::valid() const { return mVertices.size(); }

// removes triplets with two matching values
template <class T> static void removeDegenerates(std::vector<T> &buf) {
  size_t N = buf.size();
  unsigned j = 0;
  for (unsigned i = 0; i < N; i += 3) {
    T v1 = buf[i];
    T v2 = buf[i + 1];
    T v3 = buf[i + 2];
    buf[j] = v1;
    buf[j + 1] = v2;
    buf[j + 2] = v3;
    if ((v1 != v2) && (v2 != v3) && (v3 != v1)) {
      j += 3;
    }
  }
  buf.resize(j);
}

template <class T> static void stripToTri(std::vector<T> &buf) {
  int N = (int)buf.size();
  int Ntri = N - 2;
  buf.resize(Ntri * 3);

  // Iterate backwards through elements so we can operate in place
  // strip (i): 0 1 2 3 4 5 6 7 8 ...
  //  tris (j): 0 1 2 3 2 1 2 3 4 ...
  for (int i = N - 3, j = Ntri * 3 - 3; i > 0; i--, j -= 3) {
    // Odd numbered triangles must have orientation flipped
    if (i & 1) {
      buf[j] = buf[i + 2];
      buf[j + 1] = buf[i + 1];
      buf[j + 2] = buf[i];
    } else {
      buf[j] = buf[i];
      buf[j + 1] = buf[i + 1];
      buf[j + 2] = buf[i + 2];
    }
  }
}

Mesh &Mesh::toTriangles() {
  if (isTriangleStrip()) {
    primitive(TRIANGLES);
    size_t Nv = vertices().size();
    size_t Ni = indices().size();

    // indexed:
    if (Ni > 3) {
      stripToTri(indices());
      removeDegenerates(indices());
    }
    // non-indexed:
    // TODO: remove degenerate triangles
    else if (Ni == 0 && Nv > 3) {
      stripToTri(vertices());
      unsigned int Nv = (unsigned int)vertices().size();
      for (unsigned int i = 0; i < Nv - 2; i += 2) {
        index(i);
        index(i + 1);
        index(i + 2);
        index(i + 2);
        index(i + 1);
        index(i + 3);
      }
      if (normals().size() >= Nv)
        stripToTri(normals());
      if (colors().size() >= Nv)
        stripToTri(colors());
      if (texCoord1s().size() >= Nv)
        stripToTri(texCoord1s());
      if (texCoord2s().size() >= Nv)
        stripToTri(texCoord2s());
      if (texCoord3s().size() >= Nv)
        stripToTri(texCoord3s());
    }
  }
  return *this;
}

Mesh &Mesh::colorFill(const Color &v) {
  int N = vertices().size() - colors().size();
  for (int i = 0; i < N; ++i)
    color(v);
  return *this;
}

// Mesh &Mesh::coloriFill(const Colori &v) {
//  int N = vertices().size() - coloris().size();
//  for (int i = 0; i < N; ++i)
//    colori(v);
//  return *this;
//}

Mesh &
Mesh::forEachFace(const std::function<void(int v1, int v2, int v3)> &onFace) {
  if (mIndices.size()) {
    if (isTriangles()) {
      for (size_t i = 2; i < mIndices.size(); i += 3) {
        onFace(mIndices[i - 2], mIndices[i - 1], mIndices[i]);
      }
    } else if (isTriangleStrip()) {
      for (size_t i = 0; i < mIndices.size() - 2; i++) {
        int w = i & 1; // winding: 0=ccw, 1=cw
        onFace(mIndices[i + w], mIndices[i + 1 - w], mIndices[i + 2]);
      }
    } else if (isLines()) {
      for (size_t i = 1; i < mIndices.size(); i += 2) {
        onFace(mIndices[i - 1], mIndices[i], mIndices[i - 1]);
      }
    } else if (isPoints()) {
      for (size_t i = 0; i < mIndices.size(); i++) {
        onFace(mIndices[i], mIndices[i], mIndices[i]);
      }
    }
  } else {
    if (isTriangles()) {
      for (size_t i = 2; i < mVertices.size(); i += 3) {
        onFace(i - 2, i - 1, i);
      }
    } else if (isTriangleStrip()) {
      for (size_t i = 0; i < mVertices.size() - 2; i++) {
        int w = i & 1; // winding: 0=ccw, 1=cw
        onFace(i + w, i + 1 - w, i + 2);
      }
    } else if (isLines()) {
      for (size_t i = 1; i < mVertices.size(); i += 2) {
        onFace(i - 1, i, i - 1);
      }
    } else if (isPoints()) {
      for (size_t i = 0; i < mVertices.size(); i++) {
        onFace(i, i, i);
      }
    }
  }
  return *this;
}

const Mesh &Mesh::forEachFace(
    const std::function<void(int v1, int v2, int v3)> &onFace) const {
  return const_cast<Mesh *>(this)->forEachFace(onFace);
}

bool Mesh::saveSTL(const char *filePath, const char *solidName) const {
  int prim = primitive();

  if (TRIANGLES != prim && TRIANGLE_STRIP != prim) {
    AL_WARN("Unsupported primitive type. Must be either triangles or triangle "
            "strip.");
    return false;
  }

  // Create a copy since we must convert to non-indexed triangles
  Mesh m(*this);

  // Convert mesh to non-indexed triangles
  m.toTriangles();
  m.decompress();
  m.generateNormals();

  // STL vertices must be in positive octant
  Vec3f vmin, vmax;
  m.getBounds(vmin, vmax);

  std::ofstream s(filePath);
  if (s.fail())
    return false;
  s.flags(std::ios::scientific);

  s << "solid " << solidName << "\n";
  for (size_t i = 0; i < m.vertices().size(); i += 3) {
    s << "facet normal";
    for (int j = 0; j < 3; j++)
      s << " " << m.normals()[i][j];
    s << "\n";
    s << "outer loop\n";
    for (int j = 0; j < 3; ++j) {
      s << "vertex";
      for (int k = 0; k < 3; k++)
        s << " " << m.vertices()[i + j][k] - vmin[k];
      s << "\n";
    }
    s << "endloop\n";
    s << "endfacet\n";
  }
  s << "endsolid " << solidName;

  return true;
}

bool Mesh::savePLY(const char *filePath, const char *solidName,
                   bool binary) const {
  // Ref: http://paulbourke.net/dataformats/ply/

  int prim = primitive();

  if (TRIANGLES != prim && TRIANGLE_STRIP != prim) {
    AL_WARN("Unsupported primitive type. Must be either triangles or triangle "
            "strip.");
    return false;
  }

  unsigned Nv = (unsigned)vertices().size();

  if (!Nv)
    return false;

  std::ofstream s;
  s.open(filePath, binary ? (std::ios::out | std::ios::binary) : std::ios::out);
  if (s.fail())
    return false;

  // Use a copy to handle triangle strip;
  // not ideal if already triangles!
  Mesh m(*this);
  m.toTriangles();

  Nv = (unsigned int)m.vertices().size();
  const unsigned Nc = (unsigned)m.colors().size();
  const unsigned Ni = (unsigned)m.indices().size();
  // const unsigned Bi = Nv<=65536 ? 2 : 4; // max bytes/index
  const unsigned Bi =
      Nv <= 32768 ? 2 : 4; // changed since assimp import not working with full
                           // ushort range up to 65536

  int bigEndian = 1;
  if (1 == *(char *)&bigEndian)
    bigEndian = 0;

  // Header
  s << "ply\n";
  s << "format "
    << (binary ? (bigEndian ? "binary_big_endian" : "binary_little_endian")
               : "ascii")
    << " 1.0\n";
  s << "comment Exported by AlloSystem\n";

  if (solidName[0]) {
    s << "comment " << solidName << "\n";
  }

  s << "element vertex " << Nv
    << "\n"
       "property float x\n"
       "property float y\n"
       "property float z\n";

  // TODO: normals (nx,ny,nz), texcoords (s,t)

  bool hasColors = Nc >= Nv;
  if (hasColors) {
    s << "property uchar red\n"
         "property uchar green\n"
         "property uchar blue\n"
         "property uchar alpha\n";
  }

  if (Ni) {
    s << "element face " << Ni / 3 << "\n";
    s << "property list uchar " << (Bi == 4 ? "uint" : "ushort")
      << " vertex_indices\n";
  }

  s << "end_header\n";

  if (binary) {
    // Vertex data
    for (unsigned i = 0; i < Nv; ++i) {
      s.write(reinterpret_cast<const char *>(&m.vertices()[i][0]),
              sizeof(Mesh::Vertex));
      if (hasColors) {
        auto col = Colori(m.colors()[i].clamp(1.0));

        s << col.r << col.g << col.b << col.a;
      }
    }
    // Face data
    if (Ni) {
      for (unsigned i = 0; i < Ni; i += 3) {
        s << char(3); // 3 indices/face
        if (sizeof(Mesh::Index) == Bi) {
          s.write(reinterpret_cast<const char *>(&m.indices()[i]), Bi * 3);
        } else {
          if (Bi == 4) {
            uint32_t idx[3];
            idx[0] = m.indices()[i];
            idx[1] = m.indices()[i + 1];
            idx[2] = m.indices()[i + 2];
            s.write(reinterpret_cast<const char *>(idx), sizeof(int32_t) * 3);
          } else {
            uint16_t idx[3];
            idx[0] = m.indices()[i];
            idx[1] = m.indices()[i + 1];
            idx[2] = m.indices()[i + 2];
            s.write(reinterpret_cast<const char *>(idx), sizeof(uint16_t) * 3);
          }

          // printf("%u %u %u\n", idx[0], idx[1], idx[2]);
          /*for(int i=0; i<Bi*3; ++i){
            printf("%d ", reinterpret_cast<char*>(idx)[i]);
          }printf("\n");*/
        }
      }
    }
  } else {
    // Vertex data
    for (unsigned i = 0; i < Nv; ++i) {
      auto vrt = m.vertices()[i];
      s << vrt.x << " " << vrt.y << " " << vrt.z;
      if (hasColors) {
        auto col = Colori(m.colors()[i]);
        s << " " << int(col.r) << " " << int(col.g) << " " << int(col.b) << " "
          << int(col.a);
      }
      s << "\n";
    }
    // Face data
    if (Ni) {
      for (unsigned i = 0; i < Ni; i += 3) {
        auto i1 = m.indices()[i];
        auto i2 = m.indices()[i + 1];
        auto i3 = m.indices()[i + 2];
        s << "3 " << i1 << " " << i2 << " " << i3 << "\n";
      }
    }
  }

  return true;
}

bool Mesh::save(const char *filePath, const char *solidName,
                bool binary) const {
  // auto pos = filePath.find_last_of(".");
  // if(std::string::npos == pos) return false;
  // auto ext = filePath.substr(pos+1);
  // std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  size_t len = std::strlen(filePath);
  if (len < 5) {
    return false;
  }

  const char *ext3 = filePath + (len - 4);
  const char lowerExt3[] = {ext3[0], (char)std::tolower(ext3[1]),
                            (char)std::tolower(ext3[2]),
                            (char)std::tolower(ext3[3]), '\0'};

  if (std::strcmp(lowerExt3, ".ply")) {
    return savePLY(filePath, solidName, binary);
  } else if (std::strcmp(lowerExt3, ".stl")) {
    return saveSTL(filePath, solidName);
  }
  return false;
}

template <typename T> size_t vectorsizeof(const typename std::vector<T> &vec) {
  return sizeof(T) * vec.size();
}

void Mesh::print(FILE *dst) const {
  fprintf(dst, "Mesh %p (prim = %d) has:\n", this, mPrimitive);

  auto niceByteString = [](unsigned bytes, double &printVal) {
    if (bytes < 1000) {
      printVal = bytes;
      return "B";
    }
    if (bytes < 1000000) {
      printVal = bytes * 1e-3;
      return "kB";
    } else {
      printVal = bytes * 1e-6;
      return "MB";
    }
  };

#define QUERY_ATTRIB(attrib, name)                                             \
  if (attrib.size()) {                                                         \
    auto B = vectorsizeof(attrib);                                             \
    Btot += B;                                                                 \
    double byteVal;                                                            \
    const char *byteUnits = niceByteString(B, byteVal);                        \
    fprintf(dst, "%8d " #name " (%.1f %s)\n", (int)attrib.size(), byteVal,     \
            byteUnits);                                                        \
  }

  unsigned Btot = 0;
  QUERY_ATTRIB(mVertices, Vertices)
  QUERY_ATTRIB(mColors, Colors)
  //  QUERY_ATTRIB(mColoris, Coloris)
  QUERY_ATTRIB(mNormals, Normals)
  QUERY_ATTRIB(mTexCoord1s, TexCoord1s)
  QUERY_ATTRIB(mTexCoord2s, TexCoord2s)
  QUERY_ATTRIB(mTexCoord3s, TexCoord3s)
  QUERY_ATTRIB(mIndices, Indices)

  {
    double byteVal;
    const char *byteUnits = niceByteString(Btot, byteVal);
    fprintf(dst, "%8d bytes (%.1f %s)\n", Btot, byteVal, byteUnits);
  }
}

bool Mesh::debug(FILE *dst) const {

#define DPRINTF(...)                                                           \
  if (dst) {                                                                   \
    fprintf(dst, __VA_ARGS__);                                                 \
  }

  bool ok = true;
  size_t Nv = vertices().size();
  if (!Nv) {
    DPRINTF("No vertices\n");
    ok = false;
  }

#define CHECK_ARR(arr, oneOkay)                                                \
  if (arr.size() && arr.size() != Nv) {                                        \
    if (!oneOkay || arr.size() != 1) {                                         \
      DPRINTF("%d " #arr ", but %d vertices\n", (int)arr.size(), Nv);          \
      ok = false;                                                              \
    }                                                                          \
  }
  CHECK_ARR(mNormals, false);
  CHECK_ARR(mColors, true);
  //  CHECK_ARR(mColoris, true);
  CHECK_ARR(mTexCoord1s, false);
  CHECK_ARR(mTexCoord2s, false);
  CHECK_ARR(mTexCoord3s, false);
#undef CHECK_ARR

  //  if (colors().size() && coloris().size()) {
  //    DPRINTF("More than one color array populated\n");
  //    ok = false;
  //  }

  if (texCoord1s().size() && texCoord2s().size() && texCoord2s().size()) {
    DPRINTF("More than one texture coordinate array populated\n");
    ok = false;
  }

  if (indices().size()) {
    for (auto i : indices()) {
      if (i > Nv) {
        DPRINTF("Index out of bounds: %d (%d max)\n", i, Nv);
        ok = false;
        break;
      }
    }
  }

#undef DPRINTF

  return ok;
}

} // namespace al
