
#ifndef __PICKABLEMANAGER_HPP__
#define __PICKABLEMANAGER_HPP__

#include <vector>
// #include <map>

#include "al/core/graphics/al_Graphics.hpp"
#include "al/util/al_Ray.hpp"
#include "al/util/ui/al_Pickable.hpp"

namespace al {


class PickableManager {
public:
	PickableManager(){}

	PickableManager& registerPickable(Pickable &p){ mPickables.push_back(&p); return *this; }
	PickableManager& operator <<(Pickable &p){ return registerPickable(p); }
	PickableManager& operator <<(Pickable *p){ return registerPickable(*p); }
	PickableManager& operator +=(Pickable &p){ return registerPickable(p); }
	PickableManager& operator +=(Pickable *p){ return registerPickable(*p); }

	std::vector<Pickable *> pickables(){ return mPickables; }

	Hit intersect(Rayd r){
		Hit hmin = Hit(false, r, 1e10, NULL);
		for(Pickable *p : mPickables){
			Hit h = p->intersect(r);
			if(h.hit && h.t < hmin.t){
					hmin = h;
			}
		}
		return hmin;
	}

<<<<<<< Updated upstream
	bool point(Rayd &r){
		Hit h = intersect(r);
		for(Pickable *p : mPickables){
			if(p == h.p){
				p->point(r);
				mLastPoint = h;
			} else if(p->hover.get()) p->hover = false;
		}
		return true;
	}

	bool pick(Rayd &r){
		Hit h = intersect(r);
		for(Pickable *p : mPickables){
			if(p == h.p){
				p->pick(r);
				mLastPick = h;
			} else if(p->selected.get()) p->selected = false;
		}
		return true;
	}

	bool drag(Rayd &r, Vec3f dv){
		for(Pickable *p : mPickables){
			if(p->selected.get()){
				if(mZooming){
			        Vec3f v = p->pose.get().pos();
			        v.z += dv.y * 0.04;
			        p->pose.setPos(v); // should move along dir to camera instead
		    	} else if(mRotating){
		    		Vec3f dir = r(mLastPick.t) - mLastPick(); 
		    		Quatf q = Quatf().fromEuler(dir.x*0.5f, -dir.y*0.5f, 0);

		            Vec3f p1 = p->transformVecWorld(p->bb.cen);
		    		p->pose.setQuat(q*p->prevPose.quat());
		            Vec3f p2 = p->transformVecWorld(p->bb.cen);
		            p->pose.setPos(p->pose.get().pos() + p1-p2);

		    	} else if(mScaling){
                    p->scale = p->scale - dv.y*0.01*p->scale; 
					if(p->scale < 0.0005) p->scale = 0.0005;
		    	} else if(mTranslating){
					p->drag(r);
					// mLastPoint = Hit(true, r, mLastPick.t, p); //?
			        // Vec3f newPos = r(mLastPick.t)*p->scaleVec.get() + selectOffset;
			        // p->pose.setPos(newPos);
		    	} 
			}
		}
		return true;
	}
	
	bool unpick(Rayd &r){
=======
	void event(PickEvent e){
		Hit h = intersect(e.ray);
>>>>>>> Stashed changes
		for(Pickable *p : mPickables){
			if(p == h.p || p->selected.get()){
				p->event(e);

				if(e.type == Point) mLastPoint = h;
				if(e.type == Pick) mLastPick = h;
			} else p->clearSelection(); //unpick?
		}	
	}

	void unhighlightAll() {
		for(Pickable *p : mPickables){
			if(p->hover.get()) p->hover = false;
		}
	}

	void onMouseMove(Graphics &g, const Mouse& m, int w, int h){
		Rayd r = getPickRay(g, m.x(), m.y(), w, h);
		event(PickEvent(Point, r));
	}
	void onMouseDown(Graphics &g, const Mouse& m, int w, int h){
		Rayd r = getPickRay(g, m.x(), m.y(), w, h);
		event(PickEvent(Pick, r));
	}
	void onMouseDrag(Graphics &g, const Mouse& m, int w, int h){
		Rayd r = getPickRay(g, m.x(), m.y(), w, h);
		if(m.right()) event(PickEvent(RotateRay, r));
		else if(m.middle()) event(PickEvent(Scale, r, m.dy()));
		else event(PickEvent(TranslateRay, r));
	}
	void onMouseUp(Graphics &g, const Mouse& m, int w, int h){
		Rayd r = getPickRay(g, m.x(), m.y(), w, h);
		event(PickEvent(Unpick, r));
	}

	void onKeyDown(const Keyboard &k){
		switch (k.key()) {
          default: break;
    	}
	}
	void onKeyUp(const Keyboard &k){
		switch (k.key()) {
          default: break;
    	}
	}

	Hit lastPoint(){ return mLastPoint; }
	Hit lastPick(){ return mLastPick; }

protected:
	std::vector<Pickable *> mPickables;
	// std::map<int, Hit> mHover;
	// std::map<int, Hit> mSelect;

	Hit mLastPoint;
	Hit mLastPick;
	Vec3d selectOffset;

	Vec3d unproject(Graphics &g, Vec3d screenPos, bool view=true){
		auto v = Matrix4d::identity();
		if(view) v = g.viewMatrix();
		auto mvp = g.projMatrix() * v * g.modelMatrix();
		Matrix4d invprojview = Matrix4d::inverse(mvp);
		Vec4d worldPos4 = invprojview.transform(screenPos);
		return worldPos4.sub<3>(0) / worldPos4.w;
	}

	Rayd getPickRay(Graphics &g, int screenX, int screenY, int width, int height, bool view=true){
		Rayd r;
		Vec3d screenPos;
		screenPos.x = (screenX*1. / width) * 2. - 1.;
		screenPos.y = ((height - screenY)*1. / height) * 2. - 1.;
		screenPos.z = -1.;
		Vec3d worldPos = unproject(g, screenPos, view);
		r.origin().set(worldPos);

		screenPos.z = 1.;
		worldPos = unproject(g, screenPos, view);
		r.direction().set( worldPos );
		r.direction() -= r.origin();
		r.direction().normalize();
		return r;
	}
};


} // ::al

#endif
