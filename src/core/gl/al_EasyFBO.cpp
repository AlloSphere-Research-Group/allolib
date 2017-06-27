#include "al/core/gl/al_EasyFBO.hpp"

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
	mTex.create2D(mWidth, mHeight, setting.internal, setting.format, setting.type);

	mRbo.create(mWidth, mHeight, setting.depth_format);

	mFbo.bind();
	mFbo.attachTexture2D(mTex);
	mFbo.attachRBO(mRbo);
	mFbo.unbind();
}