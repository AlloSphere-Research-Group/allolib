#ifndef AL_PARAMETER_H
#define AL_PARAMETER_H

/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012-2015. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its
		contributors may be used to endorse or promote products derived from
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.

	File description:
	Parameter class that encapsulates communication of float data in a thread
	safe way to a separate thread (e.g. audio thread)
	File author(s):
	Andrés Cabrera mantaraya36@gmail.com
*/

#include <string>
#include <map>
#include <mutex>
#include <atomic>
#include <iostream>
#include <sstream>
#include <functional>
#include <float.h>
#include <cassert>
#include <vector>
#include <memory>

#include "al/core/math/al_Vec.hpp"
#include "al/core/spatial/al_Pose.hpp"
#include "al/core/types/al_Color.hpp"
#include "al/core/protocol/al_OSC.hpp"

namespace al
{

class Parameter;

/**
 * @brief The ParameterMeta class defines the base interface for Parameter metadata
 */
class ParameterMeta {
public:
    /**
     * @brief ParameterMeta
     * @param parameterName
     * @param group
     * @param prefix
     */
    ParameterMeta(std::string parameterName, std::string group = "",
        std::string prefix = "");

    virtual ~ParameterMeta() {}

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
	std::string getName(){ return mParameterName; }

    /**
    * @brief returns the text that should accompany parameters when displayed
    */
    std::string displayName(){ return mDisplayName; }

    /**
    * @brief sets the text that should accompany parameters when displayed
    */
    void displayName(std::string displayName){ mDisplayName = displayName; }

	/**
	* @brief getGroup returns the name of the group for the parameter
	*/
    std::string getGroup() { return mGroup; }

	/**
	 * @brief Generic function to return the value of the parameter as a float.
	 * 
	 * If not implemented, it will return 0.
	 */
	virtual float toFloat() {
		return 0.f;
	}
	
	/**
	 * @brief Generic function to set the parameter from a single float value
	 * 
	 * Will only have effect on parameters that have a single internal value and have implemented this function
	 */
    virtual void fromFloat(float value) {
		return;
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

    virtual void sendValue(osc::Send &sender) {
        std::cout << "sendValue function not implemented for " << typeid(*this).name() << std::endl;
    }

protected:
	std::string mFullAddress;
	std::string mParameterName;
    std::string mDisplayName;
	std::string mGroup;
	std::string mPrefix;

    std::map<std::string, float> mHints; // Provide hints for behavior
};


/**
 * @brief The ParameterWrapper class provides a generic thread safe Parameter class from the ParameterType template parameter
 */
template<class ParameterType>
class ParameterWrapper : public ParameterMeta {
public:
	/**
	* @brief ParameterWrapper
   *
   * @param parameterName The name of the parameter
   * @param Group The group the parameter belongs to
   * @param defaultValue The initial value for the parameter
   * @param prefix An address prefix that is prepended to the parameter's OSC address
   * @param min Minimum value for the parameter
   * @param max Maximum value for the parameter
   *
   * The mechanism used to protect data is locking a mutex within the set() function
   * and doing try_lock() on the mutex to update a cached value in the get()
   * function. In the worst case this might incur some jitter when reading the value.
   */
    ParameterWrapper(std::string parameterName, std::string group = "",
              ParameterType defaultValue = ParameterType(),
	          std::string prefix ="");

	ParameterWrapper(std::string parameterName, std::string Group,
	          ParameterType defaultValue,
	          std::string prefix,
	          ParameterType min,
	          ParameterType max
	        );
	
	ParameterWrapper(const ParameterWrapper& param);

    virtual ~ParameterWrapper();
	
	/**
	 * @brief set the parameter's value
	 * 
     * This function is thread-safe and can be called from any number of threads.
     * It blocks to lock a mutex so its use in critical contexts should be avoided.
	 */
    virtual void set(ParameterType value)
    {
//        if (value > mMax) value = mMax;
//        if (value < mMin) value = mMin;
        if (mProcessCallback) {
            value = (*mProcessCallback)(value); //, mProcessUdata);
        }
        for(auto cb:mCallbacks) {
            (*cb)(value);
        }
        setLocking(value);
    }

	/**
	 * @brief set the parameter's value without calling callbacks
	 *
	 * This function is thread-safe and can be called from any number of threads.
     * It blocks to lock a mutex so its use in critical contexts should be avoided.
	 * The processing callback is called, but the callbacks registered with
	 * registerChangeCallback() are not called. This is useful to avoid infinite
	 * recursion when a widget sets the parameter that then sets the widget.
	 */
    virtual void setNoCalls(ParameterType value, void *blockReceiver = NULL)
    {
//        if (value > mMax) value = mMax;
//        if (value < mMin) value = mMin;
        if (mProcessCallback) {
            value = (*mProcessCallback)(value); //, mProcessUdata);
        }

        if (blockReceiver) {
            for(auto cb:mCallbacks) {
                (*cb)(value);
            }
        }
        setLocking(value);
    }

	/**
	 * @brief set the parameter's value forcing a lock
	 */
	inline void setLocking(ParameterType value)
	{
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
	 * @brief set the minimum value for the parameter
	 * 
	 * The value returned by the get() function will be clamped and will not go
	 * under the value set by this function.
	 */
	void min(ParameterType minValue) {mMin = minValue;}
	ParameterType min() {return mMin;}
	
	/**
	 * @brief set the maximum value for the parameter
	 * 
	 * The value returned by the get() function will be clamped and will not go
	 * over the value set by this function.
	 */
	void max(ParameterType maxValue) {mMax = maxValue;}
	ParameterType max() {return mMax;}

    // typedef ParameterType (*ParameterProcessCallback)(ParameterType value, void *userData);
	// typedef void (*ParameterChangeCallback)(ParameterType value, void *sender,
	                                        // void *userData, void * blockSender);

	typedef const std::function<ParameterType (ParameterType)> ParameterProcessCallback;
	typedef const std::function<void (ParameterType)> ParameterChangeCallback;

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
	 * @brief registerChangeCallback adds a callback to be called when the value changes
	 *
	 * This function appends the callback to a list of callbacks to be called
	 * whenever a value changes.
	 *
     * @param cb
	 */
    void registerChangeCallback(ParameterChangeCallback cb);

	std::vector<ParameterWrapper<ParameterType> *> operator<< (ParameterWrapper<ParameterType> &newParam)
	{ std::vector<ParameterWrapper<ParameterType> *> paramList;
		paramList.push_back(&newParam);
		return paramList; }

	std::vector<ParameterWrapper<ParameterType> *> &operator<< (std::vector<ParameterWrapper<ParameterType> *> &paramVector)
    { paramVector.push_back(this);
		return paramVector;
	}

	// Allow automatic conversion to the data type
	// this allows e.g. float value = parameter;
	operator ParameterType() { return this->get();}

	ParameterWrapper<ParameterType> operator= (const ParameterType value) { this->set(value); return *this; }
	
protected:
	ParameterType mMin;
	ParameterType mMax;

    std::shared_ptr<ParameterProcessCallback> mProcessCallback;
	// void * mProcessUdata;
    std::vector<std::shared_ptr<ParameterChangeCallback>> mCallbacks;
	// std::vector<void *> mCallbackUdata;

private:
    std::mutex *mMutex; // pointer to avoid having to explicitly declare copy/move
	ParameterType mValue;
    ParameterType mValueCache;
};


/**
 * @brief The Parameter class
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

class Parameter : public ParameterWrapper<float>
{
public:
	/**
	* @brief Parameter
   *
   * @param parameterName The name of the parameter
   * @param Group The group the parameter belongs to
   * @param defaultValue The initial value for the parameter
   * @param prefix An address prefix that is prepended to the parameter's OSC address
   * @param min Minimum value for the parameter
   * @param max Maximum value for the parameter
   *
   * This Parameter class is designed for parameters that can be expressed as a
   * single float. It realies on float being atomic on the platform so there
   * is no locking. This is a safe assumption for most platforms today.
   */
    Parameter(std::string parameterName, std::string Group = "",
              float defaultValue = 0,
	          std::string prefix = "",
	          float min = -99999.0,
	          float max = 99999.0
	        );

	Parameter(const al::Parameter& param) :
	    ParameterWrapper<float>(param)
	{
		mFloatValue = param.mFloatValue;
	}

	/**
	 * @brief set the parameter's value
	 *
	 * This function is thread-safe and can be called from any number of threads
     * It does not block and relies on the atomicity of float.
	 */
	virtual void set(float value) override;

	/**
	 * @brief set the parameter's value without calling callbacks
	 *
	 * This function is thread-safe and can be called from any number of threads.
	 * The processing callback is called, but the callbacks registered with
	 * registerChangeCallback() are not called. This is useful to avoid infinite
	 * recursion when a widget sets the parameter that then sets the widget.
	 */
	virtual void setNoCalls(float value, void *blockReceiver = NULL) override;

	/**
	 * @brief get the parameter's value
	 *
	 * This function is thread-safe and can be called from any number of threads
	 *
	 * @return the parameter value
	 */
	virtual float get() override;

	virtual float toFloat() override {
		return mFloatValue;
	}

	virtual void fromFloat(float value) override {
		mFloatValue = value;
	}

	float operator= (const float value) { this->set(value); return value; }

    virtual void sendValue(osc::Send &sender) override {
        sender.send(getFullAddress(), get());
    }

private:
	float mFloatValue;
};


class ParameterInt : public ParameterWrapper<int32_t>
{
public:
	/**
	* @brief ParameterInt
   *
   * @param parameterName The name of the parameter
   * @param Group The group the parameter belongs to
   * @param defaultValue The initial value for the parameter
   * @param prefix An address prefix that is prepended to the parameter's OSC address
   * @param min Minimum value for the parameter
   * @param max Maximum value for the parameter
   *
   * This Parameter class is designed for parameters that can be expressed as a
   * single 32 bit integer number. It realies on float being atomic on the
   * platform so there is no locking. This is a safe assumption for most
   * desktop platforms today.
   */
    ParameterInt(std::string parameterName, std::string Group = "",
              int32_t defaultValue = 0,
	          std::string prefix = "",
	          int32_t min = 0,
	          int32_t max = 127
	        );

	ParameterInt(const al::ParameterInt& param) :
	    ParameterWrapper<int32_t>(param)
	{
		mIntValue = param.mIntValue;
	}

	/**
	 * @brief set the parameter's value
	 *
	 * This function is thread-safe and can be called from any number of threads
     * It does not block and relies on the atomicity of float.
	 */
	virtual void set(int32_t value) override;

	/**
	 * @brief set the parameter's value without calling callbacks
	 *
	 * This function is thread-safe and can be called from any number of threads.
	 * The processing callback is called, but the callbacks registered with
	 * registerChangeCallback() are not called. This is useful to avoid infinite
	 * recursion when a widget sets the parameter that then sets the widget.
	 */
	virtual void setNoCalls(int32_t value, void *blockReceiver = NULL) override;

	/**
	 * @brief get the parameter's value
	 *
	 * This function is thread-safe and can be called from any number of threads
	 *
	 * @return the parameter value
	 */
	virtual int32_t get() override;

	virtual float toFloat() override {
		return mIntValue;
	}

	virtual void fromFloat(float value) override {
		mIntValue = value;
	}

	float operator= (const int32_t value) { this->set(value); return value; }

    virtual void sendValue(osc::Send &sender) override {
        sender.send(getFullAddress(), get());
    }

private:
	int32_t mIntValue;
};

class ParameterBool : public Parameter
{
public:
	/**
	* @brief ParameterBool
   *
   * @param parameterName The name of the parameter
   * @param Group The group the parameter belongs to
   * @param defaultValue The initial value for the parameter
   * @param prefix An address prefix that is prepended to the parameter's OSC address
   * @param min Value when off/false
   * @param max Value when on/true
   *
   * This ParameterBool class is designed for boolean parameters that have
   * float values for on or off states. It relies on floats being atomic on
   * the platform.
   */
    ParameterBool(std::string parameterName, std::string Group = "",
              float defaultValue = 0,
	          std::string prefix = "",
	          float min = 0,
	          float max = 1.0
            );

    bool operator= (const bool value) { this->set(value == 1.0); return value == 1.0; }

	virtual float toFloat() override {
		return get();
	}

	virtual void fromFloat(float value) override {
		set(value);
	}
};

// These three types are blocking, should not be used in time-critical contexts
// like the audio callback. The classes were explicitly defined to overcome
// the issues related to the > and < operators needed when validating minumum
// and maximum values for the parameter
class ParameterString: public ParameterWrapper<std::string>
{
public:
    ParameterString(std::string parameterName, std::string Group = "",
                  std::string  defaultValue = "",
	              std::string prefix = "") :
	    ParameterWrapper<std::string>(parameterName, Group, defaultValue, prefix)
	{ }

	virtual float toFloat() override {
		return std::stof(get());
	}

	virtual void fromFloat(float value) override {
		set(std::to_string(value));
	}

    virtual void sendValue(osc::Send &sender) override {
        sender.send(getFullAddress(), get());
    }
};

class ParameterVec3: public ParameterWrapper<al::Vec3f>
{
public:
    ParameterVec3(std::string parameterName, std::string Group = "",
                  al::Vec3f defaultValue = al::Vec3f(),
	              std::string prefix = "") :
	    ParameterWrapper<al::Vec3f>(parameterName, Group, defaultValue, prefix)
	{ }

	ParameterVec3 operator=(const Vec3f vec) {this->set(vec); return *this;}

	float operator[](size_t index) { Vec3f vec = this->get(); return vec[index];}

    virtual void sendValue(osc::Send &sender) override {
        Vec3f vec = get();
        sender.send(getFullAddress(), vec.x, vec.y, vec.z);
    }
};

class ParameterVec4: public ParameterWrapper<al::Vec4f>
{
public:
    ParameterVec4(std::string parameterName, std::string Group = "",
                  al::Vec4f defaultValue = al::Vec4f(),
	              std::string prefix = "") :
	    ParameterWrapper<al::Vec4f>(parameterName, Group, defaultValue, prefix)
	{ }

	ParameterVec4 operator=(const Vec4f vec) {this->set(vec); return *this;}

	float operator[](size_t index) { Vec4f vec = this->get(); return vec[index];}

    virtual void sendValue(osc::Send &sender) override {
        Vec4f vec = get();
        sender.send(getFullAddress(), vec.x, vec.y, vec.z, vec.w);
    }
};


class ParameterPose: public ParameterWrapper<al::Pose>
{
public:
    ParameterPose(std::string parameterName, std::string Group = "",
                  al::Pose defaultValue = al::Pose(),
                  std::string prefix = "") :
        ParameterWrapper<al::Pose>(parameterName, Group, defaultValue, prefix)
    { }

    al::Pose operator=(const al::Pose vec) {this->set(vec); return *this;}


    virtual void sendValue(osc::Send &sender) override {
        Pose pose = get();
        Quatd q = pose.quat();
        sender.send(getFullAddress(), float(pose.x()), float(pose.y()), float(pose.z()),
                    float(q.w), float(q.x), float(q.y), float(q.z));
    }

    void setPos(const al::Vec3d v){ this->set(al::Pose(v,this->get().quat())); }
    void setQuat(const al::Quatd q){ this->set(al::Pose(this->get().pos(),q)); }
//    float operator[](size_t index) { Pose vec = this->get(); return vec[index];}
};


class ParameterMenu : public ParameterWrapper<int>
{
public:
	ParameterMenu(std::string parameterName, std::string Group = "",
		int defaultValue = 0,
		std::string prefix = "") :
		ParameterWrapper<int>(parameterName, Group, defaultValue, prefix)
	{ }

	int operator=(const int value) { this->set(value); return *this; }

	void setElements(std::vector<std::string> elements) {
		mElements = elements;
	}

	std::vector<std::string> getElements() { return mElements; }

	std::string getCurrent() {
		if (mElements.size() > 0) {
			return mElements[get()];
		}
		else {
			return "";
		}
	}

	virtual float toFloat() override {
		return (float) get();
		// return std::stof(getCurrent());
	}

	virtual void fromFloat(float value) override {
		set( (int) value);
	}

    virtual void sendValue(osc::Send &sender) override {
        sender.send(getFullAddress(), get());
    }

private:
	std::vector<std::string> mElements;
};


/**
 * @brief A parameter representing selected items from a list
 *
 * The unsigned int value is a bit field, each bit representing
 * whether an element is selected or not.
 *
 */
class ParameterChoice : public ParameterWrapper<uint16_t>
{
public:
    ParameterChoice(std::string parameterName, std::string Group = "",
        uint16_t defaultValue = 0,
        std::string prefix = "") :
        ParameterWrapper<uint16_t>(parameterName, Group, defaultValue, prefix)
    {set(0); }

    uint16_t operator=(const uint16_t value) { this->set(value); return *this; }

    void setElements(std::vector<std::string> &elements, bool allOn = false) {
        mElements = elements;
        min(0);
        assert((1 << (elements.size() - 1) ) < UINT16_MAX);
        max(1 << (elements.size() - 1));
        if (allOn) {
            uint16_t value = 0;
            for (unsigned int i = 0; i < elements.size(); i++) {
                value |= 1 << i;
            }
            set(value);
        }
    }

    void setElementSelected(std::string name, bool selected = true) {

        for (unsigned int i = 0; i < mElements.size(); i++) {
            if (mElements[i] == name) {
                uint16_t value = get();
                if (selected) {
                    value |= 1 << i;
                } else {
                    value ^= value | 1 << i;
                }
                set(value);
            }
        }
    }

    std::vector<std::string> getElements() { return mElements; }

    std::vector<std::string> getSelectedElements() {
        std::vector<std::string> selected;
        for (unsigned int i = 0; i < mElements.size(); i++) {
            if (get() & (1 << i)) {
                if (mElements.size() > i) {
                    selected.push_back(mElements[i]);
                }
                
            }
        }
        return selected;
    }

	virtual float toFloat() override {
		return (float) get();
		// return std::stof(getCurrent());
	}

	virtual void fromFloat(float value) override {
		set( (int) value);
	}
    virtual void sendValue(osc::Send &sender) override {
        sender.send(getFullAddress(), get());
    }

private:
    std::vector<std::string> mElements;
};

class ParameterColor: public ParameterWrapper<al::Color>
{
public:
    ParameterColor(std::string parameterName, std::string Group = "",
                  al::Color defaultValue = al::Color(),
                  std::string prefix = "") :
        ParameterWrapper<al::Color>(parameterName, Group, defaultValue, prefix)
    { }

    ParameterColor operator=(const al::Color vec) {this->set(vec); return *this;}

    virtual void sendValue(osc::Send &sender) override {
        Color c = get();
        sender.send(getFullAddress(), c.r, c.g, c.b, c.a);
    }
};


// Implementations -----------------------------------------------------------

template<class ParameterType>
ParameterWrapper<ParameterType>::~ParameterWrapper()
{
    delete mMutex;
}

template<class ParameterType>
ParameterWrapper<ParameterType>::ParameterWrapper(std::string parameterName, std::string group,
          ParameterType defaultValue,
          std::string prefix) :
    ParameterMeta(parameterName, group, prefix), mProcessCallback(nullptr)
{
	mValue = defaultValue;
	mValueCache = defaultValue;
    mMutex = new std::mutex;
}


template<class ParameterType>
ParameterWrapper<ParameterType>::ParameterWrapper(std::string parameterName, std::string group, ParameterType defaultValue,
                     std::string prefix, ParameterType min, ParameterType max) :
    ParameterWrapper<ParameterType>::ParameterWrapper(parameterName, group,
                                                      defaultValue, prefix)
{
	mMin = min;
	mMax = max;
    mMutex = new std::mutex;
}

template<class ParameterType>
ParameterWrapper<ParameterType>::ParameterWrapper(const ParameterWrapper<ParameterType> &param)
	: ParameterMeta(param.mParameterName, param.mGroup, param.mPrefix)
{
	mMin = param.mMin;
	mMax = param.mMax;
	mProcessCallback = param.mProcessCallback;
	// mProcessUdata = param.mProcessUdata;
	mCallbacks = param.mCallbacks;
    mMutex = new std::mutex;
	// mCallbackUdata = param.mCallbackUdata;
}

template<class ParameterType>
ParameterType ParameterWrapper<ParameterType>::get()
{
    if (mMutex->try_lock()) {
		mValueCache = mValue;
        mMutex->unlock();
	}
	return mValueCache;
}

template<class ParameterType>
void ParameterWrapper<ParameterType>::setProcessingCallback(ParameterWrapper::ParameterProcessCallback cb)
{
    mProcessCallback = std::make_shared<ParameterProcessCallback>(cb);
	// mProcessUdata = userData;
}

template<class ParameterType>
void ParameterWrapper<ParameterType>::registerChangeCallback(ParameterWrapper::ParameterChangeCallback cb)
{
    mCallbacks.push_back(std::make_shared<ParameterChangeCallback>(cb));
    // mCallbackUdata.push_back(userData);
}

}


#endif // AL_PARAMETER_H

// For backward compatibility, as ParameterServer was included in this file. Should be removed at some point.
//#include "al/util/ui/al_ParameterServer.hpp"
