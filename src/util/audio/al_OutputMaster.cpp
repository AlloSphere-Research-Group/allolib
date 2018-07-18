
#include <iostream>
#include <sstream>

#include "al/util/audio/al_OutputMaster.hpp"
#include "al/core/system/al_Time.hpp"

//#include "alloaudio/butter.h"

//#include "firfilter.h"

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

OutputMaster::OutputMaster(unsigned int num_chnls, double sampleRate, const char *address, int port,
						   const char *sendAddress, int sendPort, al_sec msg_timeout):
	m_numChnls(num_chnls),
	/*m_meterBuffer(1024 * sizeof(float)),*/ m_framesPerSec(sampleRate)/*,
	osc::Recv(port, address, msg_timeout),
	m_sendAddress(sendAddress), m_sendPort(sendPort)*/
{
	allocateChannels(m_numChnls);
	initializeData();
}

OutputMaster::~OutputMaster()
{
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
	double bass_buf[nframes];
	double filt_out;
	double filt_low = 0.0;
	double master_gain;

//	m_parameterQueue.update(0);
	master_gain = m_masterGain * (m_muteAll ? 0.0 : 1.0);
	memset(bass_buf, 0, nframes * sizeof(double));
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
                float absValue = fabs(out);
                if (m_meterMax[chan] < absValue) {
                    m_meterMax[chan] = absValue;
                }
            }
            m_meterCounter++;
            if (m_meterCounter >= m_meterUpdateSamples) {
                m_meterBuffer.write(m_meterMax.data());
                m_meterCounter = m_meterCounter - m_meterUpdateSamples;
                for (unsigned int chan = 0; chan < m_numChnls; chan++) {
                    m_meterMax[chan] = 0.0;
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
	m_meterBuffer.setSize(numChnls);
    m_meterMax.resize(numChnls);

	m_lopass1.resize(numChnls);
	m_lopass2.resize(numChnls);
	m_hipass1.resize(numChnls);
	m_hipass2.resize(numChnls);
	swIndex[0] = numChnls - 1;
	swIndex[1] =  swIndex[2] = swIndex[3] = -1;

	for (unsigned int i = 0; i < numChnls; i++) {
		m_gains[i] = 1.0;
//		m_lopass1[i] = butter_create(m_framesPerSec, BUTTER_LP);
//		m_lopass2[i] = butter_create(m_framesPerSec, BUTTER_LP);
//		m_hipass1[i] = butter_create(m_framesPerSec, BUTTER_HP);
//		m_hipass2[i] = butter_create(m_framesPerSec, BUTTER_HP);
	}
}

//void *OutputMaster::meterThreadFunc(void *arg) {
//	int chanindex = 0;
//	OutputMaster *om = static_cast<OutputMaster *>(arg);
//	float meter_levels[om->m_numChnls];

//	al::osc::Send s(om->m_sendPort, om->m_sendAddress.c_str());
//	while(om->m_runMeterThread) {
//        std::unique_lock<std::mutex> lk(om->m_meterMutex);
//        om->m_meterCond.wait(lk);
//		int bytes_read = om->m_meterBuffer.read((char *) meter_levels, om->m_numChnls * sizeof(float));
//		if (bytes_read) {
//			if (bytes_read !=  om->m_numChnls * sizeof(float)) {
//				std::cerr << "Alloaudio: Warning. Meter values underrun." << std::endl;
//			}
//			for (int i = 0; i < bytes_read/sizeof(float); i++) {
//				if (om->m_meterAddrHasChannel) {
//					std::stringstream addr;
//					addr << om->m_addressPrefix << "/meterdb/" <<  chanindex + 1;
//					s.send(addr.str(),
//						   (float) (20.0 * log10(meter_levels[i])));
//				} else {
//					s.send(om->m_addressPrefix + "/meterdb", chanindex,
//						   (float) (20.0 * log10(meter_levels[i])));
//				}

//				chanindex++;
//				if (chanindex == om->m_numChnls) {
//					chanindex = 0;
//				}
//			}
//		}
//	}
//	return nullptr;
//}

//void OutputMaster::OSCHandler::onMessage(osc::Message &m)
//{
//	if (m.addressPattern() == outputmaster->m_addressPrefix + "/gain") {
//		if (m.typeTags() == "if") {
//			int chan;
//			float gain;
//			m >> chan >> gain;
//			outputmaster->m_parameterQueue.send(outputmaster->m_parameterQueue.now(),
//												outputmaster,
//												&OutputMaster::setGainTimestamped,
//												chan, (double) gain);
//		} else {
//			std::cerr << "Alloaudio: Wrong type tags for " << outputmaster->m_addressPrefix << "/gain message: "
//					  << m.typeTags() << std::endl;
//		}
//	} else if (m.addressPattern() == outputmaster->m_addressPrefix + "/global_gain") {
//		if (m.typeTags() == "f") {
//			float gain;
//			m >> gain;
//			outputmaster->m_parameterQueue.send(outputmaster->m_parameterQueue.now(),
//												outputmaster,
//												&OutputMaster::setMasterGainTimestamped,
//												(double) gain);
//		} else {
//			std::cerr << "Alloaudio: Wrong type tags for " + outputmaster->m_addressPrefix + "/global_gain message: "
//					  << m.typeTags() << std::endl;
//		}
//	} else if (m.addressPattern() == outputmaster->m_addressPrefix + "/clipper_on") {
//		if (m.typeTags() == "i") {
//			int clipper_on;
//			m >> clipper_on;
//			outputmaster->m_parameterQueue.send(outputmaster->m_parameterQueue.now(),
//												outputmaster,
//												&OutputMaster::setClipperOnTimestamped,
//												(bool) clipper_on != 0);
//		} else if (m.typeTags() == "f") {
//			float clipper_on;
//			m >> clipper_on;
//			outputmaster->m_parameterQueue.send(outputmaster->m_parameterQueue.now(),
//												outputmaster,
//												&OutputMaster::setClipperOnTimestamped,
//												(bool) clipper_on != 0);
//		} else {
//			std::cerr << "Alloaudio: Wrong type tags for " + outputmaster->m_addressPrefix + "/clipper_on: "
//					  << m.typeTags() << std::endl;
//		}
//	} else if (m.addressPattern() == outputmaster->m_addressPrefix + "/mute_all") {
//		if (m.typeTags() == "i") {
//			int mute_all;
//			m >> mute_all;
//			outputmaster->m_parameterQueue.send(outputmaster->m_parameterQueue.now(),
//												outputmaster, &OutputMaster::setMuteAllTimestamped,
//												(bool) mute_all != 0);
//		} else if (m.typeTags() == "f") {
//			float mute_all;
//			m >> mute_all;
//			outputmaster->m_parameterQueue.send(outputmaster->m_parameterQueue.now(),
//												outputmaster, &OutputMaster::setMuteAllTimestamped,
//												(bool) mute_all != 0);
//		} else {
//			std::cerr << "Alloaudio: Wrong type tags for " + outputmaster->m_addressPrefix + "/mute_all: "
//					  << m.typeTags() << std::endl;
//		}
//	} else if (m.addressPattern() == outputmaster->m_addressPrefix + "/meter_on") {
//		if (m.typeTags() == "i") {
//			int on;
//			m >> on;
//			outputmaster->m_parameterQueue.send(outputmaster->m_parameterQueue.now(),
//												outputmaster, &OutputMaster::setMeterOnTimestamped,
//												(bool) on != 0);
//		} else if (m.typeTags() == "f") {
//			float on;
//			m >> on;
//			outputmaster->m_parameterQueue.send(outputmaster->m_parameterQueue.now(),
//												outputmaster, &OutputMaster::setMeterOnTimestamped,
//												(bool) on != 0);
//		} else {
//			std::cerr << "Alloaudio: Wrong type tags for " + outputmaster->m_addressPrefix + "/meter_update_freq: "
//					 << m.typeTags() << std::endl;
//		}
//	} else if (m.addressPattern() == outputmaster->m_addressPrefix + "/meter_update_freq") {
//		if (m.typeTags() == "f") {
//			float freq;
//			m >> freq;
//			outputmaster->m_parameterQueue.send(outputmaster->m_parameterQueue.now(),
//												outputmaster, &OutputMaster::setMeterupdateFreqTimestamped,
//												(double) freq);
//		} else {
//			std::cerr << "Alloaudio: Wrong type tags for " + outputmaster->m_addressPrefix + "/meter_update_freq: "
//					 << m.typeTags() << std::endl;
//		}
//	} else if (m.addressPattern() == outputmaster->m_addressPrefix + "/bass_management_mode") {
//		if (m.typeTags() == "i") {
//			int mode;
//			m >> mode;
//			outputmaster->m_parameterQueue.send(outputmaster->m_parameterQueue.now(),
//												outputmaster, &OutputMaster::setBassManagementModeTimestamped,
//												(int) mode);
//		} else if (m.typeTags() == "f") {
//			float mode;
//			m >> mode;
//			outputmaster->m_parameterQueue.send(outputmaster->m_parameterQueue.now(),
//												outputmaster, &OutputMaster::setBassManagementModeTimestamped,
//												(int) mode);
//		} else{
//			std::cerr << "Alloaudio: Wrong type tags for " + outputmaster->m_addressPrefix + "//bass_management_mode: "
//					 << m.typeTags() << std::endl;
//		}
//	} else if (m.addressPattern() == outputmaster->m_addressPrefix + "/bass_management_freq") {
//		if (m.typeTags() == "f") {
//			float freq;
//			m >> freq;
//			outputmaster->m_parameterQueue.send(outputmaster->m_parameterQueue.now(),
//												outputmaster, &OutputMaster::setBassManagementFreqTimestamped,
//												(double) freq);
//		} else {
//			std::cerr << "Alloaudio: Wrong type tags for " + outputmaster->m_addressPrefix + "/bass_management_freq: "
//					 << m.typeTags() << std::endl;
//		}
//	} else {
//		std::cout << "Alloaudio: Unrecognized address pattern: " << m.addressPattern() << std::endl;
//	}
//}
