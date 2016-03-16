/** @file
    @brief Implementation

    @date 2016

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2016 Sensics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Internal Includes
#include <ServerDriverHost.h>

// Generated JSON header file
#include "com_osvr_Vive_json.h"

// Library/third-party includes
// - none

// Standard includes
#include <iostream>

using namespace vr;

#ifdef DISABLE_LOG_EVENTS
#define LOG_EVENTS(X)                                                          \
    do {                                                                       \
    } while (0)
#else
#define LOG_EVENTS(X)                                                          \
    do {                                                                       \
        std::cout << X << std::endl;                                           \
    } while (0)
#endif

vr::ServerDriverHost::ServerDriverHost() {}

vr::ServerDriverHost::ServerDriverHost(OSVR_PluginRegContext ctx) {
    /// Create the initialization options
    OSVR_DeviceInitOptions opts = osvrDeviceCreateInitOptions(ctx);

    osvrDeviceTrackerConfigure(opts, &m_tracker);

    /// Create the sync device token with the options
    m_dev.initSync(ctx, "Vive", opts);

    /// Send JSON descriptor
    m_dev.sendJsonDescriptor(com_osvr_Vive_json);

    /// Register update callback
    m_dev.registerUpdateCallback(this);
}

OSVR_ReturnCode ServerDriverHost::update() { return OSVR_RETURN_SUCCESS; }

bool ServerDriverHost::TrackedDeviceAdded(const char *pchDeviceSerialNumber) {
    LOG_EVENTS("TrackedDeviceAdded(" << pchDeviceSerialNumber << ")");
    return true;
}

void ServerDriverHost::TrackedDevicePoseUpdated(uint32_t unWhichDevice,
                                                const DriverPose_t &newPose) {

    LOG_EVENTS("TrackedDevicePoseUpdated(" << unWhichDevice << ", newPose)");

    OSVR_TimeValue now;
    osvrTimeValueGetNow(&now);
    if (newPose.poseIsValid) {
        OSVR_PoseState pose;
        pose.translation.data[0] = newPose.vecPosition[0];
        pose.translation.data[1] = newPose.vecPosition[1];
        pose.translation.data[2] = newPose.vecPosition[2];
        pose.rotation.data[0] = newPose.qRotation.w;
        pose.rotation.data[1] = newPose.qRotation.x;
        pose.rotation.data[2] = newPose.qRotation.y;
        pose.rotation.data[3] = newPose.qRotation.z;

        osvrDeviceTrackerSendPoseTimestamped(m_dev, m_tracker, &pose,
                                             unWhichDevice, &now);
    }
}

void ServerDriverHost::TrackedDevicePropertiesChanged(uint32_t unWhichDevice) {
    LOG_EVENTS("TrackedDevicePropertiesChanged(" << unWhichDevice << ")");
}

void ServerDriverHost::VsyncEvent(double vsyncTimeOffsetSeconds) {
    LOG_EVENTS("VsyncEvent(" << vsyncTimeOffsetSeconds << ")");
}

void ServerDriverHost::TrackedDeviceButtonPressed(uint32_t unWhichDevice,
                                                  EVRButtonId eButtonId,
                                                  double eventTimeOffset) {
    LOG_EVENTS("TrackedDeviceButtonPressed("
               << unWhichDevice << ", " << eButtonId << ", " << eventTimeOffset
               << ")");
}

void ServerDriverHost::TrackedDeviceButtonUnpressed(uint32_t unWhichDevice,
                                                    EVRButtonId eButtonId,
                                                    double eventTimeOffset) {

    LOG_EVENTS("TrackedDeviceButtonUnpressed("
               << unWhichDevice << ", " << eButtonId << ", " << eventTimeOffset
               << ")");
}

void ServerDriverHost::TrackedDeviceButtonTouched(uint32_t unWhichDevice,
                                                  EVRButtonId eButtonId,
                                                  double eventTimeOffset) {

    LOG_EVENTS("TrackedDeviceButtonTouched("
               << unWhichDevice << ", " << eButtonId << ", " << eventTimeOffset
               << ")");
}

void ServerDriverHost::TrackedDeviceButtonUntouched(uint32_t unWhichDevice,
                                                    EVRButtonId eButtonId,
                                                    double eventTimeOffset) {

    LOG_EVENTS("TrackedDeviceButtonUntouched("
               << unWhichDevice << ", " << eButtonId << ", " << eventTimeOffset
               << ")");
}

void ServerDriverHost::TrackedDeviceAxisUpdated(
    uint32_t unWhichDevice, uint32_t unWhichAxis,
    const VRControllerAxis_t &axisState) {

    LOG_EVENTS("TrackedDeviceAxisUpdated(" << unWhichDevice << ", "
                                           << unWhichAxis << ", axisState)");
}

void ServerDriverHost::MCImageUpdated() { LOG_EVENTS("MCImageUpdated()"); }

IVRSettings *ServerDriverHost::GetSettings(const char *pchInterfaceVersion) {
    LOG_EVENTS("GetSettings(" << pchInterfaceVersion << ")");

    return m_vrSettings;
}

void ServerDriverHost::PhysicalIpdSet(uint32_t unWhichDevice,
                                      float fPhysicalIpdMeters) {

    LOG_EVENTS("PhysicalIpdSet(" << unWhichDevice << ", " << fPhysicalIpdMeters
                                 << ")");
}

void ServerDriverHost::ProximitySensorState(uint32_t unWhichDevice,
                                            bool bProximitySensorTriggered) {
    LOG_EVENTS("ProximitySensorState(" << unWhichDevice << ", "
                                       << std::boolalpha
                                       << bProximitySensorTriggered << ")");
}

void ServerDriverHost::VendorSpecificEvent(uint32_t unWhichDevice,
                                           vr::EVREventType eventType,
                                           const VREvent_Data_t &eventData,
                                           double eventTimeOffset) {

    LOG_EVENTS("VendorSpecificEvent("
               << unWhichDevice << ", eventType, eventData, " << eventTimeOffset
               << ")");
}

bool ServerDriverHost::IsExiting() {
    LOG_EVENTS("IsExiting()");
    return isExiting_;
}