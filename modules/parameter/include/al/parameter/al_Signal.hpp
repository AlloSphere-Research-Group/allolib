#ifndef AL_SIGNAL_HPP
#define AL_SIGNAL_HPP

/**
 * @file al_Signal.hpp
 * @brief Long-term name for path-addressable typed values (Parameter API v3).
 *
 * Direction: authors and ECS/composition code prefer Signal language;
 * existing Parameter / ParameterInt / … types remain the implementations and
 * stay ABI-compatible.
 *
 *   Signal<T>       — general typed wrapper (mutex store by default)
 *   ScalarSignal<T> — numeric lock-friendly store (ParamNumeric)
 *   SignalFloat     — classic float Parameter (audio-thread friendly get)
 */

#include "al/ui/al_Parameter.hpp"

namespace al {

template <typename T> using Signal = ParameterWrapper<T>;

template <typename T> using ScalarSignal = ParamNumeric<T>;

using SignalFloat = Parameter;
using SignalBool = ParameterBool;
using SignalInt = ParameterInt;
using SignalInt64 = ParameterInt64;
using SignalDouble = ParameterDouble;
using SignalString = ParameterString;
using SignalVec3 = ParameterVec3;
using SignalVec4 = ParameterVec4;
using SignalVec5 = ParameterVec5;
using SignalPose = ParameterPose;
using SignalColor = ParameterColor;
using SignalMenu = ParameterMenu;
using SignalChoice = ParameterChoice;

} // namespace al

#endif
