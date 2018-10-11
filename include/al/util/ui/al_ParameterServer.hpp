#ifndef AL_PARAMETERSERVER_H
#define AL_PARAMETERSERVER_H

/*	Allolib --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
    Copyright (C) 2012-2018. The Regents of the University of California.
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
    Expose parameters on the network
	File author(s):
	Andrés Cabrera mantaraya36@gmail.com
*/

#include "al/core/protocol/al_OSC.hpp"
#include "al/util/ui/al_Parameter.hpp"

namespace al
{

class OSCNotifier {
public:
    OSCNotifier();
    virtual ~OSCNotifier();

    /**
     * @brief addListener enables notifiying via OSC that a preset has changed
     * @param IPaddress The IP address of the listener
     * @param oscPort The network port so send the value changes on
     */
    virtual void addListener(std::string IPaddress, int oscPort) {
        mListenerLock.lock();
        mOSCSenders.push_back(new osc::Send(oscPort, IPaddress.c_str()));
        mListenerLock.unlock();
//		std::cout << "Registered listener " << IPaddress << ":" << oscPort<< std::endl;
    }

    /**
     * @brief Notify the listeners of value changes
     * @param OSCaddress The OSC path to send the value on
     * @param value The value to send
     *
     * This will send all registered data to the listeners. This is useful to
     * force a resfresh of an interface, e.g. when it just came online and is
     * unaware of state. Otherwise, when calling addListener, you should
     * register to be notified when the data changes to only do notifications
     * then.
     *
     */
    void notifyListeners(std::string OSCaddress, float value);

    void notifyListeners(std::string OSCaddress, std::string value);
    void notifyListeners(std::string OSCaddress, Vec3f value);
    void notifyListeners(std::string OSCaddress, Vec4f value);
    void notifyListeners(std::string OSCaddress, Pose value);

protected:
    std::mutex mListenerLock;
    std::vector<osc::Send *> mOSCSenders;
private:
};


/**
 * @brief The ParameterServer class creates an OSC server to receive parameter values
 *
 * Parameter objects that are registered with a ParameterServer will receive
 * incoming messages on their OSC address.
 *
 */
class ParameterServer : public osc::PacketHandler, public OSCNotifier
{
    friend class PresetServer; // To be able to take over the OSC server
public:
    /**
     * @brief ParameterServer constructor
     *
     * @param oscAddress The network address on which to listen to. If empty use all available network interfaces. Defaults to "127.0.0.1".
     * @param oscPort The network port on which to listen. Defaults to 9010.
     *
     * Usage:
     * @code
    Parameter freq("Frequency", "", 440.0);
    Parameter amp("Amplitude", "", 0.1);
    ParameterServer paramServer;
    paramServer << freq << amp;
     @endcode
     */
    // ParameterServer() : mServer(nullptr) {};
    ParameterServer(std::string oscAddress = "", int oscPort = 9010);
    ~ParameterServer();

    /**
     * Open and start receiving osc
     */
    void listen(int oscPort, std::string oscAddress = "");

    /**
     * Register a parameter with the server.
     */
    ParameterServer &registerParameter(Parameter &param);

    /**
     * Remove a parameter from the server.
     */
    void unregisterParameter(Parameter &param);

    /**
     * Register a string parameter with the server.
     */
    ParameterServer &registerParameter(ParameterString &param);

    /**
     * Remove a string parameter from the server.
     */
    void unregisterParameter(ParameterString &param);

    /**
     * Register a Vec3 parameter with the server.
     */
    ParameterServer &registerParameter(ParameterVec3 &param);

    /**
     * Remove a Vec3 parameter from the server.
     */
    void unregisterParameter(ParameterVec3 &param);

    /**
     * Register a Vec4 parameter with the server.
     */
    ParameterServer &registerParameter(ParameterVec4 &param);

    /**
     * Remove a Vec4 parameter from the server.
     */
    void unregisterParameter(ParameterVec4 &param);

    /**
     * Register a Pose parameter with the server.
     */
    ParameterServer &registerParameter(ParameterPose &param);

    /**
     * Remove a Vec4 parameter from the server.
     */
    void unregisterParameter(ParameterPose &param);

    /**
     * @brief print prints information about the server to std::out
     *
     * The print function will print the server configuration (address and port)
     * and will list the parameters with their addresses.
     */
    void print();

    /**
     * @brief stopServer stops the OSC server thread. Calling this function
     * is sometimes required when this object is destroyed abruptly and the
     * destructor is not called
     */
    void stopServer();

    bool serverRunning() { return (mServer != nullptr); }

    /**
     * @brief Get the list of registered parameters.
     */
    std::vector<Parameter *> parameters() {return mParameters;}
    std::vector<ParameterString *> stringParameters() {return mStringParameters;}
    std::vector<ParameterVec3 *> vec3Parameters() {return mVec3Parameters;}
    std::vector<ParameterVec4 *> vec4Parameters() {return mVec4Parameters;}
    std::vector<ParameterPose *> poseParameters() {return mPoseParameters;}

    /// Register parameter using the streaming operator
    template<class ParameterType>
    ParameterServer &operator << (ParameterType& newParam){ return registerParameter(newParam); }

    /// Register parameter using the streaming operator
    template<class ParameterType>
    ParameterServer &operator << (ParameterType* newParam){ return registerParameter(*newParam); }

    /**
     * @brief Append a listener to the osc server.
     * @param handler
     * OSC messages received by this server will be forwarded to all
     * registered listeners. This is the mechanism internally used to share a
     * network port between a ParameterServer, a PresetServer and a SequenceServer
     */
    void registerOSCListener(osc::PacketHandler *handler);

    void notifyAll();

        /**
         * @brief send all currently regeistered parameter values
         * @param IPaddress
         * @param oscPort
         */
        void sendAllParameters(std::string IPaddress, int oscPort);

    virtual void onMessage(osc::Message& m);

        uint16_t serverPort() {return mServer->port();}

        void verbose(bool verbose= true) { mVerbose = verbose;}

protected:
    static void changeCallback(float value, void *sender, void *userData, void *blockThis);
    static void changeStringCallback(std::string value, void *sender, void *userData, void *blockThis);
    static void changeVec3Callback(Vec3f value, void *sender, void *userData, void *blockThis);
    static void changeVec4Callback(Vec4f value, void *sender, void *userData, void *blockThis);
    static void changePoseCallback(Pose value, void *sender, void *userData, void *blockThis);

protected:
    std::vector<osc::PacketHandler *> mPacketHandlers;
    osc::Recv *mServer;
    std::vector<Parameter *> mParameters;
    std::vector<ParameterString *> mStringParameters;
    std::vector<ParameterVec3 *> mVec3Parameters;
    std::vector<ParameterVec4 *> mVec4Parameters;
    std::vector<ParameterPose *> mPoseParameters;
    std::mutex mParameterLock;
    bool mVerbose {false};
};

}


#endif // AL_PARAMETERSERVER_H
