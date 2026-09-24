#include "al/scene/al_Scene.hpp"

#include <algorithm>
#include <cstring>
#include <iostream>

using namespace al;

Scene::Scene(std::string name, TimeMasterMode masterMode)
    : mName(name), mMasterMode(masterMode) {}

Scene::~Scene() {
  auto deleteList = [](Entity *head) {
    while (head) {
      auto *next = head->mNext;
      delete head;
      head = next;
    }
  };
  deleteList(mActiveEntities);
  deleteList(mFreeEntities);
  deleteList(mEntitiesToInsert);
}

// ---- Registration and allocation ----

void Scene::rebuildArchetypeIndex() {
  std::sort(mArchetypes.begin(), mArchetypes.end(),
            [](const ArchetypeInfo &a, const ArchetypeInfo &b) {
              return a.name < b.name;
            });
  mArchetypeIndex.clear();
  for (size_t i = 0; i < mArchetypes.size(); ++i) {
    mArchetypeIndex[mArchetypes[i].name] = i;
  }
}

void Scene::registerEntity(const std::string &name, EntityFactory factory) {
  // Compute component state size by creating a prototype entity
  Entity prototype;
  factory(prototype);
  prototype.doInit();
  size_t stateSize = prototype.componentStateSize();

  mArchetypes.push_back({name, std::move(factory), stateSize});
  // Sorted-by-name so archetype indices match across cluster nodes
  rebuildArchetypeIndex();
}

Entity *Scene::allocateEntity(const std::string &name) {
  auto it = mArchetypeIndex.find(name);
  if (it == mArchetypeIndex.end()) {
    std::cerr << "Scene::allocateEntity: unknown archetype '" << name << "'"
              << std::endl;
    return nullptr;
  }
  Entity *entity = new Entity();
  entity->mNext = nullptr;
  entity->mArchetype = name;
  entity->mIsReplica = mIsReplica;
  mArchetypes[it->second].factory(*entity);
  entity->doInit();

  // If connected to a ParameterServer, register OSC callbacks on shared params
  if (mNotifier) {
    registerParameterCallbacks(entity);
  }

  return entity;
}

void Scene::allocatePool(const std::string &name, int count) {
  std::unique_lock<std::mutex> lk(mFreeLock);
  Entity *last = mFreeEntities;
  if (last) {
    while (last->mNext)
      last = last->mNext;
  }
  for (int i = 0; i < count; i++) {
    Entity *e = allocateEntity(name);
    if (!e)
      return;
    if (last) {
      last->mNext = e;
    } else {
      mFreeEntities = e;
    }
    last = e;
  }
}

Entity *Scene::getEntity(const std::string &name, bool forceAlloc) {
  std::unique_lock<std::mutex> lk(mFreeLock);

  if (!forceAlloc) {
    Entity *e = mFreeEntities;
    Entity *prev = nullptr;
    while (e) {
      if (e->mArchetype == name) {
        if (prev) {
          prev->mNext = e->mNext;
        } else {
          mFreeEntities = e->mNext;
        }
        e->mNext = nullptr;
        e->mIsReplica = false;
        return e;
      }
      prev = e;
      e = e->mNext;
    }
  }

  lk.unlock();
  return allocateEntity(name);
}

// ---- Triggering ----

int Scene::triggerOn(Entity *entity, int id) {
  if (!entity)
    return -1;

  if (id == -1) {
    id = mIdCounter++;
  }
  entity->mId = id;
  entity->doTriggerOn();

  std::unique_lock<std::mutex> lk(mInsertLock);
  entity->mNext = mEntitiesToInsert;
  mEntitiesToInsert = entity;
  return id;
}

void Scene::triggerOff(int id) {
  mIdsToTurnOff.write((char *)&id, sizeof(int));
}

void Scene::allNotesOff() { mAllNotesOff = true; }

// ---- Internal lifecycle processing ----

void Scene::processInsertions() {
  if (mInsertLock.try_lock()) {
    if (mEntitiesToInsert) {
      if (mActiveEntities) {
        auto *e = mEntitiesToInsert;
        while (e->mNext)
          e = e->mNext;
        e->mNext = mActiveEntities;
        mActiveEntities = mEntitiesToInsert;
      } else {
        mActiveEntities = mEntitiesToInsert;
      }
      mEntitiesToInsert = nullptr;
    }
    mInsertLock.unlock();
  }

  if (mAllNotesOff) {
    if (mFreeLock.try_lock()) {
      mAllNotesOff = false;
      if (mActiveEntities) {
        auto *e = mActiveEntities;
        Entity *last = e;
        while (e) {
          e->mId = -1;
          e->mActive = false;
          last = e;
          e = e->mNext;
        }
        last->mNext = mFreeEntities;
        mFreeEntities = mActiveEntities;
        mActiveEntities = nullptr;
      }
      mFreeLock.unlock();
    }
  }
}

void Scene::processTurnOffs() {
  int idsToTurnOff[16];
  size_t numBytes;
  while ((numBytes =
              mIdsToTurnOff.read((char *)idsToTurnOff, 16 * sizeof(int)))) {
    for (size_t i = 0; i < numBytes / sizeof(int); i++) {
      auto *e = mActiveEntities;
      while (e) {
        if (e->id() == idsToTurnOff[i]) {
          e->doTriggerOff();
        }
        e = e->mNext;
      }
    }
  }
}

void Scene::processInactive() {
  if (mFreeLock.try_lock()) {
    auto *e = mActiveEntities;
    Entity *prev = nullptr;
    while (e) {
      if (!e->active()) {
        e->doFree();
        e->mId = -1;

        if (prev) {
          prev->mNext = e->mNext;
          e->mNext = mFreeEntities;
          mFreeEntities = e;
          e = prev->mNext;
        } else {
          auto *next = e->mNext;
          mActiveEntities = next;
          e->mNext = mFreeEntities;
          mFreeEntities = e;
          e = mActiveEntities;
          continue;
        }
      } else {
        prev = e;
        e = e->mNext;
      }
    }
    mFreeLock.unlock();
  }
}

// ---- Rendering ----

void Scene::prepare(AudioIOData &io) {
  mInternalAudioIO.framesPerBuffer(io.framesPerBuffer());
  mInternalAudioIO.channelsOut(mEntityMaxOutputChannels);
  mInternalAudioIO.channelsIn(0);
  mAudioConfigured = true;
}

void Scene::update(double dt) {
  if (mMasterMode == TimeMasterMode::TIME_MASTER_UPDATE) {
    processInsertions();
    processTurnOffs();
  }

  auto *e = mActiveEntities;
  while (e) {
    if (e->active()) {
      e->doUpdate(dt);
    }
    e = e->mNext;
  }

  if (mMasterMode == TimeMasterMode::TIME_MASTER_UPDATE) {
    processInactive();
  }
}

void Scene::render(AudioIOData &io) {
  if (!mAudioConfigured) {
    prepare(io);
  }

  if (mMasterMode == TimeMasterMode::TIME_MASTER_AUDIO) {
    processInsertions();
    processTurnOffs();
  }

  auto *e = mActiveEntities;
  int fpb = io.framesPerBuffer();

  while (e) {
    if (e->active()) {
      mInternalAudioIO.zeroOut();
      mInternalAudioIO.frame(0);
      e->doAudio(mInternalAudioIO);

      unsigned int outChans = std::min((unsigned int)io.channelsOut(),
                                       e->numOutChannels());
      for (unsigned int c = 0; c < outChans; c++) {
        float *src = mInternalAudioIO.outBuffer(c);
        float *dst = io.outBuffer(c);
        for (int f = 0; f < fpb; f++) {
          dst[f] += src[f];
        }
      }
    }
    e = e->mNext;
  }

  if (mAudioGain != 1.0f) {
    for (unsigned int c = 0; c < io.channelsOut(); c++) {
      float *buf = io.outBuffer(c);
      for (int f = 0; f < fpb; f++) {
        buf[f] *= mAudioGain;
      }
    }
  }

  if (mMasterMode == TimeMasterMode::TIME_MASTER_AUDIO) {
    processInactive();
  }
}

void Scene::render(Graphics &g) {
  if (mMasterMode == TimeMasterMode::TIME_MASTER_GRAPHICS) {
    processInsertions();
    processTurnOffs();
  }

  auto *e = mActiveEntities;
  while (e) {
    if (e->active()) {
      g.pushMatrix();
      g.translate(e->pose().pos());
      g.rotate(e->pose().quat());
      g.scale(e->size());
      e->doDraw(g);
      g.popMatrix();
    }
    e = e->mNext;
  }

  if (mMasterMode == TimeMasterMode::TIME_MASTER_GRAPHICS) {
    processInactive();
  }
}

// ---- State synchronization ----

Entity *Scene::findEntityById(int32_t id) const {
  // Search active list
  auto *e = mActiveEntities;
  while (e) {
    if (e->mId == id)
      return e;
    e = e->mNext;
  }
  // Search insertion queue
  e = mEntitiesToInsert;
  while (e) {
    if (e->mId == id)
      return e;
    e = e->mNext;
  }
  return nullptr;
}

std::vector<int32_t> Scene::activeEntityIds() const {
  std::vector<int32_t> ids;
  for (auto *e = mActiveEntities; e; e = e->mNext) {
    if (e->active()) {
      ids.push_back(e->mId);
    }
  }
  return ids;
}

size_t Scene::packState(char *buf, size_t maxSize) const {
  char *p = buf;
  char *end = buf + maxSize;

  // Count active entities
  uint32_t numEntities = 0;
  auto *e = mActiveEntities;
  while (e) {
    if (e->active())
      numEntities++;
    e = e->mNext;
  }

  // Write entity count header
  if (p + sizeof(uint32_t) > end)
    return 0;
  std::memcpy(p, &numEntities, sizeof(uint32_t));
  p += sizeof(uint32_t);

  // Pack each active entity
  e = mActiveEntities;
  while (e) {
    if (e->active()) {
      auto it = mArchetypeIndex.find(e->mArchetype);
      if (it == mArchetypeIndex.end()) {
        e = e->mNext;
        continue;
      }

      uint16_t archIdx = (uint16_t)it->second;
      size_t entityBytes = ENTITY_HEADER_SIZE +
                           mArchetypes[archIdx].componentStateSize;
      if (p + entityBytes > end)
        break; // buffer full

      // Entity header
      int32_t id = e->mId;
      std::memcpy(p, &id, 4);
      p += 4;
      std::memcpy(p, &archIdx, 2);
      p += 2;

      // Pose: position (3 floats)
      Vec3d pos = e->mPose.pos();
      float fx = (float)pos.x, fy = (float)pos.y, fz = (float)pos.z;
      std::memcpy(p, &fx, 4);
      p += 4;
      std::memcpy(p, &fy, 4);
      p += 4;
      std::memcpy(p, &fz, 4);
      p += 4;

      // Pose: quaternion (4 floats: w, x, y, z)
      Quatd q = e->mPose.quat();
      float qw = (float)q.w, qx = (float)q.x, qy = (float)q.y,
            qz = (float)q.z;
      std::memcpy(p, &qw, 4);
      p += 4;
      std::memcpy(p, &qx, 4);
      p += 4;
      std::memcpy(p, &qy, 4);
      p += 4;
      std::memcpy(p, &qz, 4);
      p += 4;

      // Size
      float sz = e->mSize;
      std::memcpy(p, &sz, 4);
      p += 4;

      // Component states
      e->packComponentState(p);
      p += mArchetypes[archIdx].componentStateSize;
    }
    e = e->mNext;
  }

  return (size_t)(p - buf);
}

void Scene::unpackState(const char *buf, size_t size) {
  const char *p = buf;
  const char *end = buf + size;

  // Process any pending insertions first so active list is up to date
  processInsertions();
  processTurnOffs();

  if (size < sizeof(uint32_t))
    return;
  uint32_t numEntities;
  std::memcpy(&numEntities, p, sizeof(uint32_t));
  p += sizeof(uint32_t);

  // Track which IDs we receive (to detect removed entities)
  std::vector<int32_t> receivedIds;
  receivedIds.reserve(numEntities);

  for (uint32_t i = 0; i < numEntities; i++) {
    if (p + ENTITY_HEADER_SIZE > end)
      break;

    // Read entity header
    int32_t id;
    uint16_t archIdx;
    std::memcpy(&id, p, 4);
    p += 4;
    std::memcpy(&archIdx, p, 2);
    p += 2;

    if (archIdx >= mArchetypes.size()) {
      std::cerr << "Scene::unpackState: invalid archetype index " << archIdx
                << std::endl;
      break;
    }

    float fx, fy, fz;
    std::memcpy(&fx, p, 4);
    p += 4;
    std::memcpy(&fy, p, 4);
    p += 4;
    std::memcpy(&fz, p, 4);
    p += 4;

    float qw, qx, qy, qz;
    std::memcpy(&qw, p, 4);
    p += 4;
    std::memcpy(&qx, p, 4);
    p += 4;
    std::memcpy(&qy, p, 4);
    p += 4;
    std::memcpy(&qz, p, 4);
    p += 4;

    float sz;
    std::memcpy(&sz, p, 4);
    p += 4;

    size_t componentDataSize = mArchetypes[archIdx].componentStateSize;
    if (p + componentDataSize > end)
      break;

    receivedIds.push_back(id);

    // Find or create entity
    Entity *entity = findEntityById(id);
    if (!entity) {
      entity = getEntity(mArchetypes[archIdx].name);
      if (entity) {
        entity->mIsReplica = true;
        triggerOn(entity, id);
        // Process insertion immediately so we can find it next frame
        processInsertions();
      }
    }

    // Apply state
    if (entity) {
      entity->mPose.pos(Vec3d(fx, fy, fz));
      entity->mPose.quat().set(qw, qx, qy, qz);
      entity->mSize = sz;
      entity->unpackComponentState(p);
    }

    p += componentDataSize;
  }

  // Free active entities whose IDs were NOT in the received buffer
  auto *e = mActiveEntities;
  while (e) {
    if (e->active() && e->mIsReplica) {
      bool found = false;
      for (auto rid : receivedIds) {
        if (e->mId == rid) {
          found = true;
          break;
        }
      }
      if (!found) {
        e->free();
      }
    }
    e = e->mNext;
  }

  // Clean up freed entities
  processInactive();
}

// ---- Parameter sharing via OSC ----

void Scene::registerWithParameterServer(ParameterServer &server,
                                        bool isPrimary) {
  mIsReplica = !isPrimary;

  // Update replica flag on any already-allocated free entities
  auto *e = mFreeEntities;
  while (e) {
    e->mIsReplica = mIsReplica;
    e = e->mNext;
  }

  if (isPrimary) {
    registerNotifier(server);
  } else {
    server.registerOSCConsumer(this, mName);
  }
}

void Scene::registerNotifier(OSCNotifier &notifier) {
  mNotifier = &notifier;

  // Register callbacks on any already-allocated free entities
  auto *e = mFreeEntities;
  while (e) {
    registerParameterCallbacks(e);
    e = e->mNext;
  }
}

void Scene::registerParameterCallbacks(Entity *entity) {
  if (!mNotifier)
    return;
  for (auto *param : entity->sharedParameters()) {
    registerCallbackForParameter(entity, param);
  }
}

void Scene::registerCallbackForParameter(Entity *entity,
                                         ParameterMeta *param) {
  // Typed callbacks required by ParameterAPI; notify via codec registry.
  auto makeAddr = [this, entity](ParameterMeta *p) {
    return "/" + mName + "/entity/" + std::to_string(entity->id()) +
           p->getFullAddress();
  };

  auto notify = [this, entity, makeAddr](ParameterMeta *p) {
    if (mNotifier && entity->id() >= 0)
      mNotifier->notifyListeners(makeAddr(p), p, nullptr);
  };

  if (auto *p = dynamic_cast<Parameter *>(param)) {
    p->registerChangeCallback([notify, p](float) { notify(p); });
  } else if (auto *p = dynamic_cast<ParameterBool *>(param)) {
    p->registerChangeCallback([notify, p](float) { notify(p); });
  } else if (auto *p = dynamic_cast<ParameterInt *>(param)) {
    p->registerChangeCallback([notify, p](int32_t) { notify(p); });
  } else if (auto *p = dynamic_cast<ParameterString *>(param)) {
    p->registerChangeCallback([notify, p](std::string) { notify(p); });
  } else if (auto *p = dynamic_cast<ParameterPose *>(param)) {
    p->registerChangeCallback([notify, p](Pose) { notify(p); });
  } else if (auto *p = dynamic_cast<ParameterVec3 *>(param)) {
    p->registerChangeCallback([notify, p](Vec3f) { notify(p); });
  } else if (auto *p = dynamic_cast<ParameterVec4 *>(param)) {
    p->registerChangeCallback([notify, p](Vec4f) { notify(p); });
  } else if (auto *p = dynamic_cast<ParameterColor *>(param)) {
    p->registerChangeCallback([notify, p](Color) { notify(p); });
  } else {
    std::cerr << "Scene: unsupported parameter type for sharing: "
              << typeid(*param).name() << " " << param->getFullAddress()
              << std::endl;
  }
}

bool Scene::consumeMessage(osc::Message &m, std::string rootOSCPath) {
  std::string address = m.addressPattern();

  // Strip root path prefix if present
  if (rootOSCPath.size() > 0) {
    if (address.find(rootOSCPath, 0) == 1) {
      address = address.substr(rootOSCPath.size() + 1);
    } else {
      return false;
    }
  }

  // Expected format: /entity/<id>/<paramAddress>
  const std::string entityPrefix = "/entity/";
  if (address.compare(0, entityPrefix.size(), entityPrefix) != 0) {
    return false;
  }

  // Parse entity ID
  std::string rest = address.substr(entityPrefix.size());
  size_t slashPos = rest.find('/');
  if (slashPos == std::string::npos) {
    return false;
  }

  std::string idStr = rest.substr(0, slashPos);
  std::string paramAddr = rest.substr(slashPos);

  int entityId;
  try {
    entityId = std::stoi(idStr);
  } catch (...) {
    return false;
  }

  // Find the entity and apply the parameter value
  Entity *entity = findEntityById(entityId);
  if (!entity) {
    return false;
  }

  for (auto *param : entity->sharedParameters()) {
    if (ParameterServer::setParameterValueFromMessage(param, paramAddr, m)) {
      return true;
    }
  }

  return false;
}

// ---- Debug ----

void Scene::print(std::ostream &stream) {
  int activeCount = 0;
  auto *e = mActiveEntities;
  while (e) {
    activeCount++;
    e = e->mNext;
  }
  int freeCount = 0;
  e = mFreeEntities;
  while (e) {
    freeCount++;
    e = e->mNext;
  }
  stream << "Scene:" << std::endl;
  stream << "  Active entities: " << activeCount << std::endl;
  stream << "  Free entities:   " << freeCount << std::endl;
  stream << "  Registered archetypes: " << mArchetypes.size() << std::endl;
  for (size_t i = 0; i < mArchetypes.size(); i++) {
    stream << "    [" << i << "] " << mArchetypes[i].name
           << "  (state: " << mArchetypes[i].componentStateSize
           << " + " << ENTITY_HEADER_SIZE << " header bytes)" << std::endl;
  }
}
