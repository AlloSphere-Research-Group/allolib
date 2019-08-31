#include "al/graphics/al_Font.hpp"

#include <vector>
#include <cstdint>
#include <cstdio>
#include <cstring>

#define GLYPHS_PER_ROW 16	// number of glyphs to pack in a row
#define PIX_TO_EM (64.f)

namespace al{

struct Font::Impl {
	bool load(Font& font, const char* filename, int fontSize, bool antialias) {
        return false;
    }
    float ascender() const { return 0.0f; }
    float descender() const { return 0.0f; }
};


Font::Font()
:	mFontSize(12),
	mAntiAliased(true)
	// mTex(0, 0, Graphics::LUMINANCE, Graphics::UBYTE)
{
	align(0,0);
	// TODO: if this fails (mImpl == NULL), fall back to native options (e.g. Cocoa)?
	mImpl = new Impl;
}


Font::Font(const char* filename, int fontSize, bool antialias)
:	mFontSize(fontSize),
	mAntiAliased(antialias)
	// mTex(0, 0, Graphics::LUMINANCE, Graphics::UBYTE)
{
	align(0,0);
	// TODO: if this fails (mImpl == NULL), fall back to native options (e.g. Cocoa)?
	mImpl = new Impl;
    load(filename, fontSize, antialias);
}

Font::Font(Font&& other) noexcept {
    mImpl = other.mImpl;
    other.mImpl = nullptr;
}

Font& Font::operator=(Font&& other) noexcept {
    mImpl = other.mImpl;
    other.mImpl = nullptr;
    return *this;
}

Font::~Font() {
	delete mImpl;
}

bool Font::load(const char* filename, int fontSize, bool antialias){
	if(mImpl->load(*this, filename, fontSize, antialias)){
		mFontSize = fontSize;
		mAntiAliased = antialias;
		return true;
	}
	return false;
}

float Font::ascender() const { return mImpl->ascender(); }

float Font::descender() const { return mImpl->descender(); }

void Font::align(float xfrac, float yfrac){
	mAlign[0] = xfrac;
	mAlign[1] = yfrac;
}

void Font::write(Mesh& mesh, const char* text) {

	mesh.reset();
	mesh.primitive(Mesh::TRIANGLES);

	int nchars = (int)std::strlen(text);
	float margin = 2.;
	float csz = (float)mFontSize;
	float cdim = csz+margin;
	float tdim = cdim*GLYPHS_PER_ROW;
	float tcdim = ((float)cdim)/((float)tdim);

	float pos[] = {0., ascender() * mAlign[1]};

	if(mAlign[0] != 0){
		pos[0] = -width(text) * mAlign[0];
	}

	for(int i=0; i < nchars; i++) {
		int idx = text[i];
		const FontCharacter &c = mChars[idx];
		/*int margin = 1;*/

		int xidx = idx % GLYPHS_PER_ROW;
		int yidx = idx / GLYPHS_PER_ROW;
		float yy = c.y_offset;

		float tc_x0	= ((float)(xidx))*tcdim;
		float tc_y0	= ((float)(yidx))*tcdim;
		float tc_x1	= tc_x0+tcdim;
		float tc_y1	= tc_y0+tcdim;

		float v_x0  = pos[0] + c.x_offset;
		float v_x1	= v_x0+cdim;
		float v_y0	= margin+yy-pos[1];
		float v_y1	= yy-csz-pos[1];

		// draw character quad:
		mesh.texCoord(	tc_x0,	tc_y0);
		mesh.vertex(	v_x0,	v_y0,	0);

		mesh.texCoord(	tc_x1,	tc_y0);
		mesh.vertex(	v_x1,	v_y0,	0);

		mesh.texCoord(	tc_x0,	tc_y1);
		mesh.vertex(	v_x0,	v_y1,	0);

		mesh.texCoord(	tc_x0,	tc_y1);
		mesh.vertex(	v_x0,	v_y1,	0);

		mesh.texCoord(	tc_x1,	tc_y0);
		mesh.vertex(	v_x1,	v_y0,	0);

		mesh.texCoord(	tc_x1,	tc_y1);
		mesh.vertex(	v_x1,	v_y1,	0);

		pos[0] += (float)c.width;
	}
}

void Font::render(Graphics& g, const char* text) {
  write(mMesh, text);
  mTex.bind(0);
  g.texture();
  g.draw(mMesh);
  mTex.unbind(0);
}


float Font::width(const char* text) const {
  float total = 0.f;
  size_t len = std::strlen(text);
  for (size_t i=0; i < len; i++) {
    total += mChars[ (int)text[i] ].width;
  }
  return total;
}

} // al::
