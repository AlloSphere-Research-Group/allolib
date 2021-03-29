#include "al/graphics/al_RenderManager.hpp"

namespace al {
// from al_Window.hpp
float getCurrentWindowPixelDensity();
}  // namespace al

using namespace al;

//______________________________________________________________________________

MatrixStack::MatrixStack() {
  stack.emplace_back();
  stack.reserve(10);
}
void MatrixStack::mult(Matrix4f const& m) { stack.back() = stack.back() * m; }
void MatrixStack::set(const Matrix4f& m) { stack.back() = m; }
void MatrixStack::setIdentity() { stack.back().setIdentity(); }
Matrix4f MatrixStack::get() const { return stack.back(); }

void MatrixStack::push() {
  Matrix4f m = stack.back();
  stack.push_back(m);
}

void MatrixStack::pop() { stack.pop_back(); }
void MatrixStack::pop_all() {
  stack.clear();
  stack.emplace_back();
}

//______________________________________________________________________________

ViewportStack::ViewportStack() {
  stack.emplace_back();
  stack.reserve(10);
}
void ViewportStack::set(const Viewport& m) { stack.back().set(m); }
void ViewportStack::set(int left, int bottom, int width, int height) {
  stack.back().set(left, bottom, width, height);
}
Viewport ViewportStack::get() const { return stack.back(); }

void ViewportStack::push() {
  Viewport v = stack.back();
  stack.push_back(v);
}

void ViewportStack::pop() { stack.pop_back(); }

//______________________________________________________________________________

FBOStack::FBOStack() {
  stack.emplace_back();
  stack.reserve(10);
}
void FBOStack::push() {
  unsigned int i = stack.back();
  stack.push_back(i);
}
void FBOStack::pop() { stack.pop_back(); }
unsigned int FBOStack::get() const { return stack.back(); }
void FBOStack::set(unsigned int id) { stack.back() = id; }

//______________________________________________________________________________

void RenderManager::translate(float x, float y, float z) {
  mModelStack.mult(Matrix4f::translation(x, y, z));
  mMatChanged = true;
}

void RenderManager::rotate(float angle, float x, float y, float z) {
  mModelStack.mult(Matrix4f::rotate(float(M_2PI) * angle / 360.0f, x, y, z));
  mMatChanged = true;
}

void RenderManager::rotate(const Quatf& q) {
  Matrix4f m;
  q.toMatrix(m.elems());
  mModelStack.mult(m);
  mMatChanged = true;
}

void RenderManager::rotate(const Quatd& q) {
  Matrix4f m;
  q.toMatrix(m.elems());
  mModelStack.mult(m);
  mMatChanged = true;
}

void RenderManager::scale(float x, float y, float z) {
  mModelStack.mult(Matrix4f::scaling(x, y, z));
  mMatChanged = true;
}

void RenderManager::viewport(int left, int bottom, int width, int height) {
  mViewportStack.set(left, bottom, width, height);
  glViewport(left, bottom, width, height);
}

void RenderManager::pushViewport() { mViewportStack.push(); }

void RenderManager::popViewport() {
  mViewportStack.pop();
  viewport(mViewportStack.get());
}

void RenderManager::framebuffer(unsigned int id) {
  FBO::bind(id);
  mFBOStack.set(id);
  // mFBOID = id;
}

void RenderManager::pushFramebuffer() { mFBOStack.push(); }
void RenderManager::popFramebuffer() {
  mFBOStack.pop();
  framebuffer(mFBOStack.get());
}

void RenderManager::shader(ShaderProgram& s) {
  mShaderPtr = &s;
  mShaderPtr->use();
  mShaderChanged = true;

  auto mv_search = modelviewLocs.find(mShaderPtr->id());
  if (mv_search == modelviewLocs.end()) {
    modelviewLocs[mShaderPtr->id()] =
        mShaderPtr->getUniformLocation("al_ModelViewMatrix");
  }

  auto pr_search = projLocs.find(mShaderPtr->id());
  if (pr_search == projLocs.end()) {
    projLocs[mShaderPtr->id()] =
        mShaderPtr->getUniformLocation("al_ProjectionMatrix");
  }
}

void RenderManager::camera(Viewpoint const& v) {
  mViewStack.set(v.viewMatrix());
  mProjStack.set(v.projMatrix(viewport().aspect()));
  mMatChanged = true;
}

void RenderManager::camera(Viewpoint::SpecialType v) {
  // let's draw 2D things at z = [0:1] (larger z, closer)
  switch (v) {
    case Viewpoint::IDENTITY: {
      mViewStack.setIdentity();
      mProjStack.setIdentity();
    } break;

    case Viewpoint::ORTHO_FOR_2D: {
      float rpd =
          getCurrentWindowPixelDensity();  // reciprocal of pixel density
      auto v = viewport();                 // viewport in framebuffer unit
      mViewStack.setIdentity();
      mProjStack.set(Matrix4f::ortho(v.l * rpd, v.w * rpd,  // left, right
                                     v.b * rpd, v.h * rpd,  // bottom, top
                                     0, 1                   // near, far
                                     ));
    } break;

    case Viewpoint::UNIT_ORTHO: {
      auto v = viewport();
      float spanx = float(v.w) / v.h;
      float spany = 1;
      if (spanx < 1) {
        spanx = 1;
        spany = float(v.h) / v.w;
      }
      mViewStack.setIdentity();
      mProjStack.set(Matrix4f::ortho(-spanx, spanx,  // left, right
                                     -spany, spany,  // bottom, top
                                     0, 1));         // near, far
    } break;

    case Viewpoint::UNIT_ORTHO_INCLUSIVE: {
      auto v = viewport();
      float spanx = float(v.w) / v.h;
      float spany = 1;
      if (spanx > 1) {
        spanx = 1;
        spany = float(v.h) / v.w;
      }
      mViewStack.setIdentity();
      mProjStack.set(Matrix4f::ortho(-spanx, spanx,  // left, right
                                     -spany, spany,  // bottom, top
                                     0, 1));         // near, far
    } break;

    default:
      break;
  }
  mMatChanged = true;
}

void RenderManager::pushCamera() {
  mViewStack.push();
  mProjStack.push();
}

void RenderManager::popCamera() {
  mViewStack.pop();
  mProjStack.pop();
  mMatChanged = true;
}

void RenderManager::update() {
  if (mShaderChanged || mMatChanged) {
    shader().uniform(modelviewLocs[mShaderPtr->id()],
                     viewMatrix() * modelMatrix());
    shader().uniform(projLocs[mShaderPtr->id()], projMatrix());
  }

  mShaderChanged = false;
  mMatChanged = false;
}

void RenderManager::draw(VAOMesh& mesh) {
  update();
  mesh.draw();
}

void RenderManager::draw(EasyVAO& vao) {
  update();
  vao.draw();
}

static void updateVAO(EasyVAO& vao, const Mesh& m) {
  vao.primitive(m.primitive());
  vao.validate();
  vao.bind();
  vao.mNumVertices = static_cast<unsigned int>(m.vertices().size());
  vao.updateWithoutBinding(m.vertices().data(), sizeof(Vec3f),
                           m.vertices().size(), vao.mPositionAtt);
  vao.updateWithoutBinding(m.colors().data(), sizeof(Vec4f), m.colors().size(),
                           vao.mColorAtt);
  vao.updateWithoutBinding(m.texCoord2s().data(), sizeof(Vec2f),
                           m.texCoord2s().size(), vao.mTexcoord2dAtt);
  vao.updateWithoutBinding(m.normals().data(), sizeof(Vec3f),
                           m.normals().size(), vao.mNormalAtt);
  // unbind();
  vao.updateIndices(m.indices().data(), m.indices().size());
}

void RenderManager::draw(const Mesh& mesh) {
  // uses internal vao object.
  updateVAO(mInternalVAO, mesh);
  update();
  mInternalVAO.draw();
}

void RenderManager::draw(Mesh&& mesh) {
  // uses internal vao object.
  updateVAO(mInternalVAO, mesh);
  update();
  mInternalVAO.draw();
}
