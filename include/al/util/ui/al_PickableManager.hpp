
#ifndef __PICKABLEMANAGER_HPP__
#define __PICKABLEMANAGER_HPP__

#include <vector>

#include "al/core/graphics/al_Graphics.hpp"
#include "al/util/al_Ray.hpp"
#include "al/util/ui/al_Pickable.hpp"

namespace al {

struct Hit {
	bool hit;
	Rayd ray;
	double t;
	Pickable *p;

	Hit():hit(false){}
	Hit(bool h, Rayd r, double tt, Pickable* pp) : hit(h), ray(r), t(tt), p(pp){}
	// Vec3d operator(){ return ray(t); }
};

class PickableManager {
public:
	PickableManager(){ 
		mTranslating = true;
		mRotating = mScaling = mZooming = false;
	}

	PickableManager& registerPickable(Pickable &p){ mPickables.push_back(&p); return *this; }
	PickableManager& operator <<(Pickable &p){ return registerPickable(p); }
	PickableManager& operator <<(Pickable *p){ return registerPickable(*p); }

	Hit intersect(Rayd r){
		double tmin = 1e10;
		Pickable *pmin = NULL;
		for(Pickable *p : mPickables){
			double t = p->intersect(r);
			if(t >= 0.0 && t < tmin){
				pmin = p;
				tmin = t;
			}
		}
		if(pmin) return Hit(true, r, tmin, pmin);
		else return Hit(false, r, -1.0, NULL);
	}

	bool point(Rayd &r){
		Hit h = intersect(r);
		for(Pickable *p : mPickables){
			if(h.hit && p == h.p) p->hover = true;
			else p->hover = false;
		}
	}
	bool pick(Rayd &r){
		Hit h = intersect(r);
		for(Pickable *p : mPickables){
			if(h.hit && p == h.p){
				p->selected = true;
				p->prevPose.set(p->pose);
				lastSelect = h;
				selectOffset = p->pose.pos() - r(h.t)*p->scale;
			} else p->selected = false;
		}
	}
	bool drag(Rayd &r, int dy){
		// Hit h = intersect(r);
		for(Pickable *p : mPickables){
			if(p->selected){
				if(mZooming){
			        p->pose.pos().z += dy*0.04; // should move along dir to camera instead
		    	} else if(mRotating){
		    		Vec3f dir = r(lastSelect.t) - lastSelect.ray(lastSelect.t);
		    		Quatf q = Quatf().fromEuler(dir.x*0.01f, -dir.y*0.01f, 0);

		            Vec3f p1 = p->transformVecWorld(p->bb.cen);
		    		p->pose.quat() = q*p->prevPose.quat();
		            Vec3f p2 = p->transformVecWorld(p->bb.cen);
		            p->pose.pos() += p1-p2;

		    	} else if(mScaling){
			        p->scale += Vec3f(-dy*0.0005); // should move along dir to camera instead
		    	} else if(mTranslating){
			        Vec3f newPos = r(lastSelect.t)*p->scale + selectOffset;
			        p->pose.pos().set(newPos);
		    	} 
			}
		}
	}
	bool unpick(Rayd &r){
		for(Pickable *p : mPickables){
			if(!p->hover) p->selected = false;
		}
	}

	void onMouseMove(Graphics &g, const Mouse& m, int w, int h){
		Rayd r = getPickRay(g, m.x(), m.y(), w, h);
		point(r);
	}
	void onMouseDown(Graphics &g, const Mouse& m, int w, int h){
		Rayd r = getPickRay(g, m.x(), m.y(), w, h);
		pick(r);
	}
	void onMouseDrag(Graphics &g, const Mouse& m, int w, int h){
            if (m.right()) mRotating = true;
            else if (m.middle()) mScaling = true;
		Rayd r = getPickRay(g, m.x(), m.y(), w, h);
		drag(r,m.dy());
	}
	void onMouseUp(Graphics &g, const Mouse& m, int w, int h){
            mRotating = false;
            mScaling = false;
		Rayd r = getPickRay(g, m.x(), m.y(), w, h);
		unpick(r);
	}

	void onKeyDown(const Keyboard &k){
		switch (k.key()) {
          // case 't': mTranslating = true; break;
//          case 'r': mRotating = true; break;
          case 'z': mZooming = true; break;
//          case 's': mScaling = true; break;
          default: break;
    	}
	}
	void onKeyUp(const Keyboard &k){
		switch (k.key()) {
          // case 't': mTranslating = false; break;
//          case 'r': mRotating = false; break;
          case 'z': mZooming = false; break;
//          case 's': mScaling = false; break;
          default: break;
    	}
	}

protected:
	std::vector<Pickable *> mPickables;
	Hit lastSelect;
	Vec3d selectOffset;

	bool mTranslating, mRotating, mZooming, mScaling;

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
