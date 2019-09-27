#ifndef INCLUDE_AL_SOUND_SPEAKER_HPP
#define INCLUDE_AL_SOUND_SPEAKER_HPP

/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012. The Regents of the University of California.
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
	Abstraction of a loudspeaker used for sound spatialization algorithms

	File author(s):
	Lance Putnam, 2006, putnam.lance@gmail.com
*/

#include <cmath>
#include <vector>
#include "al/math/al_Constants.hpp"
#include "al/math/al_Vec.hpp"

namespace al{

/// Spatial definition of a speaker in a listening space
///
/// @ingroup Sound
class Speaker {
public:

	unsigned int deviceChannel;	///< Index in the output device channels array
    float azimuth;				///< Angle from forward to left vector
    float elevation;			///< Angle from forward-right plane to up vector
    int group;					///< Group identifier
	float radius;				///< Distance from center of listening space
    float gain;					///< Gain of speaker

	/// @param[in] deviceChan		audio device output channel
	/// @param[in] az				azimuth of speaker
	/// @param[in] el				elevation of speaker
	/// @param[in] radius			radius of speaker
	/// @param[in] gain				gain of speaker
    Speaker(unsigned int deviceChan=0, float az=0.f, float el=0.f, int group = 0, float radius=1.f, float gain=1.f)
    :	deviceChannel(deviceChan), azimuth(az), elevation(el), group(group), radius(radius), gain(gain)
	{}

	/// Get position in Cartesian coordinate (in audio space)
	template <class T>
	Speaker& posCart(T * xyz){
		using namespace std;
		float elr = toRad(elevation);
		float azr = toRad(azimuth);
		float cosel = cos(elr);
		xyz[0] = cos(azr) * cosel * radius;
		xyz[1] = sin(azr) * cosel * radius;
		xyz[2] = sin(elr) * radius;
		return *this;
	}


	void posCart2(Vec3d xyz){
		using namespace std;

        radius =  sqrt(float((xyz[0]*xyz[0])+(xyz[1]*xyz[1])+(xyz[2]*xyz[2])));
        float gd = sqrt(float((xyz[0]*xyz[0])+(xyz[1]*xyz[1])));
        elevation = atan2f(float(xyz[2]),gd)*180.f/float(M_PI);
        azimuth = atan2f(float(xyz[0]),float(xyz[1]))*180.f/float(M_PI);

	}

	/// Get position as Cartesian coordinate (in audio space)
	Vec3d vec() const {
		//TODO doxygen style commenting on coordinates like ambisonics
        double cosel = cos(toRad(double(elevation)));
        double x = cos(toRad(double(azimuth))) * cosel * double(radius);
        double y = sin(toRad(double(azimuth))) * cosel * double(radius);
        double z = sin(toRad(double(elevation))) * double(radius);
		//Ryan: the standard conversions assume +z is up, these are correct for allocore

		//        double x = sin(toRad(azimuth)) * cosel * radius;
		//		double y = sin(toRad(elevation)) * radius;
		//        double z = -1*cos(toRad(azimuth)) * cosel * radius;
		return Vec3d(x,y,z);
	}

    /// Get position as Cartesian coordinate (in graphics space)
	Vec3d vecGraphics() const {
		//TODO doxygen style commenting on coordinates like ambisonics
        double cosel = cos(toRad(double(elevation)));
        double x = cos(toRad(double(azimuth))) * cosel * double(radius);
        double y = sin(toRad(double(azimuth))) * cosel * double(radius);
        double z = sin(toRad(double(elevation))) * double(radius);
		//Ryan: the standard conversions assume +z is up, these are correct for allocore

		//        double x = sin(toRad(azimuth)) * cosel * radius;
		//		double y = sin(toRad(elevation)) * radius;
		//        double z = -1*cos(toRad(azimuth)) * cosel * radius;
		return Vec3d(-y,z,-x);
	}

	static double toRad(double d){ return d*M_PI/180.; }
    static float toRad(float d){ return d*float(M_PI)/180.f; }
};



/// A set of speakers
typedef std::vector<Speaker> Speakers;



/// Base class for a configuration of multiple speakers
///
/// @ingroup Sound
class SpeakerLayout{
public:

	/// Get number of speakers
    size_t numSpeakers() const { return speakers().size(); }

	/// Get speaker array
	Speakers& speakers(){ return mSpeakers; }
	const Speakers& speakers() const { return mSpeakers; }

	/// Add speaker
	SpeakerLayout& addSpeaker(const Speaker spkr){
		mSpeakers.push_back(spkr);
		return *this;
	}

protected:
	Speakers mSpeakers;
};



/// Generic layout of N speakers spaced equidistantly in a ring
///
/// @ingroup Sound
template <int N>
class SpeakerRingLayout : public SpeakerLayout{
public:
	/// @param[in] deviceChannelStart	starting index of device channel
	/// @param[in] phase				starting phase of first speaker, in degrees
	/// @param[in] radius				radius of all speakers
	/// @param[in] gain					gain of all speakers
    SpeakerRingLayout(unsigned int deviceChannelStart=0, float phase=0.f, float radius=1.f, float gain=1.f)
	{
		mSpeakers.reserve(N);
        for(unsigned int i=0; i<N; ++i)
            addSpeaker(Speaker(i+deviceChannelStart, 360.f/N*i + phase, 0.f, 0, radius, gain));
	}
};

/// Headset speaker layout
///
/// @ingroup allocore
class HeadsetSpeakerLayout : public SpeakerRingLayout<2>{
public:
    HeadsetSpeakerLayout(unsigned int deviceChannelStart=0, float radius=1.f, float gain=1.f)
	:	SpeakerRingLayout<2>(deviceChannelStart, 90, radius, gain)
	{}
};

/// Stereo speaker layout
///
/// @ingroup Sound
class StereoSpeakerLayout : public SpeakerLayout{
public:
    StereoSpeakerLayout(unsigned int deviceChannelStart=0, float angle=30.f, float distance=1.f, float gain=1.f):
        mLeft(deviceChannelStart, angle, 0, 0, distance, gain),
        mRight(deviceChannelStart + 1, -angle, 0, 0, distance, gain)
	{
		addSpeaker(mLeft);
		addSpeaker(mRight);
	}
private:
	Speaker mLeft;
	Speaker mRight;
};


/// Octophonic ring speaker layout
///
/// @ingroup Sound
typedef SpeakerRingLayout<8> OctalSpeakerLayout;


/// Generic layout of 8 speakers arranged in a cube with listener in the middle
///
/// @ingroup allocore
class CubeLayout : public SpeakerLayout {
public:

    CubeLayout(unsigned int deviceChannelStart=0)
	{
		mSpeakers.reserve(8);
        for(unsigned int i=0; i<4; ++i) {
            addSpeaker(Speaker(i+deviceChannelStart, 45.f + (i * 90), 0));
            addSpeaker(Speaker(4 + i+deviceChannelStart, 45.f + (i * 90), 60, 0, sqrt(5.f)));
		}
	}
};


} // al::
#endif
