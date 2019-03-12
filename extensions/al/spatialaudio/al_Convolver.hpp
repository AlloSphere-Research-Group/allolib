#ifndef AL_CONVOLVER_H
#define AL_CONVOLVER_H

#include <vector>
#include <map>

#define MAXSIZE 0x00100000

class Convproc;

namespace al {

using namespace std;

/**
  * @brief Convolver Realtime multichannel convolution class.
  *
  * Built on zita convolver, which implements a realtime multithreaded multichannel convolution algorithm using non-uniform partitioning.
  *
  */

class Convolver
{

public:
  Convolver();

  /// @brief Sets up convolver. Must be called prior to processing.
  ///
  /// Checks for valid parameters, initializes convolver and impulse responses. Output from disabled channels is set to 0.
  ///
  /// The number of IRs determines the number of output channels
  /// Use cases:
  ///  - Decorrelation: many to many each with a different IR
  ///  - Auralization: one to many each with a different IR
  ///                : many to many each with a different IR
  ///  - DRC : many to many each with different IR
  ///
  /// @param[in] ioBufferSize Size of the input and output buffers
  /// @param[in] IRs The deinterleaved IR channels.
  /// @param[in] IRlength The size of IRs
  /// @param[in] channelRoutingMap Map of input to output channels
  /// @param[in] basePartitionSize Should be set to audio callback size to minimize latency. Cannot be less than 64 samples.
  /// @param[in] density Density parameter for zita convolver.
  /// @param[in] options Options to be passed to zita convolver. Currently supports OPT_FFTW_MEASURE = 1, OPT_VECTOR_MODE  = 2.
  /// @return Returns true upon success

  bool configure(unsigned int ioBufferSize,
                 vector<float *> IRs, uint32_t IRlength,
                 map<uint32_t, vector<uint32_t> > channelRoutingMap,
                 uint32_t basePartitionSize = 64, float density = 0.0f,
                 uint32_t options = 0);

  float *getInputBuffer(unsigned int index);

  float *getOutputBuffer(unsigned int index);

  bool processBuffer();

  /// @brief Stops processing audio and cleans up convolver object.
  /// @return Returns true upon success.
  bool shutdown(void);

private:
  vector<unsigned int> m_activeChannels;
  vector<unsigned int> m_disabledChannels;
  int m_inputChannel;
  bool m_inputsAreBuses;

  size_t mNumInputs;
  unsigned int mBufferSize;
  vector<float *> mIRs;
  uint32_t mIRlength;
  map<uint32_t, vector<uint32_t>> mChannelMap;

  Convproc *m_Convproc;
};

}

#endif // AL_CONVOLVER_H
