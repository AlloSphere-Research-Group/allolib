#include "al/graphics/al_EasyFBO.hpp"

using namespace al;

void EasyFBO::init(int width, int height, EasyFBOSetting const& setting) {
  mWidth = width;
  mHeight = height;

  mTex.filterMin(setting.filterMin);
  mTex.filterMag(setting.filterMag);
  mTex.wrapS(setting.wrapS);
  mTex.wrapT(setting.wrapT);
  mTex.wrapR(setting.wrapR);
  mTex.mipmap(setting.mUseMipmap);
  mTex.create2D(mWidth, mHeight, setting.internal, setting.format,
                setting.type);

  if (setting.use_depth_texture)
    mDepthTex.create2D(mWidth, mHeight, setting.depth_format,
                       GL_DEPTH_COMPONENT, GL_FLOAT);
  else
    mRbo.create(mWidth, mHeight, setting.depth_format);

  mFbo.bind();
  mFbo.attachTexture2D(mTex);
  if (setting.use_depth_texture)
    mFbo.attachTexture2D(mDepthTex, GL_DEPTH_ATTACHMENT);
  else
    mFbo.attachRBO(mRbo);

  // clear color attachments to black and depth to 1
  // prevents glitchy output for first frame
  float c[] = {0, 0, 0, 0};
  float d = 1;
  glClearBufferfv(GL_COLOR, 0, c);
  glClearBufferfv(GL_DEPTH, 0, &d);
  mFbo.unbind();
}
