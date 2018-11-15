
#ifndef __PICKABLE_HPP__
#define __PICKABLE_HPP__

#include <vector>

#include "al/util/al_Ray.hpp"
#include "al/util/ui/al_BoundingBox.hpp"
#include "al/util/ui/al_Parameter.hpp"
#include "al/util/ui/al_ParameterBundle.hpp"

namespace al {


struct PickableState {
  std::string name;
  ParameterBool hover{"hover", name}, selected{"selected", name};
  ParameterPose pose{"pose", name};
  ParameterVec3 scaleVec{"scaleVec", name};
  Parameter scale{"scale", name, 1.0f, "", 0.0f, 10.0f};

  ParameterBundle bundle {"pickable"};

  PickableState(){
    pose = Pose();
    scaleVec = Vec3f(1);
    hover = false;
    selected = false;
    bundle << hover << selected << pose << scale << scaleVec;
    hover.setHint("latch", 1.0);
    selected.setHint("latch", 1.0);
    scale.registerChangeCallback([this](float value) {
        scaleVec.set(Vec3f(value, value, value));
    });
    hover.setHint("hide", 1.0);
    selected.setHint("hide", 1.0);
    scaleVec.setHint("hide", 1.0); // We want to show the single value scale by default.
  }
};

struct PickableBase : virtual PickableState {
  PickableBase *parent = 0;
  std::vector<PickableBase *> children;
  bool alwaysTestChildren = true;

  // initial values, and previous values
  Pose pose0, prevPose;
  Vec3f scale0, prevScale;

  /// intersection test must be specified
  virtual double intersect(Rayd &r) = 0;

  /// override these callbacks
  virtual bool onPoint(Rayd &r, double t, bool child){return false;}
  virtual bool onPick(Rayd &r, double t, bool child){return false;}
  virtual bool onDrag(Rayd &r, double t, bool child){return false;}
  virtual bool onUnpick(Rayd &r, double t, bool child){return false;}
  
  /// do interaction on self and children, call onPoint callbacks
  virtual bool point(Rayd &r){
    bool child = false;  
    double t = intersect(r);
    if(t > 0.0 || alwaysTestChildren){
      for(unsigned int i=0; i < children.size(); i++){
        Rayd ray = transformRayLocal(r);
        child |= children[i]->point(ray);
      }
    }
    return onPoint(r,t,child);
  }

  /// do interaction on self and children, call onPick callbacks
  virtual bool pick(Rayd &r){
    bool child = false;  
    double t = intersect(r);
    if(t > 0.0 || alwaysTestChildren){
      for(unsigned int i=0; i < children.size(); i++){
        Rayd ray = transformRayLocal(r);
        child |= children[i]->pick(ray);
      }
    }
    return onPick(r,t,child);
  }

  /// do interaction on self and children, call onDrag callbacks
  virtual bool drag(Rayd &r){
    bool child = false;  
    double t = intersect(r);
    if(t > 0.0 || alwaysTestChildren){
      for(unsigned int i=0; i < children.size(); i++){
        Rayd ray = transformRayLocal(r);
        child |= children[i]->drag(ray);
      }
    }
    return onDrag(r,t,child);
  }
  
  /// do interaction on self and children, call onUnpick callbacks
  virtual bool unpick(Rayd &r){
    bool child = false;  
    double t = intersect(r);
    if(t > 0.0 || alwaysTestChildren){
      for(unsigned int i=0; i < children.size(); i++){
        Rayd ray = transformRayLocal(r);
        child |= children[i]->unpick(ray);
      }
    }
    return onUnpick(r,t,child);
  }

  virtual void draw(Graphics &g){}
  virtual void drawChildren(Graphics &g){
    pushMatrix(g);
    for(unsigned int i=0; i < children.size(); i++){
      children[i]->draw(g);
    }
    popMatrix(g);
  }

  bool intersects(Rayd &r){ return intersect(r) > 0.0; }
  bool intersectsChild(Rayd &r){
    bool child = false;
    for(unsigned int i=0; i < children.size(); i++){
      Rayd ray = transformRayLocal(r);
      child |= children[i]->intersects(ray);
    }
    return child;
  }

  void addChild(PickableBase &pickable){
    pickable.parent = this;
    children.push_back(&pickable);
  }

  /// apply pickable pose transforms
  inline void pushMatrix(Graphics &g){
    g.pushMatrix();
    g.translate(pose.get().pos());
    g.rotate(pose.get().quat());
    g.scale(scaleVec.get());
  }
  /// pop matrix.
  inline void popMatrix(Graphics &g){
    g.popMatrix();
  } 

  /// transform a ray in world space to local space
  Rayd transformRayLocal(const Rayd &ray){
    Matrix4d t,r,s;
    Matrix4d model = t.translation(pose.get().pos()) * r.fromQuat(pose.get().quat()) * s.scaling(scaleVec.get());
    Matrix4d invModel = Matrix4d::inverse(model);
    Vec4d o = invModel.transform(Vec4d(ray.o, 1));
    Vec4d d = invModel.transform(Vec4d(ray.d, 0));
    return Rayd(o.sub<3>(0), d.sub<3>(0));
  }

  /// transfrom a vector in local space to world space
  Vec3f transformVecWorld(const Vec3f &v, float w=1){
    Matrix4d t,r,s;
    Matrix4d model = t.translation(pose.get().pos()) * r.fromQuat(pose.get().quat()) * s.scaling(scaleVec.get());
    Vec4d o = model.transform(Vec4d(v, w));
    return Vec3f(o.sub<3>(0));
  }  
  /// transfrom a vector in world space to local space
  Vec3f transformVecLocal(const Vec3f &v, float w=1){
    Matrix4d t,r,s;
    Matrix4d invModel = t.translation(pose.get().pos()) * r.fromQuat(pose.get().quat()) * s.scaling(scaleVec.get());
    Vec4d o = invModel.transform(Vec4d(v, w));
    return Vec3f(o.sub<3>(0));
  }
};


/// Bounding Box PickableMesh
struct Pickable : PickableBase {
  Mesh *mesh; // pointer to mesh that is wrapped
  BoundingBox bb; // original bounding box
  BoundingBox aabb; // axis aligned bounding box (after pose/scale transforms)

  // used for moving pickable naturally
  Vec3f selectOffset;
  float selectDist;

  Pickable(std::string name = ""){ PickableState::name = name;}
  Pickable(Mesh &m){set(m);}

  /// initialize bounding box;
  void set(Mesh &m){
    mesh = &m;
    bb.set(*mesh);
  }

  /// override base methods
  double intersect(Rayd &r){
    return intersectBB(r);
  }

  bool onPoint(Rayd &r, double t, bool child){
    bool hoverValue = false;
    if(t > 0.0){
      if(!child){
        hoverValue = true;
      }
    }
    if (hover.get() != hoverValue) {
        hover = hoverValue; // setting value propagates via OSC, so only set if there is a change
    }

    return hover.get() || child;
  }

  bool onPick(Rayd &r, double t, bool child){
    bool selectedValue = false;
    if(t > 0.0){
      if(!child){
        prevPose.set(pose.get());
        selectDist = t;
        selectOffset = pose.get().pos() - r(t)*scaleVec.get();
        selectedValue = true;
      }
    }
    if (selected != selectedValue) {
        selected = selectedValue; // to avoid triggering change callback if no change
    }
    return selected.get() || child;
  }
  
  bool onDrag(Rayd &r, double t, bool child){
    // if(t > 0.0){
      if(child){
        return true;
      } else if(selected.get()){
        Vec3f newPos = r(selectDist)*scaleVec.get() + selectOffset;
        pose = Pose(newPos, pose.get().quat());
        return true;
      }
    // }
    return false;
  }

  bool onUnpick(Rayd &r, double t, bool child){
    if(!hover && selected) selected = false;
    return false;
  }

  // draw
  void draw(Graphics &g){
    pushMatrix(g);
    // glPushAttrib(GL_CURRENT_BIT);
    if(mesh) g.draw(*mesh);

    if(selected.get()) g.color(0,1,1);
    else if(hover.get()) g.color(1,1,1);
    bb.draw(g, false);

    // glPopAttrib();
    popMatrix(g);
    // drawChildren(g);
  }

  void drawMesh(Graphics &g){
    if(!mesh) return;
    pushMatrix(g);
    g.draw(*mesh);
    popMatrix(g);
  }
  void drawBB(Graphics &g){
    if(!selected.get() && !hover.get()) return;
    pushMatrix(g);
    // glPushAttrib(GL_CURRENT_BIT);
    if(selected.get()) g.color(0,1,1);
    else if(hover.get()) g.color(1,1,1);
    bb.draw(g, false);
    // g.draw(bb.mesh);
    // g.draw(bb.tics);
    // glPopAttrib();
    popMatrix(g);
  }

  /// set the pickable's center position
  void setCenter(Vec3f& pos){
    updateAABB();
    Vec3f offset = aabb.cen - pose.get().pos();
    pose = Pose(pos - offset, pose.get().quat());
  }

  /// set pickable's orientation maintaining same center position
  void setQuat(Quatf& q){
    updateAABB();
    Vec3f cen;
    cen.set(aabb.cen);
    pose = Pose(pose.get().pos(), q);
    setCenter(cen);
  }

  /// intersect ray with pickable BoundingBox
  double intersectBB(Rayd ray){
    Rayd r = transformRayLocal(ray);
    return r.intersectBox(bb.cen, bb.dim);
  }

  /// intersect ray with pickable AxisAlignedBoundingBox
  double intersectAABB(Rayd ray){
    return ray.intersectBox(aabb.cen, aabb.dim);
  }

  /// intersect ray with bounding sphere
  float intersectBoundingSphere(Rayd ray){
    return ray.intersectSphere( transformVecWorld(bb.cen), (bb.dim*scaleVec.get()).mag()/2.0);
  }

  /// calculate Axis aligned bounding box from mesh bounding box and current transforms
  void updateAABB(){
    // thanks to http://zeuxcg.org/2010/10/17/aabb-from-obb-with-component-wise-abs/
    Matrix4d t,r,s;
    Matrix4d model = t.translation(pose.get().pos()) * r.fromQuat(pose.get().quat()) * s.scaling(scaleVec.get());
    Matrix4d absModel(model);
    for(int i=0; i<16; i++) absModel[i] = std::abs(absModel[i]);
    Vec4d cen = model.transform(Vec4d(bb.cen, 1));
    Vec4d dim = absModel.transform(Vec4d(bb.dim, 0));
    aabb.setCenterDim(cen.sub<3>(0),dim.sub<3>(0));
  }

};

} // ::al

#endif
