#include <stdio.h>

#include "al/core/gl/al_Graphics.hpp"

namespace al {

const char * Graphics::errorString(bool verbose){
  GLenum err = glGetError();
  #define CS(GL_ERR, desc) case GL_ERR: return verbose ? #GL_ERR ", " desc : #GL_ERR;
  switch(err){
    case GL_NO_ERROR: return "";
    CS(GL_INVALID_ENUM, "An unacceptable value is specified for an enumerated argument.")
    CS(GL_INVALID_VALUE, "A numeric argument is out of range.")
    CS(GL_INVALID_OPERATION, "The specified operation is not allowed in the current state.")
  #ifdef GL_INVALID_FRAMEBUFFER_OPERATION
    CS(GL_INVALID_FRAMEBUFFER_OPERATION, "The framebuffer object is not complete.")
  #endif
    CS(GL_OUT_OF_MEMORY, "There is not enough memory left to execute the command.")
    CS(GL_STACK_OVERFLOW, "This command would cause a stack overflow.")
    CS(GL_STACK_UNDERFLOW, "This command would cause a stack underflow.")
  #ifdef GL_TABLE_TOO_LARGE
    CS(GL_TABLE_TOO_LARGE, "The specified table exceeds the implementation's maximum supported table size.")
  #endif
    default: return "Unknown error code.";
  }
  #undef CS
}

bool Graphics::error(const char * msg, int ID){
  const char * errStr = errorString();
  if(errStr[0]){
    if(ID>=0) AL_WARN_ONCE("Error %s (id=%d): %s", msg, ID, errStr);
    else    AL_WARN_ONCE("Error %s: %s", msg, errStr);
    return true;
  }
  return false;
}

void Graphics::blendMode(BlendFunc src, BlendFunc dst, BlendEq eq){
  glBlendEquation(eq);
  glBlendFunc(src, dst);
}

void Graphics::capability(Capability cap, bool v){
  v ? enable(cap) : disable(cap);
}

void Graphics::blending(bool b){ capability(BLEND, b); }
void Graphics::colorMask(bool r, bool g, bool b, bool a){
  glColorMask(
    r?GL_TRUE:GL_FALSE,
    g?GL_TRUE:GL_FALSE,
    b?GL_TRUE:GL_FALSE,
    a?GL_TRUE:GL_FALSE
  );
}
void Graphics::colorMask(bool b){ colorMask(b,b,b,b); }
void Graphics::depthMask(bool b){ glDepthMask(b?GL_TRUE:GL_FALSE); }
void Graphics::depthTesting(bool b){ capability(DEPTH_TEST, b); }
void Graphics::scissorTest(bool b){ capability(SCISSOR_TEST, b); }
void Graphics::cullFace(bool b){ capability(CULL_FACE, b); }
void Graphics::cullFace(bool b, Face face) {
  capability(CULL_FACE, b);
  glCullFace(face);
}
void Graphics::pushMatrix(){
    mModelStack.push();
    mMatChanged = true;
}
void Graphics::popMatrix(){
    mModelStack.pop();
    mMatChanged = true;
}
void Graphics::translate(double x, double y, double z){
    mModelStack.mult(Matrix4f::translation(x, y, z));
    mMatChanged = true;
}
void Graphics::rotate(double angle, double x, double y, double z){
    mModelStack.mult(Matrix4f::rotate(angle, x, y, z));
    mMatChanged = true;
}
void Graphics::rotate(const Quatf& q) {
  Matrix4f m;
  q.toMatrix(m.elems());
  mModelStack.mult(m);
  mMatChanged = true;
}
void Graphics::scale(double s){
  scale(s, s, s);
}
void Graphics::scale(double x, double y, double z){
  mModelStack.mult(Matrix4f::scaling(x, y, z));
  mMatChanged = true;
}
void Graphics::lineWidth(float v) { glLineWidth(v); }
void Graphics::pointSize(float v) { glPointSize(v); }
void Graphics::polygonMode(PolygonMode m, Face f){ glPolygonMode(f,m); }

void Graphics::setClearColor(float r, float g, float b, float a) {
    mClearColor.set(r, g, b, a);
}

void Graphics::clearColor(int drawbuffer) {
    glClearBufferfv(GL_COLOR, drawbuffer, mClearColor.components);
}

void Graphics::clearColor(float r, float g, float b, float a, int drawbuffer) {
    setClearColor(r, g, b, a);
    clearColor(drawbuffer);
}

void Graphics::clearColor(Color const& c, int drawbuffer) {
    setClearColor(c.r, c.g, c.b, c.a);
    clearColor(drawbuffer);
}

void Graphics::setClearDepth(float d) {
    mClearDepth = d;
}

void Graphics::clearDepth() {
    glClearBufferfv(GL_DEPTH, 0, &mClearDepth);
}

void Graphics::clearDepth(float d) {
    setClearDepth(d);
    clearDepth();
}

void Graphics::uniformColor(float r, float g, float b, float a) {
  mUniformColor.set(r, g, b, a);
  mUniformColorChanged = true;
}
void Graphics::uniformColor(Color const& c) {
  mUniformColor = c;
  mUniformColorChanged = true;
}
Color Graphics::uniformColor() {
  return mUniformColor;
}

void Graphics::uniformColorMix(float m) {
  mUniformColorMix = m;
  mUniformColorChanged = true;
}
float Graphics::uniformColorMix() {
  return mUniformColorMix;
}

void Graphics::viewport(const Viewport& v){
  if (!mViewport.isEqual(v)) {
    mViewport.set(v);
    gl_viewport();
  }
}

void Graphics::viewport(int x, int y, int width, int height) {
  viewport(Viewport(x, y, width, height));
}

void Graphics::gl_viewport() {
  glViewport(mViewport.l, mViewport.b, mViewport.w, mViewport.h);
}

Viewport Graphics::viewport() const {
  return mViewport;
}

void Graphics::scissor(int left, int bottom, int width, int height) {
  scissor(Viewport(left, bottom, width, height));
}

void Graphics::scissor(const Viewport& v) {
  if (!mScissor.isEqual(v)) {
    mScissor.set(v);
    gl_scissor();
  }
}

void Graphics::gl_scissor() {
  glScissor(mScissor.l, mScissor.b, mScissor.w, mScissor.h);
}

Viewport Graphics::scissor() const {
  return mScissor;
}

void Graphics::shader(ShaderProgram& s) {
  if (mShaderPtr != nullptr && s.id() == mShaderPtr->id()) {
    // same shader
    return;
  }
  mShaderChanged = true;
  mShaderPtr = &s;
  mShaderPtr->use();
}

ShaderProgram& Graphics::shader() {
  return *mShaderPtr;
}

void Graphics::camera(Viewpoint& v) {
  mCameraChanged = true;
  mViewMat = v.viewMatrix();
  mProjMat = v.projMatrix();
  viewport(v.viewport());
}

void Graphics::camera(Viewpoint::SpecialType v) {
  camera(v, 0, 0, mWindow.fbWidth(), mWindow.fbHeight());
}

void Graphics::camera(Viewpoint::SpecialType v, int x, int y, int w, int h) {
  switch (v) {

  case Viewpoint::IDENTITY:
    mViewMat = Matrix4f::identity();
    mProjMat = Matrix4f::identity();
    viewport(x, y, w, h);
    mCameraChanged = true;
    break;

  case Viewpoint::ORTHO_FOR_2D:
    // 1. place eye so that bottom left is (0, 0), top right is (width, height)
    // 2. set lens to be ortho, with given (width and height)

    // viewport is in framebuffer unit, so get values in window pixel unit
    float half_w = (w - x) * 0.5f / mWindow.x_highres();
    float half_h = (h - y) * 0.5f / mWindow.y_highres();

    // z = 1 because 2D things will be drawn at z = 0
    // because of that we set near to 0.5 and far to 1.5
    mViewMat = Matrix4f::lookAt(
      Vec3f(half_w, half_h, 1), // eye
      Vec3f(half_w, half_h, 0), // at
      Vec3f(0, 1, 0) // up
    );
    mProjMat = Matrix4f::ortho(
      -half_w, half_w, // left, right
      -half_h, half_h, // bottom, top
      0.5f, 1.5f // near, far
    );
    viewport(x, y, w, h);
    mCameraChanged = true;
    break;
  }

}


void Graphics::texture(Texture& t, int binding_point) {
  auto search = mTextures.find(binding_point);
  if(search != mTextures.end()) { // previous binding exists
    if (search->second->id() == t.id()) { // and it was same texture
      // so do nothing and return
      // std::cout << "same texture" << std::endl;
      // return;
    }
  }

  mTextures[binding_point] = &t;
  mTextures[binding_point]->bind(binding_point);
}

Texture& Graphics::texture(int binding_point) {
  return *(mTextures[binding_point]);
}

void Graphics::draw(VAOMesh& mesh) {
  if (mShaderPtr == nullptr) {
    AL_WARN_ONCE("shader not bound: bind shader by \"g.shader(mymShaderPtr);\"");
    return;
  }
  if (mShaderChanged || mMatChanged || mCameraChanged) {
    // 3rd parameter: "don't warn even if there's no such uniform"
    shader().uniform("MVP", mProjMat * mViewMat * modelMatrix(), false);
  }
  if (mShaderChanged || mMatChanged) {
    shader().uniform("M", modelMatrix(), false);
  }
  if (mShaderChanged || mCameraChanged) {
    shader().uniform("V", viewMatrix(), false);
    shader().uniform("P", projMatrix(), false);
  }
  if (mShaderChanged || mUniformColorChanged) {
    shader().uniform("uniformColor", mUniformColor, false);
    shader().uniform("uniformColorMix", mUniformColorMix, false);
  }
  mUniformColorChanged = false;
  mShaderChanged = false;
  mMatChanged = false;
  mCameraChanged = false;
  mesh.draw();
}

void Graphics::draw(VAOMesh&& mesh) {
  draw(mesh);
}

void Graphics::framebuffer(FBO& fbo) {
  mFBOID = fbo.id();
  FBO::bind(mFBOID);
}

void Graphics::framebuffer(FBO::SpecialType fbo) {
  switch (fbo) {
    case FBO::DEFAULT: {
      // default window framebuffer
      mFBOID = 0;
      FBO::bind(0);
      break;
    }
  }
}
unsigned int Graphics::framebufferID() {
  return mFBOID;
}

} // al::
