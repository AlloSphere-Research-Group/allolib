#include "al/graphics/al_FBO.hpp"
// #include "al/graphics/al_Texture.hpp"
#include <stdio.h>
namespace al {

RBO::RBO(unsigned int format) : mFormat(format) {}

void RBO::onCreate() { glGenRenderbuffers(1, &mID); }

void RBO::onDestroy() { glDeleteRenderbuffers(1, &mID); }

unsigned int RBO::format() const { return mFormat; }

RBO& RBO::format(unsigned int v) {
  mFormat = v;
  return *this;
}

void RBO::bind() {
  validate();
  bind(id());
}

void RBO::unbind() { bind(0); }

bool RBO::resize(unsigned w, unsigned h) {
  bind();
  bool r = resize(format(), w, h);
  unbind();
  return r;
}

// static functions
unsigned RBO::maxSize() {
  int s;
  glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &s);
  return s;
}

void RBO::bind(unsigned id) { glBindRenderbuffer(GL_RENDERBUFFER, id); }

bool RBO::resize(unsigned int format, unsigned w, unsigned h) {
  unsigned mx = maxSize();
  if (w > mx || h > mx) return false;
  glRenderbufferStorage(GL_RENDERBUFFER, format, w, h);
  return true;
}

void FBO::onCreate() { glGenFramebuffers(1, &mID); }

void FBO::onDestroy() { glDeleteFramebuffers(1, &mID); }

FBO& FBO::attachRBO(const RBO& rbo, unsigned int attachment) {
  // bind();
  renderBuffer(rbo.id(), attachment);
  // unbind();
  return *this;
}

FBO& FBO::detachRBO(unsigned int attachment) {
  // bind();
  renderBuffer(0, attachment);
  // unbind();
  return *this;
}

FBO& FBO::attachTexture2D(Texture const& tex, unsigned int attachment,
                          int level) {
  // bind();
  texture2D(tex.id(), attachment, level);
  // unbind();
  return *this;
}

FBO& FBO::detachTexture2D(unsigned int attachment, int level) {
  // bind();
  texture2D(0, attachment, level);
  // unbind();
  return *this;
}

FBO& FBO::attachCubemapFace(Texture const& tex, unsigned int target_face,
                            unsigned int attachment, int level) {
  textureCubemapFace(tex.id(), target_face, attachment, level);
  return *this;
}

FBO& FBO::detachCubemapFace(unsigned int target_face, unsigned int attachment,
                            int level) {
  textureCubemapFace(0, target_face, attachment, level);
  return *this;
}

void FBO::bind() {
  validate();
  bind(id());
}

void FBO::unbind() { bind(0); }

GLenum FBO::status() {
  bind();
  int r = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  unbind();
  return r;
}

const char* FBO::statusString() { return statusString(status()); }

const char* FBO::statusString(GLenum stat) {
#define CS(v) \
  case v:     \
    return #v;
  switch (stat) {
    CS(GL_FRAMEBUFFER_COMPLETE)
    CS(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
    CS(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
    CS(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER)
    CS(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER)
    CS(GL_FRAMEBUFFER_UNSUPPORTED)
    default:
      return "GL_FRAMEBUFFER_UNKNOWN";
  };
}

// static functions
void FBO::bind(unsigned fboID) {
  // AL_GRAPHICS_ERROR("(before FBO::bind)", fboID);
  glBindFramebuffer(GL_FRAMEBUFFER, fboID);
  // AL_GRAPHICS_ERROR("binding FBO", fboID);
}

void FBO::renderBuffer(unsigned rboID, unsigned int attachment) {
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rboID);
}

void FBO::texture2D(unsigned texID, unsigned int attachment, int level) {
  glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texID,
                         level);
}

void FBO::textureCubemapFace(unsigned int texID, unsigned int target_face,
                             unsigned int attachment, int level) {
  glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, target_face, texID, 0);
}

}  // namespace al
