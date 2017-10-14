#include "al/core/graphics/al_EasyFBO.hpp"

using namespace al;

void EasyFBO::init(int width, int height, EasyFBOSetting const& setting) {
    mWidth = width;
    mHeight = height;

    mTex.filterMin(setting.filterMin);
    mTex.filterMag(setting.filterMag);
    mTex.wrapS(setting.wrapS);
    mTex.wrapT(setting.wrapT);
    mTex.wrapR(setting.wrapR);
    // mTex.mipmap(setting.mUseMipmap);
    mTex.create2D(mWidth, mHeight, setting.internal, setting.format, setting.type);

    mRbo.create(mWidth, mHeight, setting.depth_format);

    mFbo.bind();
    mFbo.attachTexture2D(mTex);
    mFbo.attachRBO(mRbo);
    float c[] = {0, 0, 0, 0};
    float d = 1;
    glClearBufferfv(GL_COLOR, 0, c);
    glClearBufferfv(GL_DEPTH, 0, &d);
    mFbo.unbind();
}