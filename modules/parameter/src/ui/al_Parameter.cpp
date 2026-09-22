#include "al/ui/al_Parameter.hpp"
#include <cstdint>

#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <vector>

#include "al/io/al_File.hpp"

using namespace al;

namespace {

/// Strip whitespace; reject empty / '.' / '..' segments.
std::string sanitizeSegment(const std::string &text) {
  static const std::regex re(R"(([^\s](?:[^\s./]*[^\s])?))");
  auto it = std::sregex_iterator(text.begin(), text.end(), re);
  if (it == std::sregex_iterator()) {
    return {};
  }
  return it->str();
}

void appendPathSegments(std::string &out, const std::string &pathLike) {
  std::stringstream ss(pathLike);
  std::string part;
  while (std::getline(ss, part, '/')) {
    std::string seg = sanitizeSegment(part);
    if (!seg.empty()) {
      out += "/" + seg;
    }
  }
}

} // namespace

// ParameterBool
// ------------------------------------------------------------------
ParameterBool::ParameterBool(std::string parameterName, std::string Group,
                             float defaultValue, float min, float max)
    : Parameter(parameterName, Group, defaultValue, min, max) {
  mValue = defaultValue;
  setDefault(defaultValue);
}

// --------------------- ParameterMeta ------------

void ParameterMeta::rebuildFullAddress() {
  std::string oldAddress = mFullAddress;
  mFullAddress.clear();
  // Group may be hierarchical for ECS: "scene/entityId" → /scene/entityId/...
  appendPathSegments(mFullAddress, mGroup);
  std::string nameSeg = sanitizeSegment(mParameterName);
  if (!nameSeg.empty()) {
    mFullAddress += "/" + nameSeg;
  } else {
    std::cout << "ParameterMeta ERROR: A valid name must be provided for the "
                 "parameter"
              << std::endl;
    mFullAddress += "/_";
  }
  if (!oldAddress.empty() && oldAddress != mFullAddress) {
    for (auto &cb : mPathChangeCallbacks) {
      cb(this, oldAddress, mFullAddress);
    }
  }
}

ParameterMeta::ParameterMeta(std::string parameterName, std::string group)
    : mParameterName(std::move(parameterName)), mGroup(std::move(group)) {
  rebuildFullAddress();
  mDisplayName = mParameterName;
}

void ParameterMeta::setName(std::string parameterName) {
  mParameterName = std::move(parameterName);
  rebuildFullAddress();
}

void ParameterMeta::setGroup(std::string group) {
  mGroup = std::move(group);
  rebuildFullAddress();
}

void ParameterMeta::setPath(std::string parameterName, std::string group) {
  mParameterName = std::move(parameterName);
  mGroup = std::move(group);
  rebuildFullAddress();
}

void ParameterMeta::registerPathChangeCallback(PathChangeCallback cb) {
  mPathChangeCallbacks.push_back(std::move(cb));
}

void ParameterMeta::set(ParameterMeta *p) {
  std::vector<VariantValue> fields;
  p->getFields(fields);
  setFields(fields);
}
