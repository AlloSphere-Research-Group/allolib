/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include "al/glv/glv_core.h"

namespace glv{

GLV::GLV(space_t width, space_t height)
:	View(Rect(width, height)), mFocusedView(this)
{
	disable(DrawBorder | FocusHighlight | DrawBack);
//	cloneStyle();
	instances().push_back(this);
}

GLV::~GLV(){ //printf("~GLV\n");
	for(unsigned i=0; i<instances().size(); ++i){
		if(instances()[i] == this){
			instances().erase(instances().begin() + i);
		}
	}
}

void GLV::drawGLV(unsigned ww, unsigned wh, double dsec){
	drawWidgets(ww, wh, dsec);
}

void GLV::broadcastEvent(Event::t e){ 

	struct A : TraversalAction{
		GLV& glv; Event::t event;
		A(GLV& g, Event::t e): glv(g), event(e){}
		bool operator()(View * v, int depth){
			glv.doEventCallbacks(*v, event);
			return true;
		}
		bool operator()(const View * v, int depth){ return false; }
	} a(*this, e);
	
	traverseDepth(a);
}



// The bubbling return values from the virtual and function pointer callbacks
// are ANDed together.
bool GLV::doEventCallbacks(View& v, Event::t e){
//	printf("doEventCallbacks: %s %d\n", v.className(), e);

	// TODO: which is better?
//	if(!v.enabled(Controllable)) return false;	// cancels all events w/o handling
	if(!v.enabled(Controllable)) return true;	// bubbles all events w/o handling

//	bool bubble = v.onEvent(e, *this);					// Execute virtual callback
//	
//	if(bubble){
//		if(v.hasCallbacks(e)){
//			const eventCallbackList& cbl = v.callbackLists[e];
//			
//			// Execute callbacks in list
//			for(eventCallbackList::const_iterator it = cbl.begin(); it != cbl.end(); it++){
//				//if(*it) bubble |= (*it)(&v, *this);
//				if(*it){
//					bool r = (*it)(&v, *this);
//					bubble &= r;
//					if(!r) break;
//				}
//			}
//		}
//	}
//	
//	return bubble | v.enabled(AlwaysBubble);

//	bool bubble = true;
//
//	if(v.hasCallbacks(e)){
//		const eventCallbackList& cbl = v.callbackLists[e];
//		
//		// Execute callbacks in list
//		for(eventCallbackList::const_iterator it = cbl.begin(); it != cbl.end(); it++){
//			//if(*it) bubble |= (*it)(&v, *this);
//			if(*it){
//				bool r = (*it)(&v, *this);
//				bubble &= r;
//				if(!bubble) goto end;
//			}
//		}
//	}
//	
//	bubble &= v.onEvent(e, *this);
//	end:
//	return bubble || v.enabled(AlwaysBubble);

	bool bubble = true;

	if(v.hasEventHandlers(e)){
		const EventHandlers& hs = v.mEventHandlersMap()[e];

		// Execute callbacks in list
		for(EventHandlers::const_iterator it = hs.begin(); it != hs.end(); ++it){
			if(*it){
				bubble = (*it)->onEvent(v, *this);
				if(!bubble) break;
			}
		}
	}

	if(bubble) bubble = v.onEvent(e, *this);
	
	return bubble || v.enabled(AlwaysBubble);
}

void GLV::doFocusCallback(bool get){

	Event::t e = get ? Event::FocusGained : Event::FocusLost;

	if(mFocusedView){
		mFocusedView->focused(get);
		
//		if(mFocusedView->numEventHandlers(e)){
//			printf("GLV::doFocusCallback(%d) on a %s\n", get, mFocusedView->className());
//			eventType(e);
			doEventCallbacks(*mFocusedView, e);
//		}
	}
}

std::vector<GLV *>& GLV::instances(){
	static std::vector<GLV *> * sInstances = new std::vector<GLV *>;
	return *sInstances;
}

bool GLV::propagateEvent(){ //printf("GLV::propagateEvent(): %s\n", Event::getName(eventtype));
	View * v = mFocusedView;
	Event::t e = eventType();
	while(v && doEventCallbacks(*v, e)) v = v->parent;
	return v != 0;
}

void GLV::refreshModels(bool clearExistingModels){
	if(clearExistingModels) mMM.clearModels();
	addModels(mMM);
}

void GLV::setFocus(View * v){

	// save current event since we do not want to propagate GetFocus and LoseFocus
//	Event::t currentEvent = eventType();	

	// do nothing if already focused
	if(v == mFocusedView) return;

	// update states before calling event callbacks
	if(mFocusedView)	mFocusedView->disable(Focused);
	if(v)				v->enable(Focused);

	doFocusCallback(false);	// Call current focused View's LoseFocus callback
	mFocusedView = v;		// Set the currently focused View
	doFocusCallback(true);	// Call newly focused View's GetFocus callback

//	eventType(currentEvent);
}

void GLV::setKeyDown(int keycode){
	eventType(Event::KeyDown);
	mKeyboard.mKeycode = keycode;
	mKeyboard.mIsDown = true;
}

void GLV::setKeyUp(int keycode){
	eventType(Event::KeyUp);
	mKeyboard.mKeycode = keycode;
	mKeyboard.mIsDown = false;
}

void GLV::setKeyModifiers(bool shift, bool alt, bool ctrl, bool caps, bool meta){
	mKeyboard.shift(shift);
	mKeyboard.alt(alt);
	mKeyboard.ctrl(ctrl);
	mKeyboard.caps(caps);
	mKeyboard.meta(meta);
}

void GLV::setMouseDown(space_t& x, space_t& y, int button, int clicks){
	eventType(Event::MouseDown);
	if(!mMouse.isDownAny()) // only change focus if no other buttons down
		setFocus(findTarget(x, y));
	mMouse.posRel(x,y);
	mMouse.updateButton(button, true, clicks);
}

void GLV::setMouseUp(space_t& x, space_t& y, int button, int clicks){
	eventType(Event::MouseUp);
	if(absToRel(mFocusedView, x, y)){
		x -= mFocusedView->l;
		y -= mFocusedView->t;
	}
	mMouse.posRel(x,y);
	mMouse.updateButton(button, false, clicks);
}

void GLV::setMouseMotion(space_t& x, space_t& y, Event::t e){
	eventType(e);
	if(absToRel(mFocusedView, x, y)){
		x -= mFocusedView->l;
		y -= mFocusedView->t;
	}
}

void GLV::setMouseMove(space_t& x, space_t& y){
	setMouseMotion(x,y,Event::MouseMove);
}

void GLV::setMouseDrag(space_t& x, space_t& y){
	setMouseMotion(x,y,Event::MouseDrag);
}

void GLV::setMousePos(int x, int y, space_t relx, space_t rely){
	mMouse.pos(x, y);
	mMouse.posRel(relx, rely);
}

void GLV::setMouseWheel(int wheelDelta){
	eventType(Event::MouseWheel);
	mMouse.bufferPos(mMouse.mW[0] + (space_t)wheelDelta, mMouse.mW);
}

bool GLV::valid(const GLV * g){
	for(unsigned i=0; i<instances().size(); ++i){
		if(instances()[i] == g) return true;
	}
	return false;
}


} // glv::
