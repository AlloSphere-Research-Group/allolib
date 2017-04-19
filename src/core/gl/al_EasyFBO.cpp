#include "al/core/gl/al_EasyFBO.hpp"

using namespace al;

void EasyFBO::init(int width, int height, EasyFBOSetting const& setting) {
	mWidth = width;
	mHeight = height;

	mTex.create2D(mWidth, mHeight, setting.internal, setting.format, setting.type);
	mRbo.create(mWidth, mHeight, setting.depth_format);

	// use temp binding unit to avoid overwriting other texture bindings
	// check al_Texture.hpp for more detail
	mTex.bind(AL_TEX_TEMP_BINDING_UNIT);
	mTex.filterMin(setting.filterMin);
	mTex.filterMag(setting.filterMag);
	mTex.wrapS(setting.wrapS);
	mTex.wrapT(setting.wrapT);
	mTex.wrapR(setting.wrapR);
	mTex.mipmap(setting.mUseMipmap);
	mTex.update();
	mTex.unbind(AL_TEX_TEMP_BINDING_UNIT);

	mFbo.bind();
	mFbo.attachTexture2D(mTex);
	mFbo.attachRBO(mRbo);
	mFbo.unbind();
}