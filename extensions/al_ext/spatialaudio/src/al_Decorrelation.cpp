/*	Decorrelation

    Copyright (C) 2014. AlloSphere Research Group, Media Arts & Technology, UCSB.
    Copyright (C) 2014. The Regents of the University of California.
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

    For details about the process to generate the IRs for decorrelation, see:

    Kendall, G. (1995). The Decorrelation of Audio Signals an Its Impact on Spatial Imagery. Computer Music Journal, 19:4, 71–87.

    and:

    Zotter, F., Frank, M., Marentakis, G., & Sontacchi, A. (2011). Phantom Source Widening with Deterministic Frequency Dependent Time Delays. DAFx-11, 307–312.

    File author(s):
    Andres Cabrera, mantaraya36@gmail.com
*/

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cmath>
#include <cstring>
#include <cassert>

#include <Gamma/FFT.h>

#include "al_ext/spatialaudio/al_Decorrelation.hpp"

using namespace al;
using namespace std;

#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif

Decorrelation::Decorrelation(uint32_t size) :
  mIRlength(size)
{
}

Decorrelation::~Decorrelation()
{
  freeIRs();
}

long al::Decorrelation::getCurrentSeed()
{
  return mSeed;
}

void Decorrelation::generateIRs(long seed, float maxjump, float phaseFactor)
{
  float *ampSpectrum = (float *) calloc(mIRlength, sizeof(float));
  float *phsSpectrum = (float *) calloc(mIRlength, sizeof(float));
  float *complexSpectrum = (float *) calloc(mIRlength * 2, sizeof(float));;
  //	#    max_jump -  is the maximum phase difference (in radians) between bins
  //	#             if -1, the random numbers are used directly (no jumping).

  freeIRs();

  int n = mIRlength/2; // before mirroring

  // Seed random number generator
  if (seed >= 0) {
    mSeed = seed;
  } else {
    mSeed = time(0);
  }
  srand(mSeed);
  for (int irIndex = 0; irIndex < mNumOuts; irIndex++) {
    // Fill in DC and Nyquist
    ampSpectrum[0] = 1.0;
    phsSpectrum[0] = 0.0;
    complexSpectrum[0] = ampSpectrum[0] * cos(phsSpectrum[0]);
    complexSpectrum[1] = ampSpectrum[0] * sin(phsSpectrum[0]);
    complexSpectrum[(n*2)] = ampSpectrum[0] * cos(phsSpectrum[0]);
    complexSpectrum[(n*2) + 1] = ampSpectrum[0] * sin(phsSpectrum[0]);

    float old_phase = 0;
    for (int i=1; i < n; i++) {
      ampSpectrum[i] = 1.0;
      if (maxjump == -1.0) {
        phsSpectrum[i] = ((rand() / (float) RAND_MAX) * M_PI)- (M_PI/2.0);
      } else {
        // make phase only move +- limit
        float delta = ((rand() / ((float) RAND_MAX)) * 2.0 * maxjump) - maxjump;
        //				std::cout << "delta " << delta << std::endl;
        float new_phase = old_phase + delta;
        phsSpectrum[i] = new_phase * phaseFactor;
        old_phase = new_phase;
      }

      complexSpectrum[i*2] = ampSpectrum[i] * cos(phsSpectrum[i]); // Real part
      complexSpectrum[i*2 + 1] = ampSpectrum[i] * sin(phsSpectrum[i]); // Imaginary
      //			std::cout << complexSpectrum[i*2] << ", " << complexSpectrum[i*2 + 1] << ",";
    }

    //		std::cout << ".... " <<std::endl;
    gam::RFFT<float> fftObj(mIRlength);

    fftObj.inverse(complexSpectrum, true);
    float *irdata = (float *) calloc(mIRlength, sizeof(float));

    for (int i=1; i <= mIRlength; i++) {
      irdata[i - 1] = complexSpectrum[i]/mIRlength;
      //			std::cout << complexSpectrum[i]/mSize << "," << std::endl;
    }
    mIRs.push_back(irdata);
  }

  free(ampSpectrum);
  free(phsSpectrum);
  free(complexSpectrum);
}

void Decorrelation::generateDeterministicIRs(long seed, float deltaFreq, float maxFreqDev,
                                             float maxTau, float startPhase, float phaseDev)
{
  float *ampSpectrum = (float *) calloc(mIRlength, sizeof(float));
  float *phsSpectrum = (float *) calloc(mIRlength, sizeof(float));
  float *complexSpectrum = (float *) calloc(mIRlength * 2, sizeof(float));;
  //	#    max_jump -  is the maximum phase difference (in radians) between bins
  //	#             if -1, the random numbers are used directly (no jumping).

  freeIRs();

  int n = mIRlength/2; // before mirroring

  // Seed random number generator
  if (seed >= 0) {
    mSeed = seed;
  } else {
    mSeed = time(0);
  }
  srand(mSeed);

  for (int irIndex = 0; irIndex < mNumOuts; irIndex++) {
    float freq = deltaFreq + ((2.0 * maxFreqDev * rand() / (float) RAND_MAX) - maxFreqDev);
    std::cout << "freq " << irIndex << ":" << freq << std::endl;
    for (int i=0; i < n + 1; i++) {
      ampSpectrum[i] = 1.0;
      float phaseOffset = startPhase + ((2.0 * phaseDev * rand() / (float) RAND_MAX) - phaseDev);
      phsSpectrum[i] = maxTau * sin(phaseOffset + (2 * M_PI * i * freq / n));

      complexSpectrum[i*2] = ampSpectrum[i] * cos(phsSpectrum[i]); // Real part
      complexSpectrum[i*2 + 1] = ampSpectrum[i] * sin(phsSpectrum[i]); // Imaginary
      //			std::cout << complexSpectrum[i*2] << ", " << complexSpectrum[i*2 + 1] << ",";
    }

    //		std::cout << ".... " <<std::endl;
    gam::RFFT<float> fftObj(mIRlength);

    fftObj.inverse(complexSpectrum, true);
    float *irdata = (float *) calloc(mIRlength, sizeof(float));

    for (int i=1; i <= mIRlength; i++) {
      irdata[i - 1] = complexSpectrum[i]/mIRlength;
      //			std::cout << complexSpectrum[i]/mSize << "," << std::endl;
    }
    mIRs.push_back(irdata);
  }
  free(ampSpectrum);
  free(phsSpectrum);
  free(complexSpectrum);
}

//void Decorrelation::onAudioCB(al::AudioIOData &io)
//{

//  unsigned int convChannel = 0;
//  for (auto channelMap: mRoutingMap) {
//    const float *inBuf;
//    if (mInputsAreBuses) {
//      inBuf = io.busBuffer(channelMap.first);
//    } else {
//      inBuf = io.inBuffer(channelMap.first);
//    }
//    memcpy(mConv.getInputBuffer(convChannel++), inBuf, io.framesPerBuffer() * sizeof(float));
//  }
//  mConv.processBuffer();
//  convChannel = 0;
//  for (auto channelMap: mRoutingMap) {
//    for(auto outputChan: channelMap.second) {
//      float *convBuffer = mConv.getOutputBuffer(convChannel++);
//      for (unsigned int i = 0; i < io.framesPerBuffer(); i++) {
//        io.out(outputChan, i) += *convBuffer++;
//      }
//    }
//  }
//}

float *al::Decorrelation::getIR(int index)
{
  if (index < 0 || index >= mNumOuts) {
    return nullptr;
  }
  return mIRs[index];
}

uint32_t al::Decorrelation::getSize()
{
  return mIRlength;
}

void al::Decorrelation::freeIRs()
{
  for (unsigned int i = 0; i < mIRs.size(); i++){
    free(mIRs[i]);
  }
  mIRs.clear();
}

bool Decorrelation::configure(uint32_t bufferSize,
                              std::map<uint32_t, vector<uint32_t>> routingMap,
                              bool inputsAreBusses,
                              long seed, float maxjump, float phaseFactor)
{
  mSeed = seed;
  mNumOuts = 0;
  for(auto outputIndeces: routingMap) {
    mNumOuts += outputIndeces.second.size();
  }
  mRoutingMap = routingMap;
  mInputsAreBuses = inputsAreBusses;
  if (mIRlength > 16 && mNumOuts != 0) {
    generateIRs(seed, maxjump, phaseFactor);
  } else {
    mIRlength = 0;
    cout << "Invalid size: " << mIRlength << " numOuts: " << mNumOuts << endl;
    return false;
  }
  if (mIRlength >= 64) {
    unsigned int options = 2; //vector mode

    //        unsigned int numInputs, unsigned int ioBufferSize,
    //                                  vector<float *> IRs, uint32_t IRlength,
    //                                  vector<pair<uint32_t, uint32_t>> channelRoutingMap,
    //                                  uint32_t basePartitionSize, float density,
    //                                  uint32_t options

    mInputsAreBuses = inputsAreBusses;
    return mConv.configure(bufferSize, mIRs, mIRlength,
                           routingMap, bufferSize, 0, options);
  }
  return false;
}

bool Decorrelation::configureDeterministic(uint32_t bufferSize,
                                           std::map<uint32_t, vector<uint32_t>> routingMap,
                                           bool inputsAreBusses,
                                           long seed, float deltaFreq,
                                           float deltaFreqDev, float maxTau, float startPhase, float phaseDev)
{
  mNumOuts = 0;
  for(auto outputIndeces: routingMap) {
    mNumOuts += outputIndeces.second.size();
  }
  mRoutingMap = routingMap;
  mInputsAreBuses = inputsAreBusses;
  generateDeterministicIRs(seed, deltaFreq, deltaFreqDev, maxTau, startPhase, phaseDev);
  if (mIRlength >= 64) {
    uint32_t options = 2; //vector mode
    return mConv.configure(bufferSize, mIRs, mIRlength,
                           routingMap, bufferSize, 0, options);
  }
  return false;
}

float *Decorrelation::getInputBuffer(unsigned int index){
  return mConv.getInputBuffer(index);
}

float *Decorrelation::getOutputBuffer(unsigned int index){
  return mConv.getOutputBuffer(index);
}

