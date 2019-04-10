#include <iostream>
#include <algorithm>
#include <cstring>
#include <cassert>
#include <algorithm>


//#include "zita-convolver-4.0.0/libs/zita-convolver.h"
#include <zita-convolver.h>

#if ZITA_CONVOLVER_MAJOR_VERSION != 4
#error "This program requires zita-convolver 4.x.x"
#endif

#include "al_ext/spatialaudio/al_Convolver.hpp"

using namespace al;

Convolver::Convolver() :
  m_Convproc(nullptr)
{
}

bool Convolver::configure(unsigned int ioBufferSize,
                          vector<float *> IRs, uint32_t IRlength,
                          map<uint32_t, vector<uint32_t>> channelRoutingMap,
                          uint32_t basePartitionSize, float density,
                          uint32_t options)
{
  mNumInputs = channelRoutingMap.size();
  mBufferSize = ioBufferSize;
  mIRs = IRs;
  mIRlength = IRlength;
  mChannelMap = channelRoutingMap;
  if (mNumInputs < 1 || mIRs.size() < 1 || mChannelMap.size() < 1) {
    std::cerr << "ERROR Convolver not properly configured" << std::endl;
    return false;
  }
  for (auto mapEntry: mChannelMap) {
    for (auto outputIndex: mapEntry.second) {
      if (outputIndex >= IRlength) {
        std::cerr << "ERROR Convolver: invalid IR index " << outputIndex << std::endl;
        return false;
      }
    }
  }
  if (basePartitionSize < ioBufferSize) {
    basePartitionSize = ioBufferSize;
    std::cout << "setting base partition size to ioBufferSize" <<std::endl;
  }

  if(m_Convproc != nullptr) {
    delete m_Convproc;
  }
  m_Convproc = new Convproc;
  m_Convproc->set_options(options);

  int configResult = m_Convproc->configure(channelRoutingMap.size(), mIRs.size(),
                                           mIRlength, mBufferSize,
                                           basePartitionSize, (mIRlength/2 < Convproc::MAXPART)?mIRlength:Convproc::MAXPART,
                                           density);
  if(configResult != 0){
    std::cerr << "ERROR convolution config failed" << std::endl;
    m_Convproc->cleanup();
    return false;
  }
  for (auto &routing: mChannelMap) {
    for (auto outputIndex: routing.second) {
      if (m_Convproc->impdata_create(routing.first, outputIndex, 1, IRs[outputIndex], 0, IRlength) != 0) {
        std::cerr << "ERROR setting convolution engine routing" << std::endl;
        m_Convproc->cleanup();
        return false;
      }
    }
  }
  if (m_Convproc->start_process(0, 0) != 0) {
    m_Convproc->cleanup();
    std::cerr << "ERROR starting convolution engine" << std::endl;
    return false;
  }
  return true;
}

float *Convolver::getInputBuffer(unsigned int index) {
  return m_Convproc->inpdata(index);
}

float *Convolver::getOutputBuffer(unsigned int index)
{
  return m_Convproc->outdata(index);
}

bool Convolver::processBuffer()
{
  //process
  // TODO to sync or not to sync...
  return m_Convproc->process(true) == 0;
}

bool Convolver::shutdown(void){
  if(m_Convproc->stop_process() != 0){
    cerr << "Warning: could not stop process" << endl;
    return false;
  }
  if(m_Convproc->cleanup() != 0){
    cerr << "Warning: cleanup failed" << endl;
    return false;
  }
  return true;
}

