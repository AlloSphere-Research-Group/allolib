#include "al/core/graphics/al_RenderManager.hpp"

using namespace al;


void RenderManager::setClearColor(float r, float g, float b, float a) {
    mClearColor.set(r, g, b, a);
}

void RenderManager::clearColor(int drawbuffer) {
    glClearBufferfv(GL_COLOR, drawbuffer, mClearColor.components);
}

void RenderManager::clearColor(float r, float g, float b, float a, int drawbuffer) {
    setClearColor(r, g, b, a);
    clearColor(drawbuffer);
}

void RenderManager::clearColor(Color const& c, int drawbuffer) {
    setClearColor(c.r, c.g, c.b, c.a);
    clearColor(drawbuffer);
}

void RenderManager::setClearDepth(float d) {
    mClearDepth = d;
}

void RenderManager::clearDepth() {
    glClearBufferfv(GL_DEPTH, 0, &mClearDepth);
}

void RenderManager::clearDepth(float d) {
    setClearDepth(d);
    clearDepth();
}

void RenderManager::clear(float r, float g, float b, float a, float d, int drawbuffer) {
    clearColor(r, g, b, a, drawbuffer);
    clearDepth(d);
}


Matrix4f RenderManager::modelMatrix() {
    return mModelStack.get();
}

Matrix4f RenderManager::viewMatrix() {
  // return mViewMat;
  return mViewStack.get();
}

Matrix4f RenderManager::projMatrix() {
  // return mProjMat;
  return mProjStack.get();
}

void RenderManager::loadIdentity() { mModelStack.setIdentity(); }

void RenderManager::pushMatrix(){
    mModelStack.push();
    mMatChanged = true;
}
void RenderManager::popMatrix(){
    mModelStack.pop();
    mMatChanged = true;
}
void RenderManager::translate(float x, float y, float z){
    mModelStack.mult(Matrix4f::translation(x, y, z));
    mMatChanged = true;
}
void RenderManager::rotate(float angle, float x, float y, float z){
    mModelStack.mult(Matrix4f::rotate(angle, x, y, z));
    mMatChanged = true;
}
void RenderManager::rotate(const Quatf& q) {
  Matrix4f m;
  q.toMatrix(m.elems());
  mModelStack.mult(m);
  mMatChanged = true;
}
void RenderManager::scale(float s){
  scale(s, s, s);
}
void RenderManager::scale(float x, float y, float z){
  mModelStack.mult(Matrix4f::scaling(x, y, z));
  mMatChanged = true;
}


void RenderManager::viewport(int left, int bottom, int width, int height) {
  // mViewport.set(left, bottom, width, height);
  mViewportStack.set(left, bottom, width, height);
  glViewport(left, bottom, width, height);
}

void RenderManager::scissor(int left, int bottom, int width, int height) {
  glScissor(left, bottom, width, height);
}

void RenderManager::framebuffer(unsigned int id) {
  FBO::bind(id);
  mFBOID = id;
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

ShaderProgram& RenderManager::shader() {
  return *mShaderPtr;
}

void RenderManager::camera(Viewpoint const& v) {
  // mViewMat = v.viewMatrix();
  // mProjMat = v.projMatrix();
  mViewStack.set(v.viewMatrix());
  mProjStack.set(v.projMatrix());
  auto const& vp = v.viewport();
  viewport(vp.l, vp.b, vp.w, vp.h, (mFBOID == 0)? mWindowPtr->highres_factor() : 1);
  mMatChanged = true;
}

void RenderManager::camera(Viewpoint const& v, int w, int h) {
  camera(v, 0, 0, w, h);
}

void RenderManager::camera(Viewpoint const& v, int x, int y, int w, int h) {
  // mViewMat = v.viewMatrix();
  // mProjMat = v.projMatrix(x, y, w, h);
  mViewStack.set(v.viewMatrix());
  mProjStack.set(v.projMatrix(x, y, w, h));
  viewport(x, y, w, h, (mFBOID == 0)? mWindowPtr->highres_factor() : 1);
  mMatChanged = true;
}

void RenderManager::camera(Viewpoint::SpecialType v) {
  camera(v, 0, 0, mWindowPtr->width(), mWindowPtr->height());
}

void RenderManager::camera(Viewpoint::SpecialType v, int w, int h) {
  camera(v, 0, 0, w, h);
}

void RenderManager::camera(Viewpoint::SpecialType v, int x, int y, int w, int h) {
  switch (v) {

  case Viewpoint::IDENTITY: {
    // mViewMat = Matrix4f::identity();
    // mProjMat = Matrix4f::identity();
    mViewStack.setIdentity();
    mProjStack.setIdentity();
  }
  break;

  case Viewpoint::ORTHO_FOR_2D: {
    // 1. place eye so that bottom left is (0, 0), top right is (width, height)
    // 2. set lens to be ortho, with given width and height
    float half_w = (w - x) * 0.5f;
    float half_h = (h - y) * 0.5f;

    // 2D things will be drawn at z = 0 >> z = 1, near: 0.5, far: 1.5
    // mViewMat = Matrix4f::lookAt(
    //   Vec3f(half_w, half_h, 1), // eye
    //   Vec3f(half_w, half_h, 0), // at
    //   Vec3f(0, 1, 0) // up
    // );
    // mProjMat = Matrix4f::ortho(
    //   -half_w, half_w, // left, right
    //   -half_h, half_h, // bottom, top
    //   0.5f, 1.5f // near, far
    // );
    mViewStack.set(Matrix4f::lookAt(
      Vec3f(half_w, half_h, 1), // eye
      Vec3f(half_w, half_h, 0), // at
      Vec3f(0, 1, 0) // up
    ));
    mProjStack.set(Matrix4f::ortho(
      -half_w, half_w, // left, right
      -half_h, half_h, // bottom, top
      0.5f, 1.5f // near, far
    ));
  }
  break;

  case Viewpoint::UNIT_ORTHO:
    float spanx = float(w) / h;
    float spany = 1;
    if (spanx < 1) {
      spanx = 1;
      spany = float(h) / w;
    }
    // mViewMat = Matrix4f::identity();
    // mProjMat = Matrix4f::ortho(
    //   -spanx, spanx, // left, right
    //   -spany, spany, // bottom, top
    //   -0.5f, 0.5f // near, far
    // );
    mViewStack.setIdentity();
    mProjStack.set(Matrix4f::ortho(
      -spanx, spanx, // left, right
      -spany, spany, // bottom, top
      -0.5f, 0.5f // near, far
    ));
  break;
  }

  // viewport is in framebuffer unit
  viewport(x, y, w, h, (mFBOID == 0)? mWindowPtr->highres_factor() : 1);
  mMatChanged = true;
}

void RenderManager::pushCamera() {
  mViewStack.push();
  mProjStack.push();
  mViewportStack.push();
}
void RenderManager::popCamera() {
  mViewStack.pop();
  mProjStack.pop();
  mViewportStack.pop();
  viewport(mViewportStack.get());
  mMatChanged = true;
}

void RenderManager::update() {
    if (mShaderChanged || mMatChanged) {
        shader().uniform(modelviewLocs[mShaderPtr->id()], viewMatrix() * modelMatrix());
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