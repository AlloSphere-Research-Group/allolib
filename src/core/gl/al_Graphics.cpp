#include <stdio.h>

#include "al/core/system/al_Printing.hpp"
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

void Graphics::viewport(int x, int y, int width, int height) {
  glViewport(x, y, width, height);
}

Viewport Graphics::viewport() const {
  GLint vp[4];
  glGetIntegerv(GL_VIEWPORT, vp);
  return Viewport(vp[0], vp[1], vp[2], vp[3]);
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
    model_stack.push();
}
void Graphics::popMatrix(){
    model_stack.pop();
}
void Graphics::translate(double x, double y, double z){
    model_stack.mult(Matrix4f::translation(x, y, z));
    mat_changed_ = true;
}
void Graphics::rotate(double angle, double x, double y, double z){
    model_stack.mult(Matrix4f::rotate(angle, x, y, z));
    mat_changed_ = true;
}
void Graphics::rotate(const Quatf& q) {
  Matrix4f m;
  q.toMatrix(m.elems());
  model_stack.mult(m);
  mat_changed_ = true;
}
void Graphics::scale(double s){
  scale(s, s, s);
}
void Graphics::scale(double x, double y, double z){
  model_stack.mult(Matrix4f::scaling(x, y, z));
  mat_changed_ = true;
}
void Graphics::lineWidth(float v) { glLineWidth(v); }
void Graphics::pointSize(float v) { glPointSize(v); }
void Graphics::polygonMode(PolygonMode m, Face f){ glPolygonMode(f,m); }

void Graphics::setClearColor(float r, float g, float b, float a) {
    mClearColor.set(r, g, b, a);
}

void Graphics::clear(int drawbuffer) {
    glClearBufferfv(GL_COLOR, drawbuffer, mClearColor.components);
}

void Graphics::clear(float r, float g, float b, float a, int drawbuffer) {
    setClearColor(r, g, b, a);
    clear(drawbuffer);
}

void Graphics::clear(Color const& c, int drawbuffer) {
    setClearColor(c.r, c.g, c.b, c.a);
    clear(drawbuffer);
}

void Graphics::setClearDepth(float d) {
    mClearDepth = d;
}

void Graphics::clearDepth() {
    glClearBufferfv(GL_COLOR, 0, &mClearDepth);
}

void Graphics::clearDepth(float d) {
    setClearDepth(d);
    clearDepth();
}

void Graphics::shader(ShaderProgram& s) {
  if (shader_ != nullptr && s.id() == shader_->id()) {
    // same shader
    return;
  }
  shader_changed_ = true;
  shader_ = &s;
  shader_->use();
}

ShaderProgram& Graphics::shader() {
  return *shader_;
}

void Graphics::camera(Viewpoint& v) {
  mat_changed_ = true;
  view_mat_ = v.viewMatrix();
  proj_mat_ = v.projMatrix();
  if (!viewport_.isEqual(v.viewport())) {
    
  }
}

void Graphics::draw(VAOMesh& mesh) {
  if (shader_ == nullptr) {
    AL_WARN_ONCE("shader not bound: bind shader by \"g.shader(myshader_);\"");
    return;
  }
  if (shader_changed_ || mat_changed_) {
    shader().uniform("MVP", proj_mat_ * view_mat_ * modelMatrix());
    shader_changed_ = false;
    mat_changed_ = false;
  }
  mesh.draw();
}


} // al::
