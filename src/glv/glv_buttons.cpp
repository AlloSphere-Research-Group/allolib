/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include "al/glv/glv_buttons.h"

namespace glv {

Buttons::Buttons(
	const Rect& r, int nx, int ny, bool toggles, bool mutExc
): Widget(r, 2, toggles, mutExc, true) {	
	data().resize(Data::BOOL, nx,ny);
	useInterval(false);
}

bool Buttons::onEvent(Event::t e, GLV& g){
	if(!Widget::onEvent(e,g)) return false;

	switch(e){
	case Event::MouseDrag:
		if(enabled(SelectOnDrag)){
		
			int oldIdx = selected();
			selectFromMousePos(g);
			int idx = selected();
			
			if(idx != oldIdx &&  g.mouse().left()){
				if(enabled(Momentary)){
					setValue(false, oldIdx);
					setValue(true, idx);
				}
				else{
					setValue(getValue(oldIdx));
				}
			}
		}
		return false;

	case Event::MouseDown:
		if(g.mouse().left()){
			selectFromMousePos(g);
			setValue(enabled(Momentary) ? true : !getValue());
			return false;
		}
		break;

	case Event::KeyDown:
		switch(g.keyboard().key()){
		case ' ': setValue(enabled(Momentary) ? true : !getValue()); return false;
		}
		break;

	case Event::KeyUp:
		switch(g.keyboard().key()){
		case ' ': if(enabled(Momentary)) setValue(false); return false;
		}
		break;

	case Event::MouseUp: return false;

	default: break;
	}
	return true;
}


} // glv::
