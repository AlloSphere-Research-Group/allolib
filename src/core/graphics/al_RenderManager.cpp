#include "al/core/graphics/al_RenderManager.hpp"

using namespace al;

ShaderProgram* RenderManager::mShaderPtr = nullptr;
std::unordered_map<unsigned int, int> RenderManager::modelviewLocs;
std::unordered_map<unsigned int, int> RenderManager::projLocs;
bool RenderManager::mShaderChanged = false;
bool RenderManager::mMatChanged = false;
ViewportStack RenderManager::mViewportStack;
EasyVAO RenderManager::mInternalVAO;
// unsigned int RenderManager::mFBOID = 0;
FBOStack RenderManager::mFBOStack;



//______________________________________________________________________________

MatrixStack::MatrixStack() { stack.emplace_back(); stack.reserve(10); }
void MatrixStack::mult(Matrix4f const& m) { stack.back() = stack.back() * m; }
void MatrixStack::set(const Matrix4f& m) { stack.back() = m; }
void MatrixStack::setIdentity() { stack.back().setIdentity(); }
Matrix4f MatrixStack::get() const { return stack.back(); }

void MatrixStack::push() {
  Matrix4f m = stack.back();
  stack.push_back(m);
}

void MatrixStack::pop() { stack.pop_back(); }
void MatrixStack::pop_all() { stack.clear(); stack.emplace_back(); }



//______________________________________________________________________________

ViewportStack::ViewportStack() { stack.emplace_back(); stack.reserve(10); }
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

FBOStack::FBOStack() { stack.emplace_back(); stack.reserve(10); }
void FBOStack::push() { unsigned int i = stack.back(); stack.push_back(i); }
void FBOStack::pop() { stack.pop_back(); }
unsigned int FBOStack::get() const { return stack.back(); }
void FBOStack::set(unsigned int id) { stack.back() = id; }



//______________________________________________________________________________

void RenderManager::translate(float x, float y, float z) {
  mModelStack.mult(Matrix4f::translation(x, y, z));
  mMatChanged = true;
}

void RenderManager::rotate(float angle, float x, float y, float z) {
  mModelStack.mult(Matrix4f::rotate(angle, x, y, z));
  mMatChanged = true;
}

void RenderManager::rotate(const Quatf& q) {
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

void RenderManager::pushFramebuffer() {
  mFBOStack.push();
}
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
    modelviewLocs[mShaderPtr->id()] = mShaderPtr->getUniformLocation("MV");
  }

  auto pr_search = projLocs.find(mShaderPtr->id());
  if (pr_search == projLocs.end()) {
    projLocs[mShaderPtr->id()] = mShaderPtr->getUniformLocation("P");
  }
}

void RenderManager::camera(Viewpoint const& v) {
  mViewStack.set(v.viewMatrix());
  mProjStack.set(v.projMatrix(viewport().aspect()));
  mMatChanged = true;
}

void RenderManager::camera(Viewpoint::SpecialType v) {
  switch (v) {
    case Viewpoint::IDENTITY: {
      mViewStack.setIdentity();
      mProjStack.setIdentity();
    } break;

    case Viewpoint::ORTHO_FOR_2D: {
      // 1. place eye so that bot-left (0, 0), top right (width, height)
      // 2. set lens to be ortho, with given width and height
      // 2D things will be drawn at z = 0 so cam_z = 1, near: 0.5, far: 1.5
      auto v = viewport();
      float half_w = (v.w - v.l) * 0.5f;
      float half_h = (v.h - v.b) * 0.5f;
      mViewStack.set(Matrix4f::lookAt(Vec3f(half_w, half_h, 1),  // eye
                                      Vec3f(half_w, half_h, 0),  // at
                                      Vec3f(0, 1, 0)             // up
                                      ));
      mProjStack.set(Matrix4f::ortho(-half_w, half_w,  // left, right
                                     -half_h, half_h,  // bottom, top
                                     0.5f, 1.5f        // near, far
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
                                     -0.5f, 0.5f     // near, far
                                     ));
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

void RenderManager::draw(Mesh& mesh) {
  // uses internal vao object.
  mInternalVAO.update(mesh);
  update();
  mInternalVAO.draw();
}

void RenderManager::draw(Mesh&& mesh) {
  // uses internal vao object.
  mInternalVAO.update(mesh);
  update();
  mInternalVAO.draw();
}