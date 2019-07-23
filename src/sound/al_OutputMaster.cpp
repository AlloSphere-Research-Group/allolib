
#include <iostream>
#include <sstream>
#include <float.h> // for FLT_MAX and FLT_MIN

#include "al/sound/al_OutputMaster.hpp"
#include "al/system/al_Time.hpp"

using namespace al;

template<class DataType>
void butter_set_fc(DataType fc, DataType sr, DataType *a, DataType *b, bool lp = true)
{
    DataType lambda;
    DataType const0 = M_PI/(double) sr;
    if (lp) { // low pass
        lambda = 1/tan(fc * const0);
    } else {
        lambda = tan(fc * const0);
    }
    DataType lambda_2 = lambda * lambda;
    a[0] = 1.0/(1.0 + (sqrt(DataType(2.0))*lambda) + (lambda_2));
    a[2] =a[0];
    b[2] =a[0] * (1.0 - (sqrt(DataType(2.0))*lambda) + (lambda_2));
    if (lp) {
        a[1] = 2.0 * a[0];
        b[1] = 2.0 * a[0] * (1.0 - lambda_2);
    } else {
        a[1] = - 2.0 * a[0];
        b[1] = 2.0 * a[0] * (lambda_2 - 1.0);
    }
}

OutputMaster::OutputMaster(unsigned int num_chnls, double sampleRate):
	m_numChnls(num_chnls), m_framesPerSec(sampleRate)
{
  initialize(num_chnls, sampleRate);
}

OutputMaster::~OutputMaster()
{
}

void OutputMaster::initialize(unsigned int num_chnls, double sampleRate)
{
  allocateChannels(num_chnls);
  initializeData();
  m_framesPerSec = sampleRate;
}

void OutputMaster::setMasterGain(double gain)
{
	m_masterGain = gain;
}

void OutputMaster::setGain(unsigned int channelIndex, double gain)
{
    assert (channelIndex < m_numChnls);
    m_gains[channelIndex] = gain;
}

void OutputMaster::setMuteAll(bool muteAll)
{
	m_muteAll = muteAll;
}

void OutputMaster::setClipperOn(bool clipperOn)
{
	m_clipperOn = clipperOn;
}

void OutputMaster::setMeterUpdateFreq(double freq)
{
	m_meterUpdateSamples = (int)(m_framesPerSec/freq);
}

void OutputMaster::setBassManagementFreq(double frequency)
{
	if (frequency > 0) {
		for (unsigned int i = 0; i < m_numChnls; i++) {
			butter_set_fc<float>(frequency, m_framesPerSec, m_lopass1[i].a(), m_lopass1[i].a());
			butter_set_fc<float>(frequency, m_framesPerSec, m_lopass2[i].a(), m_lopass2[i].a());
			butter_set_fc<float>(frequency, m_framesPerSec, m_hipass1[i].a(), m_hipass1[i].a(), false);
			butter_set_fc<float>(frequency, m_framesPerSec, m_hipass2[i].a(), m_hipass2[i].a(), false);
		}
	}
}

void OutputMaster::setBassManagementMode(bass_mgmt_mode_t mode)
{
	if (mode >= 0 && mode < BASSMODE_COUNT) {
		m_BassManagementMode = mode;
	}
}

void OutputMaster::setSwIndeces(int i1, int i2, int i3, int i4)
{
	swIndex[0] = i1;
	swIndex[1] = i1;
	swIndex[2] = i1;
	swIndex[3] = i1;
}

void OutputMaster::setMeterOn(bool meterOn)
{
	m_meterOn = meterOn;
}


int OutputMaster::getNumChnls()
{
	return m_numChnls;
}

void OutputMaster::onAudioCB(AudioIOData &io)
{
	unsigned int nframes = io.framesPerBuffer();
	double filt_out;
	double filt_low = 0.0;
	double master_gain;

//	m_parameterQueue.update(0);
	master_gain = m_masterGain * (m_muteAll ? 0.0 : 1.0);
    io.frame(0);
    while (io()) {
        double bassbuf = 0.0; // Accumulate sw signals
        for (unsigned int chan = 0; chan < m_numChnls; chan++) {
            double gain = master_gain * m_gains[chan];
            double filt_temp;

            switch (m_BassManagementMode) {
            case BASSMODE_NONE:
                break;
            case BASSMODE_MIX:
                filt_low = io.out(chan);
                break;
            case BASSMODE_LOWPASS:
                filt_temp = m_lopass1[chan](io.out(chan));
                filt_low = m_lopass1[chan](filt_temp);
                break;
            case BASSMODE_HIGHPASS:
                filt_temp = m_hipass1[chan](io.out(chan));
                filt_out = m_hipass1[chan](filt_temp);
                io.out(chan) = filt_out;
                break;
            case BASSMODE_FULL:
                filt_temp = m_lopass1[chan](io.out(chan));
                filt_low = m_lopass1[chan](filt_temp);
                filt_temp = m_hipass1[chan](io.out(chan));
                filt_out = m_hipass1[chan](filt_temp);
                io.out(chan) = filt_out;
                break;
            default:
                filt_low = 0.0;
                break;
            }
            bassbuf += filt_low;
            io.out(chan) *= gain;
            if (m_clipperOn && io.out(chan) > master_gain) {
                io.out(chan) = master_gain;
            }
        }
        if (m_BassManagementMode != BASSMODE_NONE) {
            int sw;
            for(sw = 0; sw < 4; sw++) {
                if (swIndex[sw] < 0) continue;
                io.out(swIndex[sw]) = bassbuf * m_gains[swIndex[sw]];
            }
        }
        if (m_meterOn) {
            for (unsigned int chan = 0; chan < m_numChnls; chan++) {
                float out = io.out(chan);
//                float absValue = fabs(out);
                if (m_meterMax[chan] < out) {
                    m_meterMax[chan] = out;
                }
                if (m_meterMin[chan] > out) {
                    m_meterMin[chan] = out;
                }
            }
            m_meterCounter++;
            if (m_meterCounter >= m_meterUpdateSamples) {
                m_meterMaxBuffer.write(m_meterMax.data());
                m_meterCounter = m_meterCounter - m_meterUpdateSamples;
                for (unsigned int chan = 0; chan < m_numChnls; chan++) {
                  //FIXME offload to separate thread, trigger by condition variable
                  // FIXME on separate thread allow both printing and sending values via OSC
                  std::cout <<  m_meterMax[chan] << " : " << m_meterMin[chan] << " ---- " << std::endl;
                    m_meterMax[chan] = FLT_MIN;
                    m_meterMin[chan] = FLT_MAX;
                }
            }
        }
	}
}

void OutputMaster::setGainTimestamped(al_sec until, int channelIndex, double gain)
{
	setGain(channelIndex, gain);
}

void OutputMaster::setMasterGainTimestamped(al_sec until, double gain)
{
	setMasterGain(gain);
}

void OutputMaster::setClipperOnTimestamped(al_sec until, bool on)
{
	setClipperOn(on);
}

void OutputMaster::setMuteAllTimestamped(al_sec until, bool on)
{
	setMuteAll(on);
}

void OutputMaster::setMeterOnTimestamped(al_sec until, bool on)
{
	setMeterOn(on);
}

void OutputMaster::setMeterupdateFreqTimestamped(al_sec until, double freq)
{
	setMeterUpdateFreq(freq);
}

void OutputMaster::setBassManagementFreqTimestamped(al_sec until, double freq)
{
	setBassManagementFreq(freq);
}

void OutputMaster::setBassManagementModeTimestamped(al_sec until, int mode)
{
	setBassManagementMode((bass_mgmt_mode_t) mode);
}
std::string OutputMaster::addressPrefix() const
{
	return m_addressPrefix;
}

void OutputMaster::setAddressPrefix(const std::string &addressPrefix)
{
	m_addressPrefix = addressPrefix;
}

bool OutputMaster::meterAddrHasChannel() const
{
	return m_meterAddrHasChannel;
}

void OutputMaster::setMeterAddrHasChannel(bool meterAddrHasChannel)
{
	m_meterAddrHasChannel = meterAddrHasChannel;
}


int OutputMaster::chanIsSubwoofer(int index)
{
	int i;
	for (i = 0; i < 4; i++) {
		if (swIndex[i] == index && m_BassManagementMode != BASSMODE_NONE) return 1;
	}
	return 0;
}

void OutputMaster::initializeData()
{
	m_masterGain = 1.0;
	m_muteAll = false;
	m_clipperOn = true;
	m_addressPrefix = "/Alloaudio";
	m_meterCounter = 0;
	m_meterOn = false;
	m_meterAddrHasChannel = false;

	setBassManagementMode(BASSMODE_NONE);
	setBassManagementFreq(150);
	setMeterUpdateFreq(10.0);
}

void OutputMaster::allocateChannels(unsigned int numChnls)
{
	m_gains.resize(numChnls);
    m_meterMaxBuffer.setSize(numChnls);
    m_meterMinBuffer.setSize(numChnls);
    m_meterMax.resize(numChnls);
    m_meterMin.resize(numChnls);

	m_lopass1.resize(numChnls);
	m_lopass2.resize(numChnls);
	m_hipass1.resize(numChnls);
	m_hipass2.resize(numChnls);
	swIndex[0] = numChnls - 1;
	swIndex[1] =  swIndex[2] = swIndex[3] = -1;

	for (unsigned int i = 0; i < numChnls; i++) {
		m_gains[i] = 1.0;
	}
    m_numChnls = numChnls;
}

