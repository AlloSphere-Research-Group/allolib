
#include <algorithm>
#include <cstring>
#include <cctype>

#include "al/util/ui/al_ParameterServer.hpp"

using namespace al;

// OSCNotifier implementation -------------------------------------------------

OSCNotifier::OSCNotifier() {
    mHandshakeHandler.notifier = this;
}

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

void OSCNotifier::notifyListeners(std::string OSCaddress, int value)
{
    mListenerLock.lock();
    for(osc::Send *sender: mOSCSenders) {
        sender->send(OSCaddress, value);
//        std::cout << "Notifying " << sender->address() << ":" << sender->port() << " -- " << OSCaddress << std::endl;
    }
    mListenerLock.unlock();
}

void OSCNotifier::notifyListeners(std::string OSCaddress, std::string value)
{
    mListenerLock.lock();
    for(osc::Send *sender: mOSCSenders) {
        sender->send(OSCaddress, value);
        std::cout << "Notifying " << sender->address() << ":" << sender->port() << " -- " << OSCaddress << std::endl;
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

void OSCNotifier::notifyListeners(std::string OSCaddress, Color value)
{
    mListenerLock.lock();
    for(osc::Send *sender: mOSCSenders) {
        sender->send(OSCaddress, float(value.r), float(value.g), float(value.b));
//		std::cout << "Notifying " << sender->address() << ":" << sender->port() << " -- " << OSCaddress << std::endl;
    }
    mListenerLock.unlock();
}


void OSCNotifier::notifyListeners(std::string OSCaddress, ParameterMeta *param)
{
    if (strcmp(typeid(*param).name(), typeid(ParameterBool).name() ) == 0) { // ParameterBool
        ParameterBool *p = dynamic_cast<ParameterBool *>(param);
        notifyListeners(OSCaddress, p->get());
    } else if (strcmp(typeid(*param).name(), typeid(Parameter).name()) == 0) {// Parameter
        Parameter *p = dynamic_cast<Parameter *>(param);
        notifyListeners(OSCaddress, p->get());
    } else if (strcmp(typeid(*param).name(), typeid(ParameterString).name()) == 0) {// ParameterString
        ParameterString *p = dynamic_cast<ParameterString *>(param);
        notifyListeners(OSCaddress, p->get());
    } else if (strcmp(typeid(*param).name(), typeid(ParameterPose).name()) == 0) {// ParameterPose
        ParameterPose *p = dynamic_cast<ParameterPose *>(param);
        notifyListeners(OSCaddress, p->get());
    } else if (strcmp(typeid(*param).name(), typeid(ParameterMenu).name()) == 0) {// ParameterMenu
        ParameterMenu *p = dynamic_cast<ParameterMenu *>(param);
        notifyListeners(OSCaddress, p->get());
    } else if (strcmp(typeid(*param).name(), typeid(ParameterChoice).name()) == 0) {// ParameterChoice
        ParameterChoice *p = dynamic_cast<ParameterChoice *>(param);
        notifyListeners(OSCaddress, p->get());
    } else if (strcmp(typeid(*param).name(), typeid(ParameterVec3).name()) == 0) {// ParameterVec3
        ParameterVec3 *p = dynamic_cast<ParameterVec3 *>(param);
        notifyListeners(OSCaddress, p->get());
    } else if (strcmp(typeid(*param).name(), typeid(ParameterVec4).name()) == 0) {// ParameterVec4
        ParameterVec4 *p = dynamic_cast<ParameterVec4 *>(param);
        notifyListeners(OSCaddress, p->get());
    } else if (strcmp(typeid(*param).name(), typeid(ParameterColor).name()) == 0) {// ParameterColor
        ParameterColor *p = dynamic_cast<ParameterColor *>(param);
        notifyListeners(OSCaddress, p->get());
    } else {
        std::cout << "OSCNotifier::notifyListeners Unsupported Parameter type for notification" << std::endl;
    }
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
    mServer = new osc::Recv();
    if (mServer) {
        if (mServer->open(oscPort, oscAddress.c_str())) {
            mServer->handler(*this);
            mServer->start();
        } else {
            delete mServer;
            mServer = nullptr;
        }
    } else {
        std::cout << "Error starting OSC server." << std::endl;
    }
}

ParameterServer &ParameterServer::registerParameter(ParameterMeta &param)
{
    mParameterLock.lock();
    mParameters.push_back(&param);
    mParameterLock.unlock();
    mListenerLock.lock();
    if (strcmp(typeid(param).name(), typeid(ParameterBool).name() ) == 0) { // ParameterBool
        ParameterBool *p = dynamic_cast<ParameterBool *>(&param);
        p->registerChangeCallback([this, p](float value){
            notifyListeners(p->getFullAddress(), value);
        });
    } else if (strcmp(typeid(param).name(), typeid(Parameter).name()) == 0) {// Parameter
        //        std::cout << "Register parameter " << param.getName() << std::endl;
                Parameter *p = dynamic_cast<Parameter *>(&param);
                p->registerChangeCallback([this, p](float value){
                    notifyListeners(p->getFullAddress(), value);
                });
    } else if (strcmp(typeid(param).name(), typeid(ParameterInt).name()) == 0) {// ParameterInt
//        std::cout << "Register parameter " << param.getName() << std::endl;
        ParameterInt *p = dynamic_cast<ParameterInt *>(&param);
        p->registerChangeCallback([this, p](int32_t value){
            notifyListeners(p->getFullAddress(), value);
        });
    } else if (strcmp(typeid(param).name(), typeid(ParameterPose).name()) == 0) {// ParameterPose
        ParameterPose *p = dynamic_cast<ParameterPose *>(&param);
        p->registerChangeCallback([this, p](al::Pose value){
            notifyListeners(p->getFullAddress(), value);
        });
    } else if (strcmp(typeid(param).name(), typeid(ParameterMenu).name()) == 0) {// ParameterMenu
        ParameterMenu *p = dynamic_cast<ParameterMenu *>(&param);
        p->registerChangeCallback([this, p](int value){
            notifyListeners(p->getFullAddress(), value);
        });
    } else if (strcmp(typeid(param).name(), typeid(ParameterChoice).name()) == 0) {// ParameterChoice
        ParameterChoice *p = dynamic_cast<ParameterChoice *>(&param);
        p->registerChangeCallback([this, p](uint16_t value){
            notifyListeners(p->getFullAddress(), (int) value);
        });
    } else if (strcmp(typeid(param).name(), typeid(ParameterVec3).name()) == 0) {// ParameterVec3
        ParameterVec3 *p = dynamic_cast<ParameterVec3 *>(&param);

        p->registerChangeCallback([this, p](al::Vec3f value){
            notifyListeners(p->getFullAddress(), value);
        });
    } else if (strcmp(typeid(param).name(), typeid(ParameterVec4).name()) == 0) {// ParameterVec4
        ParameterVec4 *p = dynamic_cast<ParameterVec4 *>(&param);
        p->registerChangeCallback([this, p](al::Vec4f value){
            notifyListeners(p->getFullAddress(), value);
        });
    } else if (strcmp(typeid(param).name(), typeid(ParameterColor).name()) == 0) {// ParameterColor
        ParameterColor *p = dynamic_cast<ParameterColor *>(&param);

        p->registerChangeCallback([this, p](Color value){
            Vec4f valueVec(value.r, value.g, value.b, value.a);
            notifyListeners(p->getFullAddress(), valueVec);
        });
    } else {
        std::cout << "Unsupported Parameter type for server on registration" << std::endl;
    }

    mListenerLock.unlock();
    return *this;
}

ParameterServer &ParameterServer::registerParameterBundle(ParameterBundle &bundle)
{
    if (mCurrentActiveBundle.find(bundle.name()) == mCurrentActiveBundle.end()) {
        mParameterBundles[bundle.name()] = std::vector<ParameterBundle *>();
        mCurrentActiveBundle[bundle.name()] = 0;
    }
    mParameterBundles[bundle.name()].push_back(&bundle);
    bundle.addNotifier(this);

    return *this;
}

void ParameterServer::unregisterParameter(ParameterMeta &param)
{
    std::unique_lock<std::mutex> lk(mParameterLock);
    auto it = mParameters.begin();
    for(it = mParameters.begin(); it != mParameters.end(); it++) {
        if (*it == &param) {
            mParameters.erase(it);
        }
    }
}

void ParameterServer::onMessage(osc::Message &m)
{
    m.resetStream(); // Needs to be moved to caller...
    if (mVerbose) {
      m.print();
    }
    std::string requestAddress = "/sendAllParameters";
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
        return;
    }
    mParameterLock.lock();
    for(ParameterMeta *param: mParameters) {
        if (setParameterValueFromMessage(param, m.addressPattern(), m)) {
            m.resetStream();
        }
    }
    for (auto &bundleGroup:mParameterBundles) {
        auto oscAddress = m.addressPattern();
        setValuesForBundleGroup(m, bundleGroup.second, oscAddress);
    }
    for (osc::PacketHandler *handler: mPacketHandlers) {
        m.resetStream();
        handler->onMessage(m);
    }
    for (auto consumer: mMessageConsumers) {
        m.resetStream();
        consumer.first->consumeMessage(m, consumer.second);
    }
    mParameterLock.unlock();
}

void ParameterServer::print(std::ostream &stream)
{
    if (!mServer) {
        stream << "Parameter server not running." << std::endl;
        return;
    }
    stream << " ---- Parameter server listening on " << mServer->address()
              << ":" << mServer->port() << std::endl;
    for (ParameterMeta *p:mParameters) {
        printParameterInfo(p);
    }
    for (auto bundleGroup: mParameterBundles) {
        stream << " --- Bundle " << bundleGroup.first << std::endl;
        for (auto bundle: bundleGroup.second) {
            printBundleInfo(bundle, bundleGroup.first);
        }
    }
    if (mOSCSenders.size() > 0) {
        stream << "Registered listeners: " << std::endl;
        for (auto sender:mOSCSenders) {
            stream << sender->address() << ":" << sender->port() << std::endl;
        }
    }
    stream << " ---- " << std::endl;
}

void ParameterServer::stopServer()
{
    if (mServer) {
        mServer->stop();
        delete mServer;
        mServer = nullptr;
    }
}

bool ParameterServer::serverRunning() {
    if (!mServer) {
        return false;
    } else {
        return mServer->isOpen();
    }
}

std::vector<Parameter *> ParameterServer::parameters()
{
    std::vector<Parameter *> params;
    for (auto *p: mParameters) {
        if (strcmp(typeid(*p).name(), typeid(Parameter).name() ) == 0 )
        {
            params.push_back(static_cast<Parameter *>(p));
        }
    }
    return params;
}

std::vector<ParameterString *> ParameterServer::stringParameters()
{
    std::vector<ParameterString *> params;
    for (auto *p: mParameters) {
        if ( (strcmp(typeid(*p).name(), typeid(ParameterString).name() ) == 0)  )
        {
             params.push_back(static_cast<ParameterString *>(p));
        }
    }
    return params;

}

std::vector<ParameterVec3 *> ParameterServer::vec3Parameters()
{
    std::vector<ParameterVec3 *> params;
    for (auto *p: mParameters) {
        if ( (strcmp(typeid(*p).name(), typeid(ParameterVec3).name() ) == 0)  )
        {
             params.push_back(static_cast<ParameterVec3 *>(p));
        }
    }
    return params;
}

std::vector<ParameterVec4 *> ParameterServer::vec4Parameters()
{
    std::vector<ParameterVec4 *> params;
    for (auto *p: mParameters) {
        if ( (strcmp(typeid(*p).name(), typeid(ParameterVec4).name() ) == 0)  )
        {
             params.push_back(static_cast<ParameterVec4 *>(p));
        }
    }
    return params;
}

std::vector<ParameterPose *> ParameterServer::poseParameters()
{
    std::vector<ParameterPose *> params;
    for (auto *p: mParameters) {
        if ( (strcmp(typeid(*p).name(), typeid(ParameterPose).name() ) == 0)  )
        {
             params.push_back(static_cast<ParameterPose *>(p));
        }
    }
    return params;
}

void ParameterServer::registerOSCListener(osc::PacketHandler *handler)
{
    mParameterLock.lock();
    mPacketHandlers.push_back(handler);
    mParameterLock.unlock();
}

void ParameterServer::registerOSCConsumer(osc::MessageConsumer *consumer, std::string rootPath)
{
    mParameterLock.lock();
    mMessageConsumers.push_back({consumer, rootPath});
    mParameterLock.unlock();
}

void ParameterServer::notifyAll()
{
    for(ParameterMeta *param: mParameters) {
        notifyListeners(param->getFullAddress(), param);
    }
    for(auto bundleGroup: mParameterBundles) {
        for (ParameterBundle *bundle: bundleGroup.second) {
            std::string prefix = bundle->bundlePrefix();
            for (ParameterMeta *param: bundle->parameters()) {

                notifyListeners(prefix + param->getFullAddress(), param);
            }
        }
    }
}

void ParameterServer::sendAllParameters(std::string IPaddress, int oscPort)
{
    osc::Send sender(oscPort, IPaddress.c_str());
    for(ParameterMeta *param: mParameters) {
        param->sendValue(sender);
    }
}

void ParameterServer::requestAllParameters(std::string IPaddress, int oscPort)
{
  osc::Send sender(oscPort, IPaddress.c_str());
  sender.send("/sendAllParameters", mServer->address(), mServer->port());
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

bool ParameterServer::setParameterValueFromMessage(ParameterMeta *param, std::string address, osc::Message &m)
{
    if (strcmp(typeid(*param).name(), typeid(ParameterBool).name() ) == 0) { // ParameterBool
        ParameterBool *p = dynamic_cast<ParameterBool *>(param);
        if(address == p->getFullAddress() && m.typeTags() == "f"){
            float val;
            m >> val;
            // Extract the data out of the packet
            p->set(val);
            // std::cout << "ParameterServer::onMessage" << val << std::endl;
            return true;
        }
        // notifyListeners(p->getFullAddress(), p->get());
    } else if (strcmp(typeid(*param).name(), typeid(Parameter).name()) == 0) {// Parameter
        Parameter *p = dynamic_cast<Parameter *>(param);
        if(address == p->getFullAddress() && m.typeTags() == "f"){
            float val;
            m >> val;
            // Extract the data out of the packet
            p->set(val);
            // std::cout << "ParameterServer::onMessage" << val << std::endl;
            return true;
        }
        // notifyListeners(p->getFullAddress(), p->get());
    } else if (strcmp(typeid(*param).name(), typeid(ParameterInt).name()) == 0) {// ParameterInt
        ParameterInt *p = dynamic_cast<ParameterInt *>(param);
        if(address == p->getFullAddress() && m.typeTags() == "i"){
            int32_t val;
            m >> val;
            // Extract the data out of the packet
            p->set(val);
            // std::cout << "ParameterServer::onMessage" << val << std::endl;
            return true;
        }
        // notifyListeners(p->getFullAddress(), p->get());
    } else if (strcmp(typeid(*param).name(), typeid(ParameterString).name()) == 0) {// Parameter
        ParameterString *p = dynamic_cast<ParameterString *>(param);
        if(address == p->getFullAddress() && m.typeTags() == "s"){
            std::string val;
            m >> val;
            // Extract the data out of the packet
            p->set(val);
            // std::cout << "ParameterServer::onMessage" << val << std::endl;
            return true;
        }
        // notifyListeners(p->getFullAddress(), p->get());
    } else if (strcmp(typeid(*param).name(), typeid(ParameterPose).name()) == 0) {// ParameterPose
        ParameterPose *p = dynamic_cast<ParameterPose *>(param);
        if(address == p->getFullAddress() && m.typeTags() == "fffffff"){
            float x, y, z, w, qx, qy, qz;
            m >> x >> y >> z >> w >> qx >> qy >> qz;
            p->set(Pose(Vec3d(x,y,z), Quatd(w, qx, qy, qz)));
            return true;
        } else if(address == p->getFullAddress() + "/pos" && m.typeTags() == "fff"){
            float x,y,z;
            m >> x >> y >> z;
            Pose currentPose = p->get();
            currentPose.pos() = Vec3d(x,y,z);
            p->set(currentPose);
            return true;
        } else if(address == p->getFullAddress() + "/pos/x" && m.typeTags() == "f"){
            float x;
            m >> x;
            Pose currentPose = p->get();
            currentPose.pos().x = x;
            p->set(currentPose);
            return true;
        } else if(address == p->getFullAddress() + "/pos/y" && m.typeTags() == "f"){
            float y;
            m >> y;
            Pose currentPose = p->get();
            currentPose.pos().y = y;
            p->set(currentPose);
            return true;
        } else if(address == p->getFullAddress() + "/pos/z" && m.typeTags() == "f"){
            float z;
            m >> z;
            Pose currentPose = p->get();
            currentPose.pos().z= z;
            p->set(currentPose);
            return true;
        }
    } else if (strcmp(typeid(*param).name(), typeid(ParameterMenu).name()) == 0) {// ParameterMenu
        ParameterMenu *p = dynamic_cast<ParameterMenu *>(param);
        if(address == p->getFullAddress() && m.typeTags() == "i"){
            int value;
            m >> value;
            p->set(value);
            return true;
        }
    } else if (strcmp(typeid(*param).name(), typeid(ParameterChoice).name()) == 0) {// ParameterChoice
        ParameterChoice *p = dynamic_cast<ParameterChoice *>(param);
        if(address == p->getFullAddress() && m.typeTags() == "i"){
            int value;
            m >> value;
            p->set(value);
            return true;
        }
    } else if (strcmp(typeid(*param).name(), typeid(ParameterVec3).name()) == 0) {// ParameterVec3
        ParameterVec3 *p = dynamic_cast<ParameterVec3 *>(param);
        if(address == p->getFullAddress() && m.typeTags() == "fff"){
            float x,y,z;
            m >> x >> y >> z;
            p->set(Vec3f(x,y,z));
            return true;
        }
    } else if (strcmp(typeid(*param).name(), typeid(ParameterVec4).name()) == 0) {// ParameterVec4
        ParameterVec4 *p = dynamic_cast<ParameterVec4 *>(param);
        if(address == p->getFullAddress() && m.typeTags() == "ffff"){
            float a,b,c,d;
            m >> a >> b >> c >> d;
            p->set(Vec4f(a,b,c,d));
        }
        return true;
    } else if (strcmp(typeid(*param).name(), typeid(ParameterColor).name()) == 0) {// ParameterColor
        ParameterColor *p = dynamic_cast<ParameterColor *>(param);
        if(address == p->getFullAddress() && m.typeTags() == "ffff"){
            float a,b,c,d;
            m >> a >> b >> c >> d;
            p->set(Color(a,b,c,d));
            return true;
        }
    } else if (strcmp(typeid(*param).name(), typeid(Trigger).name()) == 0) {// ParameterColor
      Trigger *p = dynamic_cast<Trigger *>(param);
      if(address == p->getFullAddress()){
        if (m.typeTags().size() == 0) {
          p->trigger();
          return true;
        } else if (m.typeTags() == "f") {
          float val;
          m >> val;
          if (val == 1.0) {
            p->trigger();
          }
          return true;
        }
      }
  } else {
        // TODO this check should be performed on registration
        std::cout << "Unsupported registered Parameter on message " << typeid(*param).name() << std::endl;
    }
    return false;
}

void ParameterServer::printParameterInfo(ParameterMeta *p) {
    std::cout << "Parameter " << p->getName() << "(" << p->displayName() << ")"
              << "[" << typeid(*p).name() << "] : " <<  p->getFullAddress() << std::endl;
}

void ParameterServer::printBundleInfo(ParameterBundle *bundle, std::string id, int depth)
{
    for (int i = 0; i < depth; i++) std::cout << "  ";
    std::cout << "--- Bundle: " << bundle->name() << " id: " << id << " index "<< bundle->bundleIndex() << " prefix " << bundle->bundlePrefix() << std::endl;
    depth++;
    for(auto *p: bundle->parameters()) {
        for (int i = 0; i < depth; i++) std::cout << "  ";
        printParameterInfo(p);
    }
    for (auto bundleGroup: bundle->bundles()) {
        printBundleInfo(bundleGroup.second, bundleGroup.first, depth);
    }
    for (int i = 0; i < depth - 1; i++) std::cout << "  ";
    std::cout << "--- End Bundle: " << bundle->name() << " id: " << id << std::endl;
}

void ParameterServer::setValuesForBundleGroup(osc::Message &m, std::vector<ParameterBundle *> bundleGroup, std::string rootAddress) {
    for (auto bundle: bundleGroup) {
        std::string bundlePrefix = bundle->bundlePrefix();
        if (rootAddress.compare(0, bundlePrefix.size(), bundlePrefix) == 0) {
            for (ParameterMeta *p: bundle->parameters()) {
                std::string subAddress = rootAddress.substr(bundlePrefix.size());
                if (setParameterValueFromMessage(p, subAddress, m)) {
                    m.resetStream();
                    //                            std::cout << " match " << p->getFullAddress() <<std::endl;
                    continue;
                }
            }
            for (auto subBundleGroups: bundle->bundles()) {
                setValuesForBundleGroup(m, {subBundleGroups.second}, rootAddress);
            }
        }
    }
}
