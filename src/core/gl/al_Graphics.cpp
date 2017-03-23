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

void Graphics::viewport(const Viewport& v){
  viewport_.set(v);
  gl_viewport();
}

void Graphics::viewport(int x, int y, int width, int height) {
  viewport_.set(x, y, width, height);
  gl_viewport();
}

void Graphics::gl_viewport() {
  glViewport(viewport_.l, viewport_.b, viewport_.w, viewport_.h);
}

Viewport Graphics::viewport() const {
  return viewport_;
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
  camera_changed_ = true;
  view_mat_ = v.viewMatrix();
  proj_mat_ = v.projMatrix();
  if (!viewport_.isEqual(v.viewport())) {
    viewport(v.viewport());
  }
}

void Graphics::texture(Texture& t, int binding_point) {
  auto search = textures_.find(binding_point);
  if(search != textures_.end()) { // previous binding exists
    if (search->second->id() == t.id()) { // and it was same texture
      // so do nothing and return
      // std::cout << "same texture" << std::endl;
      return;
    }
  }

  textures_[binding_point] = &t;
  textures_[binding_point]->bind(binding_point);
}

Texture& Graphics::texture(int binding_point) {
  return *(textures_[binding_point]);
}

void Graphics::draw(VAOMesh& mesh) {
  if (shader_ == nullptr) {
    AL_WARN_ONCE("shader not bound: bind shader by \"g.shader(myshader_);\"");
    return;
  }
  if (shader_changed_ || mat_changed_ || camera_changed_) {
    // 3rd parameter: "don't warn even if there's no such uniform"
    shader().uniform("MVP", proj_mat_ * view_mat_ * modelMatrix(), false);
  }
  if (shader_changed_ || mat_changed_) {
    shader().uniform("M", modelMatrix(), false);
  }
  if (shader_changed_ || camera_changed_) {
    shader().uniform("V", viewMatrix(), false);
    shader().uniform("P", projMatrix(), false);
  }
  shader_changed_ = false;
  mat_changed_ = false;
  camera_changed_ = false;
  mesh.draw();
}

Graphics& graphics() {
  static Graphics g;
  return g;
}



} // al::
