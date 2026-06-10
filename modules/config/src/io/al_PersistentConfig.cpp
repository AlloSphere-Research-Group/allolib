#include "al/io/al_PersistentConfig.hpp"

using namespace al;

void PersistentConfig::setAppName(std::string appName)
{
  mAppName = appName;
  //    std::cout << "Persistent config created for " << appName << std::endl;
}

void PersistentConfig::registerDouble(std::string name, double* value)
{
  mDoubles.push_back({name, value});
}

void PersistentConfig::registerInt(std::string name, int64_t* value)
{
  mInts.push_back({name, value});
}

void PersistentConfig::registerString(std::string name, std::string* value)
{
  mStrings.push_back({name, value});
}

void PersistentConfig::registerParameter(Parameter& param)
{
  auto fullAddress = param.getFullAddress();
  if (fullAddress.find('.') == std::string::npos) {
    mParameters.push_back(&param);
  }
  else {
    std::cerr << "ERROR: Parameters with '.' in name are not supported in "
                 "PersistentConfig. Not including."
              << fullAddress << std::endl;
    assert(0 == 1);
  }
}

void PersistentConfig::registerParameter(ParameterInt& param)
{
  auto fullAddress = param.getFullAddress();
  if (fullAddress.find('.') == std::string::npos) {
    mParameterInts.push_back(&param);
  }
  else {
    std::cerr << "ERROR: Parameters with '.' in name are not supported in "
                 "PersistentConfig. Not including."
              << fullAddress << std::endl;
    assert(0 == 1);
  }
}

void PersistentConfig::registerParameter(ParameterString& param)
{
  auto fullAddress = param.getFullAddress();
  if (fullAddress.find('.') == std::string::npos) {
    mParameterStrings.push_back(&param);
  }
  else {
    std::cerr << "ERROR: Parameters with '.' in name are not supported in "
                 "PersistentConfig. Not including."
              << fullAddress << std::endl;
    assert(0 == 1);
  }
}

void PersistentConfig::registerParameter(ParameterVec3& param)
{
  auto fullAddress = param.getFullAddress();
  if (fullAddress.find('.') == std::string::npos) {
    mParameterVec3s.push_back(&param);
  }
  else {
    std::cerr << "ERROR: Parameters with '.' in name are not supported in "
                 "PersistentConfig. Not including: "
              << fullAddress << std::endl;
    assert(0 == 1);
  }
}

void PersistentConfig::registerParameter(ParameterVec4& param)
{
  auto fullAddress = param.getFullAddress();
  if (fullAddress.find('.') == std::string::npos) {
    mParameterVec4s.push_back(&param);
  }
  else {
    std::cerr << "ERROR: Parameters with '.' in name are not supported in "
                 "PersistentConfig. Not including: "
              << fullAddress << std::endl;
    assert(0 == 1);
  }
}

void PersistentConfig::registerParameter(ParameterColor& param)
{
  auto fullAddress = param.getFullAddress();
  if (fullAddress.find('.') == std::string::npos) {
    mParameterColors.push_back(&param);
  }
  else {
    std::cerr << "ERROR: Parameters with '.' in name are not supported in "
                 "PersistentConfig. Not including: "
              << fullAddress << std::endl;
    assert(0 == 1);
  }
}

void PersistentConfig::registerParameter(ParameterPose& param)
{
  auto fullAddress = param.getFullAddress();
  if (fullAddress.find('.') == std::string::npos) {
    mParameterPoses.push_back(&param);
  }
  else {
    std::cerr << "ERROR: Parameters with '.' in name are not supported in "
                 "PersistentConfig. Not including: "
              << fullAddress << std::endl;
    assert(0 == 1);
  }
}

bool PersistentConfig::read()
{
  if (!File::exists(mAppName + ".toml")) {
    write();
  }
  TomlFile configFile{mAppName + ".toml"};
  for (auto& configEntry : mDoubles) {
    if (configFile.hasKey(configEntry.first)) {
      *configEntry.second = configFile.getd(configEntry.first);
    }
  }
  for (auto& configEntry : mInts) {
    if (configFile.hasKey(configEntry.first)) {
      *configEntry.second = configFile.geti(configEntry.first);
    }
  }
  for (auto& configEntry : mStrings) {
    if (configFile.hasKey(configEntry.first)) {
      *configEntry.second = configFile.gets(configEntry.first);
    }
  }

  for (auto param : mParameters) {
    std::string name;
    if (param->getGroup().size() > 0) {
      name = param->getGroup() + ".";
    }
    name += param->getName();
    if (configFile.hasKey(name)) {
      param->setNoCalls(configFile.getd(name));
    }
  }
  for (auto param : mParameterInts) {
    std::string name;
    if (param->getGroup().size() > 0) {
      name = param->getGroup() + ".";
    }
    name += param->getName();
    if (configFile.hasKey(name)) {
      param->setNoCalls(configFile.geti(name));
    }
  }
  for (auto param : mParameterStrings) {
    std::string name;
    if (param->getGroup().size() > 0) {
      name = param->getGroup() + ".";
    }
    name += param->getName();
    if (configFile.hasKey(name)) {
      param->setNoCalls(configFile.gets(name));
    }
  }
  for (auto param : mParameterColors) {
    std::string name;
    if (param->getGroup().size() > 0) {
      name = param->getGroup() + ".";
    }
    name += param->getName();
    if (configFile.hasKey(name)) {
      auto vals = configFile.getVector<double>(name);
      if (vals.size() != 4) {
        std::cerr << "ERROR: Color parameter " << name
                  << " has wrong number of components in config file. Expected "
                     "4, got "
                  << vals.size() << ". Ignoring." << std::endl;
        continue;
      }
      param->setNoCalls(Color(vals[0], vals[1], vals[2], vals[3]));
    }
  }
  for (auto param : mParameterPoses) {
    std::string name;
    if (param->getGroup().size() > 0) {
      name = param->getGroup() + ".";
    }
    name += param->getName();
    if (configFile.hasKey(name)) {
      auto vals = configFile.getVector<double>(name);
      if (vals.size() != 7) {
        std::cerr << "ERROR: Pose parameter " << name
                  << " has wrong number of components in config file. Expected "
                     "7, got "
                  << vals.size() << ". Ignoring." << std::endl;
        continue;
      }
      param->setNoCalls(Pose(Vec3d(vals[0], vals[1], vals[2]),
                             Quatd(vals[3], vals[4], vals[5], vals[6])));
    }
  }
  return true;
}

void PersistentConfig::write()
{
  TomlFile configFile{mAppName + ".toml"};
  for (auto& configEntry : mDoubles) {
    configFile.set(configEntry.first, *configEntry.second);
  }
  for (auto& configEntry : mInts) {
    configFile.set(configEntry.first, *configEntry.second);
  }
  for (auto& configEntry : mStrings) {
    configFile.set(configEntry.first, *configEntry.second);
  }

  for (auto param : mParameters) {
    std::string name;
    if (param->getGroup().size() > 0) {
      name = param->getGroup() + ".";
    }
    name += param->getName();
    configFile.set(name, param->get());
  }
  for (auto param : mParameterInts) {
    std::string name;
    if (param->getGroup().size() > 0) {
      name = param->getGroup() + ".";
    }
    name += param->getName();
    configFile.set(name, param->get());
  }
  for (auto param : mParameterStrings) {
    std::string name;
    if (param->getGroup().size() > 0) {
      name = param->getGroup() + ".";
    }
    name += param->getName();
    configFile.set(name, param->get());
  }
  for (auto param : mParameterColors) {
    std::string name;
    if (param->getGroup().size() > 0) {
      name = param->getGroup() + ".";
    }
    name += param->getName();
    std::vector<double> rgba;
    rgba.resize(4);
    Color bg = param->get();
    rgba[0] = bg.r;
    rgba[1] = bg.g;
    rgba[2] = bg.b;
    rgba[3] = bg.a;
    configFile.setVector<double>(name, rgba);
  }
  for (auto param : mParameterPoses) {
    std::string name;
    if (param->getGroup().size() > 0) {
      name = param->getGroup() + ".";
    }
    name += param->getName();
    std::vector<double> poseList;
    poseList.resize(7);
    Pose pose = param->get();
    poseList[0] = pose.x();
    poseList[1] = pose.y();
    poseList[2] = pose.z();
    poseList[3] = pose.quat().w;
    poseList[4] = pose.quat().x;
    poseList[5] = pose.quat().y;
    poseList[6] = pose.quat().z;
    configFile.setVector<double>(name, poseList);
  }

  configFile.write();
}
