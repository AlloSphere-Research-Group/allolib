#ifndef INC_GLV_BUTTONS_H
#define INC_GLV_BUTTONS_H

/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include "al/glv/glv_core.h"
#include "al/glv/glv_widget.h"

namespace glv {

/// One or more buttons on a grid
class Buttons : public Widget {
public:

	/// @param[in] r			geometry
	/// @param[in] nx			number along x
	/// @param[in] ny			number along y
	/// @param[in] momentary	whether the button state matches button press state
	/// @param[in] mutExc		whether multiple buttons can be on
	Buttons(
		const Rect& r=Rect(), int nx=1, int ny=1,
		bool momentary=false, bool mutExc=false
	);

	virtual const char * className() const { return "Buttons"; }
	virtual void onDraw(GLV& g);
	virtual bool onEvent(Event::t e, GLV& g);

	bool getValue() const { return Widget::getValue<bool>(); }
	bool getValue(int i) const { return Widget::getValue<bool>(i); }
	bool getValue(int i1, int i2) const { return Widget::getValue<bool>(i1, i2); }

protected:
};



/// Single button
class Button : public Buttons {
public:
	/// @param[in] r			geometry
	/// @param[in] momentary	whether the button state matches button press state
	Button(const Rect& r=Rect(20), bool momentary=false)
	: Buttons(r, 1,1, momentary, false)	{}
	
	virtual const char * className() const { return "Button"; }
};


} // glv::

#endif

