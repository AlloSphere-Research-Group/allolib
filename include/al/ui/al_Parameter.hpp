#ifndef AL_PARAMETER_H
#define AL_PARAMETER_H

/*	Allocore --
        Multimedia / virtual environment application class library

        Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology,
   UCSB. Copyright (C) 2012-2015. The Regents of the University of California.
        All rights reserved.

        Redistribution and use in source and binary forms, with or without
        modification, are permitted provided that the following conditions are
   met:

                Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

                Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following disclaimer in the
                documentation and/or other materials provided with the
   distribution.

                Neither the name of the University of California nor the names
   of its contributors may be used to endorse or promote products derived from
                this software without specific prior written permission.

        THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
        IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

        File description:
        Parameter class that encapsulates communication of float data in a
   thread safe way to a separate thread (e.g. audio thread) File author(s):
        Andrés Cabrera mantaraya36@gmail.com
*/

#include <float.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

#include "al/math/al_Vec.hpp"
#include "al/protocol/al_OSC.hpp"
#include "al/spatial/al_Pose.hpp"
#include "al/types/al_Color.hpp"
#include "al/types/al_ValueSource.hpp"

namespace al {

enum class TimeMasterMode {
  TIME_MASTER_AUDIO,
  TIME_MASTER_GRAPHICS,
  TIME_MASTER_UPDATE,
  TIME_MASTER_FREE,
  TIME_MASTER_CPU
};

// ParameterField
// @ingroup UI
class ParameterField {
public:
  typedef enum { FLOAT, INT32, STRING, NULLDATA } ParameterDataType;

  ParameterField() {
    mData = nullptr;
    mType = NULLDATA;
  }

  ParameterField(const float value) {
    mType = FLOAT;
    mData = new float;
    *static_cast<float *>(mData) = value;
  }

  ParameterField(const double value) {
    mType = FLOAT;
    mData = new float;
    *static_cast<float *>(mData) = float(value);
  }

  ParameterField(const int32_t value) {
    mType = INT32;
    mData = new int32_t;
    *static_cast<int32_t *>(mData) = value;
  }

  ParameterField(const std::string value) {
    mType = STRING;
    mData = new std::string;
    *static_cast<std::string *>(mData) = value;
  }

  ParameterField(const char *value) {
    mType = STRING;
    mData = new std::string;
    *static_cast<std::string *>(mData) = value;
  }

  virtual ~ParameterField() {
    switch (mType) {
    case FLOAT:
      delete static_cast<float *>(mData);
      break;
    case STRING:
      delete static_cast<std::string *>(mData);
      break;
    case INT32:
      delete static_cast<int32_t *>(mData);
      break;
    case NULLDATA:
      break;
    }
  }

  // Copy constructor
  ParameterField(const ParameterField &paramField) : mType(paramField.mType) {
    switch (mType) {
    case FLOAT:
      mData = new float;
      *static_cast<float *>(mData) = *static_cast<float *>(paramField.mData);
      break;
    case STRING:
      mData = new std::string;
      *static_cast<std::string *>(mData) =
          *static_cast<std::string *>(paramField.mData);
      break;
    case INT32:
      mData = new int32_t;
      *static_cast<int32_t *>(mData) =
          *static_cast<int32_t *>(paramField.mData);
      break;
    case NULLDATA:
      break;
    }
  }

  // Move constructor
  ParameterField(ParameterField &&that) noexcept
      : mType(NULLDATA), mData(nullptr) {
    swap(*this, that);
  }

  // Copy assignment operator
  ParameterField &operator=(const ParameterField &other) {
    ParameterField copy(other);
    swap(*this, copy);
    return *this;
  }

  // Move assignment operator
  ParameterField &operator=(ParameterField &&that) {
    swap(*this, that);
    return *this;
  }

  friend void swap(ParameterField &lhs, ParameterField &rhs) noexcept {
    std::swap(lhs.mData, rhs.mData);
    std::swap(lhs.mType, rhs.mType);
  }

  ParameterDataType type() { return mType; }

  template <typename type> type get() { return *static_cast<type *>(mData); }

  template <typename type> void set(type value) {
    if (std::is_same<type, float>::value) {
      if (mType == FLOAT) {
        *static_cast<type *>(mData) = value;
      } else {
        std::cerr
            << "ERROR: Unexpected type for parameter field set(). Ignoring."
            << std::endl;
      }
    } else if (std::is_same<type, double>::value) {
      if (mType == FLOAT) {
        *static_cast<float *>(mData) = value;
      } else {
        std::cerr
            << "ERROR: Unexpected type for parameter field set(). Ignoring."
            << std::endl;
      }
    } else if (std::is_same<type, std::string>::value) {
      if (mType == STRING) {
        *static_cast<type *>(mData) = value;
      } else {
        std::cerr
            << "ERROR: Unexpected type for parameter field set(). Ignoring."
            << std::endl;
      }
    } else if (std::is_same<type, char *>::value) {
      if (mType == STRING) {
        *static_cast<std::string *>(mData) = value;
      } else {
        std::cerr
            << "ERROR: Unexpected type for parameter field set(). Ignoring."
            << std::endl;
      }
    } else if (std::is_same<type, int32_t>::value) {
      if (mType == INT32) {
        *static_cast<type *>(mData) = value;
      } else {
        std::cerr
            << "ERROR: Unexpected type for parameter field set(). Ignoring."
            << std::endl;
      }
    } else if (std::is_same<type, int>::value) {
      if (mType == INT32) {
        *static_cast<int32_t *>(mData) = value;
      } else {
        std::cerr
            << "ERROR: Unexpected type for parameter field set(). Ignoring."
            << std::endl;
      }
    }
  }

private:
  ParameterDataType mType;
  void *mData;
};

class Parameter;

/**
 * @brief The ParameterMeta class defines the base interface for Parameter
 * metadata
 * @ingroup UI
 */
class ParameterMeta {
public:
  /**
   * @brief ParameterMeta
   * @param parameterName
   * @param group
   * @param prefix
   */
  ParameterMeta(std::string parameterName, std::string group = "");

  virtual ~ParameterMeta() {}

  // Don't allow copy
  ParameterMeta(const ParameterMeta &) = delete;

  /**
   * @brief return the full OSC address for the parameter
   *
   * The parameter needs to be registered to a ParameterServer to listen to
   * OSC values on this address
   */
  std::string getFullAddress() { return mFullAddress; }

  /**
   * @brief getName returns the name of the parameter
   */
  std::string getName() { return mParameterName; }

  /**
   * @brief returns the text that should accompany parameters when displayed
   */
  std::string displayName() { return mDisplayName; }

  /**
   * @brief sets the text that should accompany parameters when displayed
   */
  void displayName(std::string displayName) { mDisplayName = displayName; }

  /**
   * @brief getGroup returns the name of the group for the parameter
   */
  std::string getGroup() { return mGroup; }

  /**
   * @brief Generic function to return the value of the parameter as a float.
   *
   * If not implemented, it will return 0.
   */
  virtual float toFloat() { return 0.f; }

  /**
   * @brief Generic function to set the parameter from a single float value
   *
   * Will only have effect on parameters that have a single internal value and
   * have implemented this function. Returns true if paramter is able to set
   * value from float
   */
  virtual bool fromFloat(float value) {
    (void)value;
    return false;
  }

  void setHint(std::string hintName, float hintValue) {
    mHints[hintName] = hintValue;
  }

  float getHint(std::string hintName, bool *exists = nullptr) {
    float value = 0.0f;
    if (mHints.find(hintName) != mHints.end()) {
      value = mHints[hintName];
      if (exists) {
        *exists = true;
      }
    } else {
      if (exists) {
        *exists = false;
      }
    }

    return value;
  }

  virtual void getFields(std::vector<ParameterField> & /*fields*/) {
    std::cout
        << "get(std::vector<ParameteterField> &fields) not implemented for "
        << typeid(*this).name() << std::endl;
  }

  virtual void setFields(std::vector<ParameterField> & /*fields*/) {
    std::cout
        << "set(std::vector<ParameteterField> &fields) not implemented for "
        << typeid(*this).name() << std::endl;
  }

  virtual void sendValue(osc::Send &sender, std::string prefix = "") {
    (void)prefix; // Remove compiler warning
    std::cout << "sendValue function not implemented for "
              << typeid(*this).name() << std::endl;
  }

  virtual void sendMeta(osc::Send &sender, std::string bundleName = "",
                        std::string id = "") {
    (void)bundleName; // Remove compiler warning
    std::cout << "sendMeta function not implemented for "
              << typeid(*this).name() << std::endl;
  }

  void set(ParameterMeta *p);

protected:
  std::string mFullAddress;
  std::string mParameterName;
  std::string mDisplayName;
  std::string mGroup;

  std::map<std::string, float> mHints; // Provide hints for behavior
};

/**
 * @brief The ParameterWrapper class provides a generic thread safe Parameter
 * class from the ParameterType template parameter
 * @ingroup UI
 */
template <class ParameterType> class ParameterWrapper : public ParameterMeta {
public:
  /**
   * @brief ParameterWrapper
   *
   * @param parameterName The name of the parameter
   * @param Group The group the parameter belongs to
   * @param defaultValue The initial value for the parameter
   * @param prefix An address prefix that is prepended to the parameter's OSC
   * address
   * @param min Minimum value for the parameter
   * @param max Maximum value for the parameter
   *
   * The mechanism used to protect data is locking a mutex within the set()
   * function and doing try_lock() on the mutex to update a cached value in the
   * get() function. In the worst case this might incur some jitter when reading
   * the value.
   */
  ParameterWrapper(std::string parameterName, std::string group = "",
                   ParameterType defaultValue = ParameterType());

  ParameterWrapper(std::string parameterName, std::string Group,
                   ParameterType defaultValue, ParameterType min,
                   ParameterType max);

  ParameterWrapper(const ParameterWrapper &param);

  virtual ~ParameterWrapper();

  /**
   * @brief set the parameter's value
   *
   * This function is thread-safe and can be called from any number of threads.
   * It blocks to lock a mutex so its use in critical contexts should be
   * avoided.
   */
  virtual void set(ParameterType value, ValueSource *src = nullptr) {
    //        if (value > mMax) value = mMax;
    //        if (value < mMin) value = mMin;
    mValueCache = get();
    if (mProcessCallback) {
      value = (*mProcessCallback)(value); //, mProcessUdata);
    }

    runChangeCallbacksSynchronous(value, src);
    setLocking(value);
  }

  /**
   * @brief reset value to default value
   */
  virtual void reset() { set(mDefault); }

  /**
   * @brief set the parameter's value without calling callbacks
   *
   * This function is thread-safe and can be called from any number of threads.
   * It blocks to lock a mutex so its use in critical contexts should be
   * avoided. The processing callback is called, but the callbacks registered
   * with registerChangeCallback() are not called. This is useful to avoid
   * infinite recursion when a widget sets the parameter that then sets the
   * widget.
   */

  virtual void setNoCalls(ParameterType value, void *blockReceiver = nullptr) {
    //        if (value > mMax) value = mMax;
    //        if (value < mMin) value = mMin;
    mValueCache = get();
    if (mProcessCallback) {
      value = (*mProcessCallback)(value); //, mProcessUdata);
    }
    if (blockReceiver) {
      for (auto cb : mCallbacks) {
        (*cb)(value);
      }
    }
    setLocking(value);
  }

  /**
   * @brief set the parameter's value forcing a lock
   *
   * No callbacks are called.
   */
  inline void setLocking(ParameterType value) {
    mMutex->lock();
    mValue = value;
    mMutex->unlock();
  }

  /**
   * @brief get the parameter's value
   *
   * This function is thread-safe and can be called from any number of threads
   *
   * @return the parameter value
   */
  virtual ParameterType get();

  /**
   * @brief Get previous value
   * @return
   *
   * This function is only useful when queried from a value callback. It will
   * represent the previous value of the parameter in that case.
   */
  virtual ParameterType getPrevious();

  /**
   * @brief set the minimum value for the parameter
   *
   * The value returned by the get() function will be clamped and will not go
   * under the value set by this function.
   */
  void min(ParameterType minValue, ValueSource *src = nullptr) {
    mMin = minValue;
    for (auto cb : mMetaCallbacksSrc) {
      (*cb)(src);
    }
  }
  ParameterType min() const { return mMin; }

  /**
   * @brief set the maximum value for the parameter
   *
   * The value returned by the get() function will be clamped and will not go
   * over the value set by this function.
   */
  void max(ParameterType maxValue, ValueSource *src = nullptr) {
    mMax = maxValue;
    for (auto cb : mMetaCallbacksSrc) {
      (*cb)(src);
    }
  }
  ParameterType max() const { return mMax; }

  void setDefault(const ParameterType &defaultValue) {
    mDefault = defaultValue;
  }
  ParameterType getDefault() const { return mDefault; }

  // typedef ParameterType (*ParameterProcessCallback)(ParameterType value, void
  // *userData); typedef void (*ParameterChangeCallback)(ParameterType value,
  // void *sender, void *userData, void * blockSender);

  typedef const std::function<ParameterType(ParameterType)>
      ParameterProcessCallback;
  typedef const std::function<void(ParameterType)> ParameterChangeCallback;
  typedef const std::function<void(ParameterType, ValueSource *)>
      ParameterChangeCallbackSrc;

  typedef const std::function<void(ValueSource *)>
      ParameterMetaChangeCallbackSrc;

  /**
   * @brief setProcessingCallback sets a callback to be called whenever the
   * parameter value changes
   *
   * Setting a callback can be useful when specific actions need to be taken
   * whenever a parameter changes, but it can also be used to modify the value
   * of the incoming parameter value before it is stored in the parameter.
   * The registered callback must return the value to be stored in the
   * parameter.
   * Only one callback may be registered here.
   *
   * @param cb The callback function
   *
   * @return the transformed value
   */
  void setProcessingCallback(ParameterProcessCallback cb);

  /**
   * @brief registerChangeCallback adds a callback to be called when the value
   * changes
   *
   * This function appends the callback to a list of callbacks to be called
   * whenever a value changes.
   *
   * @param cb
   */
  void registerChangeCallback(ParameterChangeCallback cb);

  void registerChangeCallback(ParameterChangeCallbackSrc cb);

  void registerMetaChangeCallback(ParameterMetaChangeCallbackSrc cb);

  /**
   * @brief Determines whether value change callbacks are called synchronously
   * @param synchronous
   *
   * If set to true, parameter change callbacks are called directly from the
   * setter function, i.e. as soon as the parameter value changes. This behavior
   * might be problematic in some cases, for example when an OSC message
   * triggers a change in the opengl state. This will cause a crash as the
   * opengl functions need to be called from the opengl context instead of from
   * a thread in the network context. By setting this to false and then calling
   * runChangeCallbacks within the opengl thread will call the callbacks
   * whenever the value has changed, but at the right time, in the right
   * context.
   */
  void setSynchronousCallbacks(bool synchronous = true) {
    if (mCallbacks.size() > 0 && mCallbacks[0] == nullptr) {
      if (synchronous) {
        mCallbacks.erase(mCallbacks.begin());
      }
    }
    if (!synchronous) {
      mCallbacks.insert(mCallbacks.begin(), nullptr);
    }
  }

  bool hasChange() { return mChanged; }

  /**
   * @brief call change callbacks if value has changed since last call
   */
  bool processChange() {
    if (!mChanged) {
      return false;
    }
    if (mChanged && mCallbacks.size() > 0 && mCallbacks[0] == nullptr) {
      auto callbackIt = mCallbacks.begin() + 1;
      ParameterType value = get();
      mChanged = false;
      while (callbackIt != mCallbacks.end()) {
        (*(*callbackIt))(value);
        callbackIt++;
      }
    }
    return true;
  }

  std::vector<ParameterWrapper<ParameterType> *>
  operator<<(ParameterWrapper<ParameterType> &newParam) {
    std::vector<ParameterWrapper<ParameterType> *> paramList;
    paramList.push_back(&newParam);
    return paramList;
  }

  std::vector<ParameterWrapper<ParameterType> *> &
  operator<<(std::vector<ParameterWrapper<ParameterType> *> &paramVector) {
    paramVector.push_back(this);
    return paramVector;
  }

  // Allow automatic conversion to the data type
  // this allows e.g. float value = parameter;
  operator ParameterType() { return this->get(); }

  ParameterWrapper<ParameterType> operator=(const ParameterType value) {
    this->set(value);
    return *this;
  }

protected:
  ParameterType mMin;
  ParameterType mMax;

  ParameterType mValue;
  ParameterType mValueCache;

  ParameterType mDefault;

  void runChangeCallbacksSynchronous(ParameterType &value, ValueSource *src);

  std::shared_ptr<ParameterProcessCallback> mProcessCallback;
  // void * mProcessUdata;
  // std::vector<void *> mCallbackUdata;

private:
  // pointer to avoid having to explicitly declare copy/move
  std::unique_ptr<std::mutex> mMutex;

  bool mChanged{false};

private:
  std::vector<std::shared_ptr<ParameterChangeCallback>> mCallbacks;
  std::vector<std::shared_ptr<ParameterChangeCallbackSrc>> mCallbacksSrc;

  std::vector<std::shared_ptr<ParameterMetaChangeCallbackSrc>>
      mMetaCallbacksSrc;
};

/**
 * @brief The Parameter class
 * @ingroup UI
 *
 * The Parameter class offers a simple way to encapsulate float values. It is
 * not inherently thread safe, but since floats are atomic on most platforms
 * it is safe in practice.
 *
 * Parameters are created with:
 * @code
        Parameter freq("Frequency", "Group", default, "/path/prefix");
 * @endcode
 *
 * Then values can be set from a low priority thread:
 * @code
        // In a simulator thread
        freq.set(var);
 * @endcode
 *
 * And read back from a high priority thread:
 * @code
        // In the audio thread
        float curFreq = freq.get()
 * @endcode
 *
 * The values are clamped between a minimum and maximum set using the min() and
 * max() functions.
 *
 * The ParameterServer class allows exposing Parameter objects via OSC.
 *
 */

class Parameter : public ParameterWrapper<float> {
public:
  /**
   * @brief Parameter
   *
   * @param parameterName The name of the parameter
   * @param Group The group the parameter belongs to
   * @param defaultValue The initial value for the parameter
   * @param min Minimum value for the parameter
   * @param max Maximum value for the parameter
   *
   * This Parameter class is designed for parameters that can be expressed as a
   * single float. It realies on float being atomic on the platform so there
   * is no locking. This is a safe assumption for most platforms today.
   */
  Parameter(std::string parameterName, std::string group = "",
            float defaultValue = 0, float min = -99999.0, float max = 99999.0);

  Parameter(std::string parameterName, float defaultValue, float min = -99999.0,
            float max = 99999.0);

  [[deprecated("Prefix is ignored")]] Parameter(
      std::string parameterName, std::string Group, float defaultValue,
      std::string prefix, float min = -99999.0, float max = 99999.0);

  Parameter(const al::Parameter &param) : ParameterWrapper<float>(param) {
    mValue = param.mValue;
    setDefault(param.getDefault());
  }

  /**
   * @brief set the parameter's value
   *
   * This function is thread-safe and can be called from any number of threads
   * It does not block and relies on the atomicity of float.
   */
  virtual void set(float value, ValueSource *src = nullptr) override;

  /**
   * @brief set the parameter's value without calling callbacks
   *
   * This function is thread-safe and can be called from any number of threads.
   * The processing callback is called, but the callbacks registered with
   * registerChangeCallback() are not called. This is useful to avoid infinite
   * recursion when a widget sets the parameter that then sets the widget.
   */
  virtual void setNoCalls(float value, void *blockReceiver = nullptr) override;

  /**
   * @brief get the parameter's value
   *
   * This function is thread-safe and can be called from any number of threads
   *
   * @return the parameter value
   */
  virtual float get() override;

  virtual float toFloat() override { return mValue; }

  virtual bool fromFloat(float value) override {
    set(value);
    return true;
  }

  float operator=(const float value) {
    this->set(value);
    return value;
  }

  virtual void getFields(std::vector<ParameterField> &fields) override {
    fields.emplace_back(ParameterField(get()));
  }

  virtual void setFields(std::vector<ParameterField> &fields) override {
    assert(fields.size() == 1);
    if (fields.size() == 1) {
      assert(fields[0].type() == ParameterField::FLOAT);
      set(fields[0].get<float>());
    } else {
      std::cout << "Wrong number of parameters for " << getFullAddress()
                << std::endl;
    }
  }

  virtual void sendValue(osc::Send &sender, std::string prefix = "") override {
    sender.send(prefix + getFullAddress(), get());
  }

  virtual void sendMeta(osc::Send &sender, std::string bundleName = "",
                        std::string id = "") override {
    if (bundleName.size() == 0) {
      sender.send("/registerParameter", getName(), getGroup(), getDefault(),
                  std::string(), min(), max());
    } else {
      sender.send("/registerBundleParameter", bundleName, id, getName(),
                  getGroup(), getDefault(), std::string(), min(), max());
    }
  }

private:
};

/// ParamaterInt
/// @ingroup UI
class ParameterInt : public ParameterWrapper<int32_t> {
public:
  /**
   * @brief ParameterInt
   *
   * @param parameterName The name of the parameter
   * @param Group The group the parameter belongs to
   * @param defaultValue The initial value for the parameter
   * @param min Minimum value for the parameter
   * @param max Maximum value for the parameter
   *
   * This Parameter class is designed for parameters that can be expressed as a
   * single 32 bit integer number. It realies on float being atomic on the
   * platform so there is no locking. This is a safe assumption for most
   * desktop platforms today.
   */
  ParameterInt(std::string parameterName, std::string Group = "",
               int32_t defaultValue = 0, int32_t min = 0, int32_t max = 127);

  [[deprecated("Prefix is ignored")]] ParameterInt(
      std::string parameterName, std::string Group, int32_t defaultValue,
      std::string prefix, int32_t min = 0, int32_t max = 127);

  ParameterInt(const al::ParameterInt &param)
      : ParameterWrapper<int32_t>(param) {
    mValue = param.mValue;
    setDefault(param.getDefault());
  }

  /**
   * @brief set the parameter's value
   *
   * This function is thread-safe and can be called from any number of threads
   * It does not block and relies on the atomicity of float.
   */
  virtual void set(int32_t value, ValueSource *src = nullptr) override;

  /**
   * @brief set the parameter's value without calling callbacks
   *
   * This function is thread-safe and can be called from any number of threads.
   * The processing callback is called, but the callbacks registered with
   * registerChangeCallback() are not called. This is useful to avoid infinite
   * recursion when a widget sets the parameter that then sets the widget.
   */
  virtual void setNoCalls(int32_t value,
                          void *blockReceiver = nullptr) override;

  //  /**
  //   * @brief get the parameter's value
  //   *
  //   * This function is thread-safe and can be called from any number of
  //   threads
  //   *
  //   * @return the parameter value
  //   */
  //  virtual int32_t get() override;

  virtual float toFloat() override { return float(mValue); }

  virtual bool fromFloat(float value) override {
    set(int32_t(value));
    return true;
  }

  float operator=(const int32_t value) {
    this->set(value);
    return float(value);
  }

  virtual void sendValue(osc::Send &sender, std::string prefix = "") override {
    sender.send(prefix + getFullAddress(), get());
  }

  virtual void getFields(std::vector<ParameterField> &fields) override {
    fields.emplace_back(ParameterField(get()));
  }

  virtual void setFields(std::vector<ParameterField> &fields) override {
    if (fields.size() == 1) {
      assert(fields[0].type() == ParameterField::INT32);
      set(fields[0].get<int32_t>());
    } else {
      std::cout << "Wrong number of parameters for " << getFullAddress()
                << std::endl;
    }
  }

  virtual void sendMeta(osc::Send &sender, std::string bundleName = "",
                        std::string id = "") override {
    if (bundleName.size() == 0) {
      sender.send("/registerParameter", getName(), getGroup(), getDefault(),
                  std::string(), min(), max());
    } else {
      sender.send("/registerBundleParameter", bundleName, id, getName(),
                  getGroup(), getDefault(), std::string(), min(), max());
    }
  }

private:
};

/// ParamaterBool
/// @ingroup UI
class ParameterBool : public Parameter {
public:
  using Parameter::get;
  using Parameter::setFields;
  /**
   * @brief ParameterBool
   *
   * @param parameterName The name of the parameter
   * @param Group The group the parameter belongs to
   * @param defaultValue The initial value for the parameter
   * @param prefix An address prefix that is prepended to the parameter's OSC
   * address
   * @param min Value when off/false
   * @param max Value when on/true
   *
   * This ParameterBool class is designed for boolean parameters that have
   * float values for on or off states. It relies on floats being atomic on
   * the platform.
   */
  ParameterBool(std::string parameterName, std::string Group = "",
                float defaultValue = 0, float min = 0, float max = 1.0);

  [[deprecated("Prefix is ignored")]] ParameterBool(
      std::string parameterName, std::string Group, float defaultValue,
      std::string prefix, float min = 0, float max = 1.0);

  bool operator=(bool value) {
    this->set(value ? 1.0f : 0.0f);
    return value == 1.0;
  }

  virtual float toFloat() override { return get(); }

  virtual bool fromFloat(float value) override {
    set(value);
    return true;
  }

  virtual void getFields(std::vector<ParameterField> &fields) override {
    fields.emplace_back(ParameterField(get() == 1.0f ? 1 : 0));
  }

  virtual void setFields(std::vector<ParameterField> &fields) override {
    if (fields.size() == 1) {
      if (fields[0].type() == ParameterField::INT32) {
        set(fields[0].get<int32_t>() == 1 ? 1.0f : 0.0f);
      } else if (fields[0].type() == ParameterField::FLOAT) {
        set(fields[0].get<float>());
      }
    } else {
      std::cout << "Wrong number of parameters for " << getFullAddress()
                << std::endl;
    }
  }

  virtual void sendMeta(osc::Send &sender, std::string bundleName = "",
                        std::string id = "") override {

    if (bundleName.size() == 0) {
      sender.send("/registerParameter", getName(), getGroup(), getDefault(),
                  std::string(), min(), max());
    } else {
      sender.send("/registerBundleParameter", bundleName, id, getName(),
                  getGroup(), getDefault(), std::string(), min(), max());
    }
  }
};

// Symbolizes a distributed action that
// has no value per se, but that can be used to trigger actions
class Trigger : public ParameterWrapper<bool> {
public:
  Trigger(std::string parameterName, std::string Group = "")
      : ParameterWrapper<bool>(parameterName, Group, false) {
    mValue = false;
    mValueCache = false;
  }

  virtual float toFloat() override { return get() ? 1.0f : 0.0f; }

  virtual bool fromFloat(float value) override {
    set(value != 0.0f);
    return true;
  }

  virtual void sendValue(osc::Send &sender, std::string prefix = "") override {
    sender.send(prefix + getFullAddress());
  }

  void trigger() { set(true); }
};

// These three types are blocking, should not be used in time-critical
// contexts like the audio callback. The classes were explicitly defined to
// overcome the issues related to the > and < operators needed when validating
// minumum and maximum values for the parameter

/// ParameterString
/// @ingroup UI
class ParameterString : public ParameterWrapper<std::string> {
public:
  using ParameterWrapper<std::string>::get;
  using ParameterWrapper<std::string>::set;

  ParameterString(std::string parameterName, std::string Group = "",
                  std::string defaultValue = "")
      : ParameterWrapper<std::string>(parameterName, Group, defaultValue) {}

  [[deprecated("Prefix is ignored")]] ParameterString(std::string parameterName,
                                                      std::string Group,
                                                      std::string defaultValue,
                                                      std::string /*prefix*/)
      : ParameterWrapper<std::string>(parameterName, Group, defaultValue) {}

  virtual float toFloat() override {
    float value = 0.0;
    try {
      value = std::stof(get());
    } catch (...) {
      // ignore
    }
    return value;
  }

  virtual bool fromFloat(float value) override {
    set(std::to_string(value));
    return true;
  }

  virtual void sendValue(osc::Send &sender, std::string prefix = "") override {
    sender.send(prefix + getFullAddress(), get());
  }

  virtual void getFields(std::vector<ParameterField> &fields) override {
    fields.emplace_back(ParameterField(get()));
  }

  virtual void setFields(std::vector<ParameterField> &fields) override {
    if (fields.size() == 1) {
      set(fields[0].get<std::string>());
    } else {
      std::cout << "Wrong number of parameters for " << getFullAddress()
                << std::endl;
    }
  }

  virtual void sendMeta(osc::Send &sender, std::string bundleName = "",
                        std::string id = "") override {
    if (bundleName.size() == 0) {
      sender.send("/registerParameter", getName(), getGroup(), getDefault(),
                  std::string(), min(), max());
    } else {
      sender.send("/registerBundleParameter", bundleName, id, getName(),
                  getGroup(), getDefault(), std::string(), min(), max());
    }
  }
};

class ParameterVec3 : public ParameterWrapper<al::Vec3f> {
public:
  using ParameterWrapper<al::Vec3f>::get;
  using ParameterWrapper<al::Vec3f>::set;

  ParameterVec3(std::string parameterName, std::string Group = "",
                al::Vec3f defaultValue = al::Vec3f())
      : ParameterWrapper<al::Vec3f>(parameterName, Group, defaultValue) {}

  ParameterVec3 operator=(const Vec3f vec) {
    this->set(vec);
    return *this;
  }

  float operator[](size_t index) {
    assert(index < INT_MAX); // Hack to remove
    Vec3f vec = this->get();
    return vec[index];
  }

  virtual void sendValue(osc::Send &sender, std::string prefix = "") override {
    Vec3f vec = get();
    sender.send(prefix + getFullAddress(), vec.x, vec.y, vec.z);
  }

  virtual void getFields(std::vector<ParameterField> &fields) override {
    Vec3f vec = this->get();
    fields.emplace_back(ParameterField(vec.x));
    fields.emplace_back(ParameterField(vec.y));
    fields.emplace_back(ParameterField(vec.z));
  }

  virtual void setFields(std::vector<ParameterField> &fields) override {
    if (fields.size() == 3) {
      Vec3f vec(fields[0].get<float>(), fields[1].get<float>(),
                fields[2].get<float>());
      set(vec);
    } else {
      std::cout << "Wrong number of parameters for " << getFullAddress()
                << std::endl;
    }
  }
};

/// ParameterVec4
/// @ingroup UI
class ParameterVec4 : public ParameterWrapper<al::Vec4f> {
public:
  using ParameterWrapper<al::Vec4f>::get;
  using ParameterWrapper<al::Vec4f>::set;

  ParameterVec4(std::string parameterName, std::string Group = "",
                al::Vec4f defaultValue = al::Vec4f())
      : ParameterWrapper<al::Vec4f>(parameterName, Group, defaultValue) {}

  ParameterVec4 operator=(const Vec4f vec) {
    this->set(vec);
    return *this;
  }

  float operator[](size_t index) {
    Vec4f vec = this->get();
    return vec[index];
  }

  virtual void sendValue(osc::Send &sender, std::string prefix = "") override {
    Vec4f vec = get();
    sender.send(prefix + getFullAddress(), vec.x, vec.y, vec.z, vec.w);
  }

  virtual void getFields(std::vector<ParameterField> &fields) override {
    Vec4f vec = this->get();
    fields.emplace_back(ParameterField(vec.x));
    fields.emplace_back(ParameterField(vec.y));
    fields.emplace_back(ParameterField(vec.z));
    fields.emplace_back(ParameterField(vec.w));
  }

  virtual void setFields(std::vector<ParameterField> &fields) override {
    if (fields.size() == 4) {
      Vec4f vec(fields[0].get<float>(), fields[1].get<float>(),
                fields[2].get<float>(), fields[3].get<float>());
      set(vec);
    } else {
      std::cout << "Wrong number of parameters for " << getFullAddress()
                << std::endl;
    }
  }
};

/// ParameterPose
/// @ingroup UI
class ParameterPose : public ParameterWrapper<al::Pose> {
public:
  using ParameterWrapper<al::Pose>::get;
  using ParameterWrapper<al::Pose>::set;

  ParameterPose(std::string parameterName, std::string Group = "",
                al::Pose defaultValue = al::Pose())
      : ParameterWrapper<al::Pose>(parameterName, Group, defaultValue) {}

  al::Pose operator=(const al::Pose vec) {
    this->set(vec);
    return *this;
  }

  virtual void sendValue(osc::Send &sender, std::string prefix = "") override {
    Pose pose = get();
    Quatd q = pose.quat();
    sender.send(prefix + getFullAddress(), float(pose.x()), float(pose.y()),
                float(pose.z()), float(q.w), float(q.x), float(q.y),
                float(q.z));
  }

  void setPos(const al::Vec3d v) { this->set(al::Pose(v, this->get().quat())); }
  void setQuat(const al::Quatd q) { this->set(al::Pose(this->get().pos(), q)); }
  //    float operator[](size_t index) { Pose vec = this->get(); return
  //    vec[index];}

  virtual void getFields(std::vector<ParameterField> &fields) override {
    Quatf quat = get().quat();
    Vec4f pos = get().pos();
    fields.reserve(7);
    fields.emplace_back(ParameterField(pos.x));
    fields.emplace_back(ParameterField(pos.y));
    fields.emplace_back(ParameterField(pos.z));
    fields.emplace_back(ParameterField(quat.w));
    fields.emplace_back(ParameterField(quat.x));
    fields.emplace_back(ParameterField(quat.y));
    fields.emplace_back(ParameterField(quat.z));
  }

  virtual void setFields(std::vector<ParameterField> &fields) override {
    if (fields.size() == 7) {
      Pose vec(Vec3f(fields[0].get<float>(), fields[1].get<float>(),
                     fields[2].get<float>()),
               Quatf(fields[3].get<float>(), fields[4].get<float>(),
                     fields[5].get<float>(), fields[6].get<float>()));
      set(vec);
    } else {
      std::cout << "Wrong number of parameters for " << getFullAddress()
                << std::endl;
    }
  }
};

/// ParameterMenu
/// @ingroup UI
class ParameterMenu : public ParameterWrapper<int32_t> {
public:
  using ParameterWrapper<int32_t>::set;
  using ParameterWrapper<int32_t>::get;

  ParameterMenu(std::string parameterName, std::string Group = "",
                int defaultValue = 0)
      : ParameterWrapper<int>(parameterName, Group, defaultValue) {}

  int operator=(const int32_t value) {
    this->set(value);
    return *this;
  }

  void setElements(std::vector<std::string> elements) {
    std::lock_guard<std::mutex> lk(mElementsLock);
    mElements = elements;
  }

  std::vector<std::string> getElements() {
    std::lock_guard<std::mutex> lk(mElementsLock);
    return mElements;
  }

  std::string getCurrent() {
    int current = get();
    std::lock_guard<std::mutex> lk(mElementsLock);
    if (mElements.size() > 0 && current >= 0 &&
        current < int32_t(mElements.size())) {
      return mElements[current];
    } else {
      return "";
    }
  }

  void setCurrent(std::string element, bool noCalls = false) {
    mElementsLock.lock();
    auto position = std::find(mElements.begin(), mElements.end(), element);
    bool found = position != mElements.end();
    int foundPosition = (int32_t)std::distance(mElements.begin(), position);
    mElementsLock.unlock();
    if (found) {
      if (noCalls) {
        setNoCalls(foundPosition);
      } else {
        set(foundPosition);
      }
    } else {
      std::cerr << "ERROR: Could not find element: " << element << std::endl;
    }
  }

  virtual float toFloat() override {
    return float(get());
    // return std::stof(getCurrent());
  }

  virtual bool fromFloat(float value) override {
    set((int32_t)value);
    return true;
  }

  virtual void sendValue(osc::Send &sender, std::string prefix = "") override {
    sender.send(prefix + getFullAddress(), get());
  }

  virtual void getFields(std::vector<ParameterField> &fields) override {
    fields.emplace_back(ParameterField(getCurrent()));
  }

  virtual void setFields(std::vector<ParameterField> &fields) override {
    // TODO an option should be added to allow storing the current element as
    // index instead of text.
    if (fields.size() == 1) {
      setCurrent(fields[0].get<std::string>());
    } else {
      std::cout << "Wrong number of parameters for " << getFullAddress()
                << std::endl;
    }
  }

private:
  std::mutex mElementsLock;
  std::vector<std::string> mElements;
};

/**
 * @brief A parameter representing selected items from a list
 * @ingroup UI
 *
 * The unsigned int value is a bit field, each bit representing
 * whether an element is selected or not.
 *
 */
class ParameterChoice : public ParameterWrapper<uint64_t> {
public:
  using ParameterWrapper<uint64_t>::get;
  using ParameterWrapper<uint64_t>::set;

  ParameterChoice(std::string parameterName, std::string Group = "",
                  uint64_t defaultValue = 0)
      : ParameterWrapper<uint64_t>(parameterName, Group, defaultValue) {}

  ParameterChoice &operator=(const uint64_t value) {
    this->set(value);
    return *this;
  }

  void setElements(std::vector<std::string> &elements, bool allOn = false) {
    mElements = elements;
    min(0);
    assert(((uint64_t)1 << (elements.size() - 1)) < UINT64_MAX);
    max((uint64_t)1 << (elements.size() - 1));
    if (allOn) {
      uint16_t value = 0;
      for (unsigned int i = 0; i < elements.size(); i++) {
        value |= 1 << i;
      }
      set(value);
    }
  }

  void setElementSelected(std::string name, bool selected = true) {
    for (size_t i = 0; i < mElements.size(); i++) {
      if (mElements[i] == name) {
        uint64_t value = get();
        if (selected) {
          value |= UINT64_C(1) << i;
        } else {
          value ^= value | UINT64_C(1) << i;
        }
        set(value);
      }
    }
  }

  std::vector<std::string> getElements() { return mElements; }

  std::vector<std::string> getSelectedElements() {
    std::vector<std::string> selected;
    for (uint64_t i = 0; i < mElements.size(); i++) {
      if (get() & ((uint64_t)1 << i)) {
        if (mElements.size() > i) {
          selected.push_back(mElements[i]);
        }
      }
    }
    return selected;
  }

  void set(std::vector<int8_t> on) {
    uint64_t value = 0;
    for (auto onBit : on) {
      if (onBit < 64) {
        value |= UINT64_C(1) << onBit;
      } else {
        std::cerr << __FILE__ << " " << __FUNCTION__
                  << " bit index too high. Ignoring" << std::endl;
      }
    }
    set(value);
  }

  virtual float toFloat() override {
    return (float)get();
    // return std::stof(getCurrent());
  }

  virtual bool fromFloat(float value) override {
    set((uint64_t)value);
    return true;
  }

  virtual void sendValue(osc::Send &sender, std::string prefix = "") override {
    sender.send(prefix + getFullAddress(), (int32_t)get());
  }

  virtual void getFields(std::vector<ParameterField> &fields) override {
    if (get() > INT32_MAX) {
      std::cerr << "WARNING: Can't fit choice value." << std::endl;
    }
    fields.emplace_back(ParameterField((int32_t)get()));
  }

  virtual void setFields(std::vector<ParameterField> &fields) override {
    // TODO an option should be added to allow storing the current element as
    // index instead of text.
    if (fields.size() == 1) {
      set(fields[0].get<int32_t>());
    } else {
      std::cout << "Wrong number of parameters for " << getFullAddress()
                << std::endl;
    }
  }

private:
  std::vector<std::string> mElements;
};

/// ParameterColor
/// @ingroup UI
class ParameterColor : public ParameterWrapper<al::Color> {
public:
  using ParameterWrapper<al::Color>::setFields;
  using ParameterWrapper<al::Color>::get;

  ParameterColor(std::string parameterName, std::string Group = "",
                 al::Color defaultValue = al::Color())
      : ParameterWrapper<al::Color>(parameterName, Group, defaultValue) {}

  ParameterColor operator=(const al::Color vec) {
    this->set(vec);
    return *this;
  }

  virtual void sendValue(osc::Send &sender, std::string prefix = "") override {
    Color c = get();
    sender.send(prefix + getFullAddress(), c.r, c.g, c.b, c.a);
  }

  virtual void getFields(std::vector<ParameterField> &fields) override {
    Color vec = this->get();
    fields.emplace_back(ParameterField(vec.r));
    fields.emplace_back(ParameterField(vec.g));
    fields.emplace_back(ParameterField(vec.b));
    fields.emplace_back(ParameterField(vec.a));
  }

  virtual void setFields(std::vector<ParameterField> &fields) override {
    if (fields.size() == 4) {
      Color vec(fields[0].get<float>(), fields[1].get<float>(),
                fields[2].get<float>(), fields[3].get<float>());
      set(vec);
    } else {
      std::cout << "Wrong number of parameters for " << getFullAddress()
                << std::endl;
    }
  }
};

// Implementations -----------------------------------------------------------

template <class ParameterType>
ParameterWrapper<ParameterType>::~ParameterWrapper() {
  //  delete mMutex;
}

template <class ParameterType>
ParameterWrapper<ParameterType>::ParameterWrapper(std::string parameterName,
                                                  std::string group,
                                                  ParameterType defaultValue)
    : ParameterMeta(parameterName, group), mProcessCallback(nullptr) {
  mValue = defaultValue;
  mValueCache = defaultValue;
  mMutex = std::make_unique<std::mutex>();
  setDefault(defaultValue);
  std::shared_ptr<ParameterChangeCallback> mAsyncCallback =
      std::make_shared<ParameterChangeCallback>(
          [&](ParameterType value) { mChanged = true; });
}

template <class ParameterType>
ParameterWrapper<ParameterType>::ParameterWrapper(std::string parameterName,
                                                  std::string group,
                                                  ParameterType defaultValue,
                                                  ParameterType min,
                                                  ParameterType max)
    : ParameterWrapper<ParameterType>::ParameterWrapper(parameterName, group,
                                                        defaultValue) {
  mMin = min;
  mMax = max;
  mMutex = std::make_unique<std::mutex>();
  setDefault(defaultValue);
}

template <class ParameterType>
ParameterWrapper<ParameterType>::ParameterWrapper(
    const ParameterWrapper<ParameterType> &param)
    : ParameterMeta(param.mParameterName, param.mGroup) {
  mMin = param.mMin;
  mMax = param.mMax;
  mProcessCallback = param.mProcessCallback;
  // mProcessUdata = param.mProcessUdata;
  mCallbacks = param.mCallbacks;
  mMutex = std::make_unique<std::mutex>();
  setDefault(param.getDefault());
  // mCallbackUdata = param.mCallbackUdata;
}

template <class ParameterType>
ParameterType ParameterWrapper<ParameterType>::get() {
  ParameterType current = mValueCache;
  if (mMutex->try_lock()) {
    current = mValue;
    mMutex->unlock();
  }
  return current;
}

template <class ParameterType>
ParameterType ParameterWrapper<ParameterType>::getPrevious() {
  return mValueCache;
}

template <class ParameterType>
void ParameterWrapper<ParameterType>::setProcessingCallback(
    typename ParameterWrapper::ParameterProcessCallback cb) {
  mProcessCallback = std::make_shared<ParameterProcessCallback>(cb);
  // mProcessUdata = userData;
}

template <class ParameterType>
void ParameterWrapper<ParameterType>::registerChangeCallback(
    ParameterChangeCallback cb) {
  mCallbacks.push_back(std::make_shared<ParameterChangeCallback>(cb));
  // mCallbackUdata.push_back(userData);
}

template <class ParameterType>
void ParameterWrapper<ParameterType>::registerChangeCallback(
    ParameterChangeCallbackSrc cb) {
  mCallbacksSrc.push_back(std::make_shared<ParameterChangeCallbackSrc>(cb));
  // mCallbackUdata.push_back(userData);
}

template <class ParameterType>
void ParameterWrapper<ParameterType>::registerMetaChangeCallback(
    ParameterMetaChangeCallbackSrc cb) {
  mMetaCallbacksSrc.push_back(
      std::make_shared<ParameterMetaChangeCallbackSrc>(cb));
}

template <class ParameterType>
void ParameterWrapper<ParameterType>::runChangeCallbacksSynchronous(
    ParameterType &value, ValueSource *src) {
  for (auto cb : mCallbacks) {
    if (cb == nullptr) {
      // If first callback if nullptr, callbacks must be processed async
      mChanged = true;
      return;
    } else {
      (*cb)(value);
    }
  }
  for (auto cb : mCallbacksSrc) {
    (*cb)(value, src);
  }
}

} // namespace al

#endif // AL_PARAMETER_H
