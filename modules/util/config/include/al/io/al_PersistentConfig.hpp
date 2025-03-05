#ifndef INCLUDE_PERSISTENTCONFIG_HPP
#define INCLUDE_PERSISTENTCONFIG_HPP

#include <cinttypes>
#include <iostream>
#include <string>
#include <vector>

#include "al/io/al_File.hpp"
#include "al/io/al_Toml.hpp"
#include "al/ui/al_Parameter.hpp"

namespace al {

/**
 * @brief The PersistentConfig class allows easy sotrage of persisitent values
 *
 * You must register the variables or parameters you want to store and recall
 * with the PersistentConfig object. You can then call read() and write() to
 * recall or store values for these variables and parameters.
 */
class PersistentConfig {
public:
  /**
   * @brief Set name of app. This determines the file name for the config file.
   * @param appName
   */
  void setAppName(std::string appName);

  /**
   * @brief Register a double variable for persistent storage
   */
  void registerDouble(std::string name, double *value);

  /**
   * @brief Register an int64_t variable for persistent storage
   */
  void registerInt(std::string name, int64_t *value);

  /**
   * @brief Register a string variable for persistent storage
   */
  void registerString(std::string name, std::string *value);

  // TODO Complete support for all parameter types, perhaps with a better
  // mechanism using templates
  void registerParameter(Parameter &param);
  void registerParameter(ParameterInt &param);
  void registerParameter(ParameterString &param);
  void registerParameter(ParameterVec3 &param);
  void registerParameter(ParameterVec4 &param);
  void registerParameter(ParameterColor &param);
  void registerParameter(ParameterPose &param);

  /**
   * @brief read
   * @return
   *
   * This function sets Parameter values using setNoCalls(), so any callbacks
   * registered are not called. You must manually call processChange() for the
   * parameter to force calling the callbacks.
   */
  bool read();

  void write();

private:
  std::string mAppName{"app"};
  std::vector<std::pair<std::string, double *>> mDoubles;
  std::vector<std::pair<std::string, int64_t *>> mInts;
  std::vector<std::pair<std::string, std::string *>> mStrings;

  std::vector<Parameter *> mParameters;
  std::vector<ParameterInt *> mParameterInts;
  std::vector<ParameterString *> mParameterStrings;
  std::vector<ParameterVec3 *> mParameterVec3s;
  std::vector<ParameterVec4 *> mParameterVec4s;
  std::vector<ParameterColor *> mParameterColors;
  std::vector<ParameterPose *> mParameterPoses;
};

} // namespace al

#endif // INCLUDE_PERSISTENTCONFIG_HPP
