/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include "al/glv/glv_core.h"

namespace glv{

namespace Event{

	const char * toString(const Event::t e){
		#define CS(t) case Event::t: return #t;
		switch(e){
			CS(Null)
			CS(Quit)
			CS(WindowCreate) CS(WindowDestroy) CS(WindowResize)
			CS(FocusGained) CS(FocusLost)
			CS(MouseDown) CS(MouseUp) CS(MouseMove) CS(MouseDrag) CS(MouseWheel)
			CS(KeyDown) CS(KeyUp) CS(KeyRepeat)
			CS(Unused)
			default: return "?";
		};
		#undef CS
	}
}


Style::Style(bool deletable)
:	SmartPointer(deletable)
{}


StyleColor::StyleColor(){
	set(BlackOnWhite);
}



void StyleColor::set(Preset preset){
	switch(preset){
//		case BlackOnWhite:	back.set(1.0); border.set(0); fore.set(0.5); 
//							selection.set(0.7); text.set(0);
//							break;
//		case WhiteOnBlack:	back.set(0.0); border.set(1); fore.set(0.5);
//							selection.set(0.2); text.set(1);
//							break;
		case BlackOnWhite:	back.set(1.0f); border.set(0.4f); fore.set(0.0f); 
							selection.set(0.7f); text.set(0.0f);
							break;
		case WhiteOnBlack:	back.set(0.0f); border.set(0.5f); fore.set(1.0f);
							selection.set(0.2f); text.set(1.0f);
							break;
		case Gray:			back.set(0.6f); border.set(0.1f); fore.set(0.8f);
							selection.set(0.8f); text.set(0.0f);
							break;
		case SmokyGray:		set(al::Color(0.6f, 0.7f), 0.4f); break;

		default:;
	}
}


void StyleColor::set(const al::Color& c, float contrast){
	al::HSV hsv = c;
	float h = hsv.h;
	float s = hsv.s;
	float v = hsv.v;
	
	fore = c;

	float vt;
	vt = v + (v < 0.5 ? contrast : -contrast);
	back = al::Color(al::HSV(h,s,vt), c.a);

	vt = v + (v < 0.5 ? contrast/2 : -contrast/2);
	selection = al::Color(al::HSV(h,s,vt), c.a);
	
	//text = c.inverse().blackAndWhite();
	text = back.inverse().blackAndWhite();
	border = fore;
}

} // glv::
