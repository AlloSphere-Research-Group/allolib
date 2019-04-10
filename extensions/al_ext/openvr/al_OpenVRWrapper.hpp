#ifndef INCLUDE_OPENVRWRAPPER_HPP
#define INCLUDE_OPENVRWRAPPER_HPP

#include <cmath>
#include <iostream>
#include "al/core.hpp"

#include "al/core/graphics/al_Graphics.hpp"

#include "al/core/graphics/al_VAOMesh.hpp"
#include "al/core/graphics/al_EasyFBO.hpp"
#include "al/util/al_Ray.hpp"

#include <openvr.h>

namespace al {
    
// TO DO: buttonPress event may sometimes crash the program for unknown reason.

struct Controller{
public:
    int deviceID;
    al::Mat4f mat;
    al::Vec3f pos;
    al::Quatf quat;
    al::Vec3f lpos;
    al::Vec3f vel;

    uint64_t buttonsDown;
    uint64_t buttonsLE;
    uint64_t buttonsTE;
    // bool buttonsPressed; //buttons mean any button
    // bool buttonsTouched;
    bool touchpadPressed;
    bool touchpadTouched;
    bool triggerPressed;
    bool triggerTouched;
    bool systemPressed;
    al::Vec2f touchPos;
    al::Vec2f touchVel;
    bool triggered; // triggered has a customizable threshold
    float triggerThreshold;
    float triggerPressure;
    bool gripped;
    
    Controller(){
        deviceID = -1;
        // buttonsPressed = false;
        // buttonsTouched = false;
        touchpadTouched = false;
        touchpadPressed = false;
        triggerPressed = false;
        triggerTouched = false;
        triggerThreshold = 0.9f; // 0.0 - 1.0f
        systemPressed= false;
        triggered = false;
        gripped = false;
    }

    bool buttonDown(int b){return (buttonsDown & (uint64_t(1) << b)) != 0; }
    bool buttonPress(int b){ return (buttonsLE & (uint64_t(1) << b)) != 0; }
    bool buttonRelease(int b){ return (buttonsTE & (uint64_t(1) << b)) != 0; }
    bool triggerDown(){ return buttonDown(33); }
    bool triggerPress(){ return buttonPress(33); }
    bool triggerRelease(){ return buttonRelease(33); }
    bool touchpadDown(){ return buttonDown(32); }
    bool touchpadPress(){ return buttonPress(32); }
    bool touchpadRelease(){ return buttonRelease(32); }
    bool gripDown(){ return buttonDown(2); }
    bool gripPress(){ return buttonPress(2); }
    bool gripRelease(){ return buttonRelease(2); }
    
    bool Triggered(float threshold = 0.9f){
        triggerThreshold = threshold;
        return triggered;
    }

    Rayd ray(){ return Rayd(pos, -quat.toVectorZ()); }
    Pose pose(){ return Pose(pos,quat); }
    
};


class OpenVRWrapper
{
public:
    bool init();
    bool update();
    void draw(std::function<void(Graphics &)> drawingFunction, Graphics &g);
    void close();

    Controller LeftController;
    Controller RightController;
    
    al::Mat4f HMDPose;
    al::Mat4f projectionLeft;
    al::Mat4f projectionRight;
    al::Mat4f eyePosLeft;
    al::Mat4f eyePosRight;

protected:

    vr::IVRSystem* vr_context;
    vr::TrackedDevicePose_t tracked_device_pose[vr::k_unMaxTrackedDeviceCount];
    std::string driver_name, driver_serial;
    std::string tracked_device_type[vr::k_unMaxTrackedDeviceCount];
    int m_iValidPoseCount = 0;
    std::string m_strPoseClasses;
    al::Mat4f m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];
    char m_rDevClassChar[vr::k_unMaxTrackedDeviceCount];   // for each device, a character representing its class
    float m_fNearClip = 0.01, m_fFarClip = 100;
    al::EasyFBO fboLeft;
    al::EasyFBO fboRight;

    //region - Mengyu ->controller matrix
    int numTrackedControllers = 2;
    // al::Mat4f RightHandPose;
    // al::Mat4f LeftHandPose;
    // int LeftControllerID = -1;
    // int RightControllerID = -1;
    vr::TrackedDeviceIndex_t LeftHand_index;
    vr::TrackedDeviceIndex_t RightHand_index;
    // al::Vec3f LeftHandPos;
    // al::Vec3f RightHandPos;
    // al::Quatf LeftHandQuat;
    // al::Quatf RightHandQuat;
    al::Vec3f HMDPos;
    al::Quatf HMDQuat;


    std::map<int, Controller *> controllers;

    //optional:
    uint64_t m_ulOverlayHandle;
    uint64_t m_ulOverlayThumbnailHandle;

    //end region

    void drawVREye(std::function<void(Graphics &)> drawingFunction, Graphics &g, EasyFBO& fbo, Mat4f& view, Mat4f& proj);;

    void finishDraw(al::Graphics &g);

    static al::Mat4f ConvertSteamVRMatrixToAlMat4f(const vr::HmdMatrix44_t &matPose);
    static al::Mat4f ConvertSteamVRMatrixToAlMat4f(const vr::HmdMatrix34_t &matPose);
    static al::Mat4f GetHMDMatrixProjectionEye(vr::IVRSystem* vrsys, vr::Hmd_Eye nEye,
                                               float nearClip, float farClip);

    static al::Mat4f GetHMDMatrixPoseEye(vr::IVRSystem* vrsys, vr::Hmd_Eye nEye);

    static std::string GetTrackedDeviceString(vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice,
                                         vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError);

    static std::string GetTrackedDeviceClassString(vr::ETrackedDeviceClass td_class);


    // void process_overlay_event(const vr::VREvent_t & event){
    //     switch( event.eventType ){
           
    //     }
    // }

    void process_vr_event(const vr::VREvent_t & event);

};
}

#endif
