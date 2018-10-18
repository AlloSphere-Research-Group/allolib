
#include <algorithm>
#include <cstring>
#include <cctype>

#include "al/util/ui/al_ParameterServer.hpp"

using namespace al;

// OSCNotifier implementation -------------------------------------------------

OSCNotifier::OSCNotifier() {}

OSCNotifier::~OSCNotifier() {
    for(osc::Send *sender: mOSCSenders) {
        delete sender;
    }
}

void OSCNotifier::notifyListeners(std::string OSCaddress, float value)
{
    mListenerLock.lock();
    for(osc::Send *sender: mOSCSenders) {
        sender->send(OSCaddress, value);
//		std::cout << "Notifying " << sender->address() << ":" << sender->port() << " -- " << OSCaddress << std::endl;
    }
    mListenerLock.unlock();
}

void OSCNotifier::notifyListeners(std::string OSCaddress, std::string value)
{
    mListenerLock.lock();
    for(osc::Send *sender: mOSCSenders) {
        sender->send(OSCaddress, value);
//		std::cout << "Notifying " << sender->address() << ":" << sender->port() << " -- " << OSCaddress << std::endl;
    }
    mListenerLock.unlock();
}

void OSCNotifier::notifyListeners(std::string OSCaddress, Vec3f value)
{
    mListenerLock.lock();
    for(osc::Send *sender: mOSCSenders) {
        sender->send(OSCaddress, value[0], value[1], value[2]);
//		std::cout << "Notifying " << sender->address() << ":" << sender->port() << " -- " << OSCaddress << std::endl;
    }
    mListenerLock.unlock();
}

void OSCNotifier::notifyListeners(std::string OSCaddress, Vec4f value)
{
    mListenerLock.lock();
    for(osc::Send *sender: mOSCSenders) {
                sender->send(OSCaddress, value[0], value[1], value[2], value[3]);
//		std::cout << "Notifying " << sender->address() << ":" << sender->port() << " -- " << OSCaddress << std::endl;
    }
    mListenerLock.unlock();
}

void OSCNotifier::notifyListeners(std::string OSCaddress, Pose value)
{
    mListenerLock.lock();
    for(osc::Send *sender: mOSCSenders) {
        sender->send(OSCaddress, (float) value.pos()[0], (float) value.pos()[1], (float) value.pos()[2],
                (float) value.quat().w, (float) value.quat().x, (float) value.quat().y, (float) value.quat().z);
//		std::cout << "Notifying " << sender->address() << ":" << sender->port() << " -- " << OSCaddress << std::endl;
    }
    mListenerLock.unlock();
}

// ParameterServer ------------------------------------------------------------

ParameterServer::ParameterServer(std::string oscAddress, int oscPort)
    : mServer(nullptr)
{
    listen(oscPort, oscAddress);
}

ParameterServer::~ParameterServer()
{
//	std::cout << "~ParameterServer()" << std::endl;
    if (mServer) {
        mServer->stop();
        delete mServer;
        mServer = nullptr;
    }
}

void ParameterServer::listen(int oscPort, std::string oscAddress)
{
    if (mServer) {
        mServer->stop();
        delete mServer;
        mServer = nullptr;
    }
    mServer = new osc::Recv(oscPort, oscAddress.c_str(), 0.001); // Is 1ms wait OK?
    if (mServer) {
        mServer->handler(*this);
        mServer->start();
    } else {
        std::cout << "Error starting OSC server." << std::endl;
    }
}

ParameterServer &ParameterServer::registerParameter(Parameter &param)
{
    mParameterLock.lock();
    mParameters.push_back(&param);
    mParameterLock.unlock();
    mListenerLock.lock();
    param.registerChangeCallback([&](float value){
        notifyListeners(param.getFullAddress(), value);
    });
    mListenerLock.unlock();
    return *this;
}

void ParameterServer::unregisterParameter(Parameter &param)
{
    std::unique_lock<std::mutex> lk(mParameterLock);
    std::vector<Parameter *>::iterator it = mParameters.begin();
    for(it = mParameters.begin(); it != mParameters.end(); it++) {
        if (*it == &param) {
            mParameters.erase(it);
        }
    }
}

ParameterServer &ParameterServer::registerParameter(ParameterString &param)
{
    mParameterLock.lock();
    mStringParameters.push_back(&param);
    mParameterLock.unlock();
    mListenerLock.lock();
    param.registerChangeCallback([&](std::string value){
        notifyListeners(param.getFullAddress(), value);
    });
    mListenerLock.unlock();
    return *this;
}

void ParameterServer::unregisterParameter(ParameterString &param)
{
    mParameterLock.lock();
    auto it = mStringParameters.begin();
    for(it = mStringParameters.begin(); it != mStringParameters.end(); it++) {
        if (*it == &param) {
            mStringParameters.erase(it);
        }
    }
    mParameterLock.unlock();
}

ParameterServer &ParameterServer::registerParameter(ParameterVec3 &param)
{
    mParameterLock.lock();
    mVec3Parameters.push_back(&param);
    mParameterLock.unlock();
    mListenerLock.lock();
    param.registerChangeCallback([&](Vec3f value){
        notifyListeners(param.getFullAddress(), value);
    });
    mListenerLock.unlock();
    return *this;
}

void ParameterServer::unregisterParameter(ParameterVec3 &param)
{
    std::unique_lock<std::mutex> lk(mParameterLock);
    auto it = mVec3Parameters.begin();
    for(it = mVec3Parameters.begin(); it != mVec3Parameters.end(); it++) {
        if (*it == &param) {
            mVec3Parameters.erase(it);
        }
    }
    mParameterLock.unlock();
}

ParameterServer &ParameterServer::registerParameter(ParameterPose &param)
{
    mParameterLock.lock();
    mPoseParameters.push_back(&param);
    mParameterLock.unlock();
    mListenerLock.lock();
    param.registerChangeCallback([&](Pose value){
        notifyListeners(param.getFullAddress(), value);
    });
    mListenerLock.unlock();
    return *this;

}

void ParameterServer::unregisterParameter(ParameterPose &param)
{
    std::unique_lock<std::mutex> lk(mParameterLock);
    auto it = mPoseParameters.begin();
    for(it = mPoseParameters.begin(); it != mPoseParameters.end(); it++) {
        if (*it == &param) {
            mPoseParameters.erase(it);
        }
    }
}

ParameterServer &ParameterServer::registerParameterBundle(ParameterBundle &bundle)
{
    if (mCurrentActiveBundle.find(bundle.name()) == mCurrentActiveBundle.end()) {
        mParameterBundles[bundle.name()] = std::vector<ParameterBundle *>();
        mCurrentActiveBundle[bundle.name()] = 0;
    }
    mParameterBundles[bundle.name()].push_back(&bundle);
    return *this;
}

void ParameterServer::onMessage(osc::Message &m)
{
    std::string requestAddress = "/request";
    m.resetStream(); // Needs to be moved to caller...
    if(m.addressPattern() == requestAddress) {
        if (m.typeTags() == "si") {
            std::string address;
            int port;
            m >> address >> port;
            sendAllParameters(address, port);
        } else if (m.typeTags() == "i") {
            int port;
            m >> port;
            sendAllParameters(m.senderAddress(), port);
        }
    }
        if (mVerbose) {
            m.print();
        }
    mParameterLock.lock();
        for (Parameter *p:mParameters) {
        if(m.addressPattern() == p->getFullAddress() && m.typeTags() == "f"){
                    float val;
                    m >> val;
            // Extract the data out of the packet
            p->set(val);
            // std::cout << "ParameterServer::onMessage" << val << std::endl;
        }
    }
    for (ParameterVec3 *p:mVec3Parameters) {
        if(m.addressPattern() == p->getFullAddress() && m.typeTags() == "fff"){
            float x,y,z;
            m >> x >> y >> z;
            p->set(Vec3f(x,y,z));
        }
    }
    for (ParameterVec4 *p:mVec4Parameters) {
        if(m.addressPattern() == p->getFullAddress() && m.typeTags() == "ffff"){
            float a,b,c,d;
            m >> a >> b >> c >> d;
            p->set(Vec4f(a,b,c,d));
        }
    }
    for (ParameterPose *p:mPoseParameters) {
        if(m.addressPattern() == p->getFullAddress() && m.typeTags() == "fffffff"){
            float x, y, z, w, qx, qy, qz;
            m >> x >> y >> z >> w >> qx >> qy >> qz;
            p->set(Pose(Vec3d(x,y,z), Quatd(w, qx, qy, qz)));
        } else if(m.addressPattern() == p->getFullAddress() + "/pos" && m.typeTags() == "fff"){
            float x,y,z;
            m >> x >> y >> z;
            Pose currentPose = p->get();
            currentPose.pos() = Vec3d(x,y,z);
            p->set(currentPose);
        } else if(m.addressPattern() == p->getFullAddress() + "/pos/x" && m.typeTags() == "f"){
            float x;
            m >> x;
            Pose currentPose = p->get();
            currentPose.pos().x = x;
            p->set(currentPose);
        } else if(m.addressPattern() == p->getFullAddress() + "/pos/y" && m.typeTags() == "f"){
            float y;
            m >> y;
            Pose currentPose = p->get();
            currentPose.pos().y = y;
            p->set(currentPose);
        } else if(m.addressPattern() == p->getFullAddress() + "/pos/z" && m.typeTags() == "f"){
            float z;
            m >> z;
            Pose currentPose = p->get();
            currentPose.pos().z= z;
            p->set(currentPose);
        }
    }
    for (auto &bundles:mParameterBundles) {
       auto oscAddress = m.addressPattern();
        std::string bundleName = oscAddress.substr(1, oscAddress.find("/", 1) - 1);
        // TODO there might be a clash if the bundle name matches a parameter name. Should we worry?
        if (bundleName == bundles.first) { // If OSC address starts with bundle name, we are addressing specific instances of the bundle
            std::string bundleIndex = oscAddress.substr(bundleName.size() + 2, oscAddress.find("/", bundleName.size() + 3) - (bundleName.size() + 2));

            auto is_number = [](const std::string& s) {
                return !s.empty() && std::find_if(s.begin(),
                    s.end(), [](char c) { return !std::isdigit(c); }) == s.end(); };
            if (is_number(bundleIndex)) {
                unsigned int index = std::stoul(bundleIndex);
                if (index < bundles.second.size()) {
                    std::string subAddress = oscAddress.substr(bundleName.size() + bundleIndex.size() + 2);
                    for (ParameterMeta *p: bundles.second[index]->parameters()) {
                        if (p->getFullAddress() == subAddress){
                            if (strcmp(typeid(*p).name(), typeid(ParameterBool).name() ) == 0) { // ParameterBool
                                float value;
                                m >> value;
                                static_cast<ParameterBool *>(p)->set(value);
                            } else if (strcmp(typeid(*p).name(), typeid(Parameter).name()) == 0) {// Parameter
                                float value;
                                m >> value;
                                static_cast<Parameter *>(p)->set(value);
                            } else if (strcmp(typeid(*p).name(), typeid(ParameterPose).name()) == 0) {// ParameterPose
                            } else if (strcmp(typeid(*p).name(), typeid(ParameterMenu).name()) == 0) {// ParameterMenu
                            } else if (strcmp(typeid(*p).name(), typeid(ParameterChoice).name()) == 0) {// ParameterChoice
                            } else if (strcmp(typeid(*p).name(), typeid(ParameterVec3).name()) == 0) {// ParameterVec3
                            } else {
//                                 TODO this check should be performed on registration
                                std::cout << "Unsupported Parameter type for bundle from OSC" << std::endl;
                            }
                        }
                    }
                }
            } else if (bundleIndex == "_current") {
                int index = mCurrentActiveBundle[bundleName];
                if (m.typeTags() == "i") {
                    m >> index;
                } else if (m.typeTags() == "f") {
                    float value;
                    m >> value;
                    index = (int) value;
                }
                std::cout << "current " << index << std::endl;
                if (index >= 0 && index < mParameterBundles[bundleName].size()) {
                    mCurrentActiveBundle[bundleName] = index;
                }
            }

        } else { // We will try to pass the values to the current bundle
            std::cout << bundleName << " ... " << mCurrentActiveBundle[bundles.first] << " ;;;; " << mParameterBundles[bundles.first].size() << std::endl;
            for (ParameterMeta *p: mParameterBundles[bundles.first].at(mCurrentActiveBundle[bundles.first])->parameters()) {
                if (p->getFullAddress() == m.addressPattern()){
                    if (strcmp(typeid(*p).name(), typeid(ParameterBool).name() ) == 0) { // ParameterBool
                        float value;
                        m >> value;
                        static_cast<ParameterBool *>(p)->set(value);
                    } else if (strcmp(typeid(*p).name(), typeid(Parameter).name()) == 0) {// Parameter
                        float value;
                        m >> value;
                        static_cast<Parameter *>(p)->set(value);
                    } else if (strcmp(typeid(*p).name(), typeid(ParameterPose).name()) == 0) {// ParameterPose
                    } else if (strcmp(typeid(*p).name(), typeid(ParameterMenu).name()) == 0) {// ParameterMenu
                    } else if (strcmp(typeid(*p).name(), typeid(ParameterChoice).name()) == 0) {// ParameterChoice
                    } else if (strcmp(typeid(*p).name(), typeid(ParameterVec3).name()) == 0) {// ParameterVec3
                    } else {
                        //                                 TODO this check should be performed on registration
                        std::cout << "Unsupported Parameter type for bundle from OSC" << std::endl;
                    }
                }
            }
        }
    }
    for (osc::PacketHandler *handler: mPacketHandlers) {
        m.resetStream();
        handler->onMessage(m);
    }
    mParameterLock.unlock();
}

void ParameterServer::print()
{
    std::cout << "Parameter server listening on " << mServer->address()
              << ":" << mServer->port() << std::endl;
    for (Parameter *p:mParameters) {
        std::cout << "Parameter " << p->getName() << " : " <<  p->getFullAddress() << std::endl;
    }
    if (mOSCSenders.size() > 0) {
        std::cout << "Registered listeners: " << std::endl;
        for (auto sender:mOSCSenders) {
            std::cout << sender->address() << ":" << sender->port() << std::endl;
        }
    }
}

void ParameterServer::stopServer()
{
    if (mServer) {
        mServer->stop();
        delete mServer;
        mServer = nullptr;
    }
}

void ParameterServer::registerOSCListener(osc::PacketHandler *handler)
{
    mParameterLock.lock();
    mPacketHandlers.push_back(handler);
    mParameterLock.unlock();
}

void ParameterServer::notifyAll()
{
    for(Parameter *parameter: mParameters) {
        notifyListeners(parameter->getFullAddress(), parameter->get());
        }
    for (ParameterVec3 *p:mVec3Parameters) {
        notifyListeners(p->getFullAddress(), p->get());
    }
    for (ParameterVec4 *p:mVec4Parameters) {
        notifyListeners(p->getFullAddress(), p->get());
    }

}

void ParameterServer::sendAllParameters(std::string IPaddress, int oscPort)
{
    osc::Send sender(oscPort, IPaddress.c_str());
    for(Parameter *parameter: mParameters) {
        sender.send(parameter->getFullAddress(), parameter->get());
    }
    for (ParameterVec3 *p:mVec3Parameters) {
        Vec3f vec = p->get();
        sender.send(p->getFullAddress(), vec.x, vec.y, vec.z);
    }
    for (ParameterVec4 *p:mVec4Parameters) {
        Vec4f vec = p->get();
        sender.send(p->getFullAddress(), vec[0], vec[1], vec[2], vec[3]);
    }
}

void ParameterServer::changeCallback(float value, void *sender, void *userData, void *blockThis)
{
    ParameterServer *server = static_cast<ParameterServer *>(userData);
    Parameter *parameter = static_cast<Parameter *>(sender);
        server->notifyListeners(parameter->getFullAddress(), value);
}

void ParameterServer::changeStringCallback(std::string value, void *sender, void *userData, void *blockThis)
{
    ParameterServer *server = static_cast<ParameterServer *>(userData);
    ParameterString *parameter = static_cast<ParameterString *>(sender);
        server->notifyListeners(parameter->getFullAddress(), value);
}

void ParameterServer::changeVec3Callback(Vec3f value, void *sender, void *userData, void *blockThis)
{
    ParameterServer *server = static_cast<ParameterServer *>(userData);
    ParameterVec3 *parameter = static_cast<ParameterVec3 *>(sender);
        server->notifyListeners(parameter->getFullAddress(), value);
}

void ParameterServer::changeVec4Callback(Vec4f value, void *sender, void *userData, void *blockThis)
{
    ParameterServer *server = static_cast<ParameterServer *>(userData);
    ParameterVec4 *parameter = static_cast<ParameterVec4 *>(sender);
    server->notifyListeners(parameter->getFullAddress(), value);
}

void ParameterServer::changePoseCallback(Pose value, void *sender, void *userData, void *blockThis)
{
    ParameterServer *server = static_cast<ParameterServer *>(userData);
    ParameterPose *parameter = static_cast<ParameterPose *>(sender);
    server->notifyListeners(parameter->getFullAddress(), value);
}
