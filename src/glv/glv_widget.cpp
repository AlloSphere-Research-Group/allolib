#include "al/core/graphics/al_Mesh.hpp"
#include "al/glv/glv_widget.h"

namespace glv{

//bool Widget::widgetKeyDown(View * v, GLV& g){
//	Widget& w = *(Widget *)v;
//	switch(g.keyboard().key()){
//		case Key::Down:	++w.sy; break;
//		case Key::Up:	--w.sy; break;
//		case Key::Right:++w.sx; break;
//		case Key::Left:	--w.sx; break;
//		default: return true;
//	}
//	w.clipIndices();
//	return false;
//}


Widget::Widget(
	const Rect& r, space_t pad, bool moment, bool mutExc, bool drawGrid
)
:	View(r), sx(0), sy(0), mInterval(0,1), mPrevVal(0), mUseInterval(true)
{
	padding(pad);
	property(DrawGrid, drawGrid);
	property(MutualExc, mutExc);
	property(Momentary, moment);
//	addCallback(Event::KeyDown, widgetKeyDown);
}

bool Widget::onEvent(Event::t e, GLV& g){
	switch(e){
		case Event::KeyDown:
			switch(g.keyboard().key()){
//				case Key::Down:	if(sizeY()>1){ ++sy; clipIndices(); return false; }
//				case Key::Up:	if(sizeY()>1){ --sy; clipIndices(); return false; }
//				case Key::Right:if(sizeX()>1){ ++sx; clipIndices(); return false; }
//				case Key::Left:	if(sizeX()>1){ --sx; clipIndices(); return false; }
				case Key::Down:	if(sizeY()>1){ select(sx, sy+1); return false; }
				case Key::Up:	if(sizeY()>1){ select(sx, sy-1); return false; }
				case Key::Right:if(sizeX()>1){ select(sx+1, sy); return false; }
				case Key::Left:	if(sizeX()>1){ select(sx-1, sy); return false; }
				default:;
			}
			break;

		case Event::MouseUp:
			if(enabled(Momentary)) setValue(mPrevVal);
			return false;

//		case Event::MouseDown:
//			sx = (int)((g.mouse.xRel() / w) * sizeX());
//			sy = (int)((g.mouse.yRel() / h) * sizeY());
//			clipIndices();
//			return false;

		default:;
	}
	return true;
}

void Widget::onDataModelSync(){
	if(!hasVariables()) return;
	IndexDataMap::iterator it = variables().begin();

	for(; it!=variables().end(); ++it){
		int idx = it->first;

		if(validIndex(idx)){
			const Data& dat = it->second;
			if(data().slice(idx, data().size()-idx) != dat){
				assignData(dat, idx);
			}
		}
	}
}

// note: indices passed in are always valid
bool Widget::onAssignData(Data& d, int ind1, int ind2){
	if(data().isNumerical()){
		if(enabled(MutualExc)){
			double v = 0;
			if(useInterval()) v = glv::clip(v, max(), min());
			data().assignAll(v);
		}

		if(useInterval()){
			for(int i=0; i<d.size(); ++i){
				double v = d.at<double>(i);
				v = glv::clip(v, max(), min());
				d.assign(v, i);
			}
		}
	}

	int idx = data().indexFlat(ind1,ind2);	// starting index of model

	// Update any attached variables containing this index
	if(hasVariables()){
		for (auto& p : variables()) {
			auto& i = p.first;
			auto& v = p.second;

			int id0 = i;
			int id1 = id0 + v.size();
			int is0 = idx;
			int is1 = is0 + d.size();
			
			// the intersection
			int i0 = glv::max(id0, is0);
			int i1 = glv::min(id1, is1);
			
			if(i0 < i1){
				v.slice(i0-id0, i1-i0).assign(
					d.slice(i0-is0, i1-i0)
				);
			}
		}
	}
	
	Data modelOffset = data().slice(idx, data().size()-idx);

	if(d != modelOffset){
		data().assign(d, ind1, ind2);
		ModelChange modelChange(data(), idx);
		notify(this, Update::Value, &modelChange);
	}

	return true;
}

void Widget::selectFromMousePos(GLV& g){
	//if(containsPoint(g.mouse().xRel() + left(), g.mouse().yRel() + top())){
		select(
			(g.mouse().xRel() / w) * sizeX(),
			(g.mouse().yRel() / h) * sizeY()
		);
	//}
}

Widget& Widget::select(int ix, int iy){
	clipIndices(ix,iy);
	int iold = selected();
	int inew = data().indexFlat(ix,iy);
	if(iold != inew && data().size()){
		onCellChange(iold, inew);
		ChangedSelection csel = {iold, inew};
		notify(this, Update::Selection, &csel);
		sx=ix; sy=iy;
		mPrevVal = data().at<double>(selected());
	}
	return *this;
}

Widget& Widget::setValueMax(){
	for(int i=0; i<size(); ++i){ setValue(max(), i); } return *this;
}

Widget& Widget::setValueMid(){
	for(int i=0; i<size(); ++i){ setValue(mid(), i); } return *this;
}

} // glv::
