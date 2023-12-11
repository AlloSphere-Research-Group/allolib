#include "al/graphics/al_GPUObject.hpp"

using namespace al;

GPUObject::GPUObject() : mID(0), mResubmit(false) {}

// should call destroy() in child's dtor, since we need the virtual one to be
// called
GPUObject::~GPUObject() {}

void GPUObject::validate() {
  if (mResubmit) {
    destroy();
    mResubmit = false;
  }
  if (!created()) create();
}

void GPUObject::invalidate() { mResubmit = true; }

bool GPUObject::created() const { return id() != 0; }

void GPUObject::create() {
  if (created()) destroy();
  onCreate();
}

void GPUObject::destroy() {
  if (created()) onDestroy();
  mID = 0;
}