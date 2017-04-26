#ifndef INC_GLV_ICON_H
#define INC_GLV_ICON_H

/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

namespace glv{

/*
These are intended to replace the function pointer icons in draw::.
They are a bit more difficult to handle than function pointers, such as when 
passing into a constructor. C++ polymorphism forces us to use either pointers
or references.
*/

struct Icon{
	virtual ~Icon(){}
	virtual void draw(float l, float t, float r, float b){}
};

struct Check : public Icon {
	void draw(float l, float t, float r, float b);
};

struct Cross : public Icon{
	void draw(float l, float t, float r, float b);
};

struct Rectangle : public Icon {
	void draw(float l, float t, float r, float b);
};

} // glv::
#endif
