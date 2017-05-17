/** @file
    @brief Implementation

    @date 2016

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2016 Razer Inc.
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

// Library/third-party includes
// - none

// Standard includes

using namespace vr;

vr::ServerDriverHost::ServerDriverHost()
    : logger_(osvr::util::log::make_logger("ServerDriverHost")) {}

bool ServerDriverHost::TrackedDeviceAdded(const char *pchDeviceSerialNumber,
                                          ETrackedDeviceClass eDeviceClass,
                                          ITrackedDeviceServerDriver *pDriver) {
    if (onTrackedDeviceAdded) {
        return onTrackedDeviceAdded(pchDeviceSerialNumber, eDeviceClass,
                                    pDriver);
    }
    logger_->info("TrackedDeviceAdded(") << pchDeviceSerialNumber << ")";
    return true;
}

void ServerDriverHost::TrackedDevicePoseUpdated(uint32_t unWhichDevice,
                                                const DriverPose_t &newPose,
                                                uint32_t unPoseStructSize) {
    logger_->debug("TrackedDevicePoseUpdated(") << unWhichDevice << ")";
}

void ServerDriverHost::VsyncEvent(double vsyncTimeOffsetSeconds) {
    logger_->info("VsyncEvent(") << vsyncTimeOffsetSeconds << ")";
}

void ServerDriverHost::TrackedDeviceButtonPressed(uint32_t unWhichDevice,
                                                  EVRButtonId eButtonId,
                                                  double eventTimeOffset) {
    logger_->info("TrackedDeviceButtonPressed(")
        << unWhichDevice << ", " << eButtonId << ", " << eventTimeOffset << ")";
}

void ServerDriverHost::TrackedDeviceButtonUnpressed(uint32_t unWhichDevice,
                                                    EVRButtonId eButtonId,
                                                    double eventTimeOffset) {
    logger_->info("TrackedDeviceButtonUnpressed(")
        << unWhichDevice << ", " << eButtonId << ", " << eventTimeOffset << ")";
}

void ServerDriverHost::TrackedDeviceButtonTouched(uint32_t unWhichDevice,
                                                  EVRButtonId eButtonId,
                                                  double eventTimeOffset) {
    logger_->info("TrackedDeviceButtonTouched(")
        << unWhichDevice << ", " << eButtonId << ", " << eventTimeOffset << ")";
}

void ServerDriverHost::TrackedDeviceButtonUntouched(uint32_t unWhichDevice,
                                                    EVRButtonId eButtonId,
                                                    double eventTimeOffset) {
    logger_->info("TrackedDeviceButtonUntouched(")
        << unWhichDevice << ", " << eButtonId << ", " << eventTimeOffset << ")";
}

void ServerDriverHost::TrackedDeviceAxisUpdated(
    uint32_t unWhichDevice, uint32_t unWhichAxis,
    const VRControllerAxis_t &axisState) {
    logger_->info("TrackedDeviceAxisUpdated(")
        << unWhichDevice << ", " << unWhichAxis << ", axisState)";
}

void ServerDriverHost::ProximitySensorState(uint32_t unWhichDevice,
                                            bool bProximitySensorTriggered) {
    /// gets called 1000/sec from some "main thread" and logging can be pricy.
    logger_->debug("ProximitySensorState(")
        << unWhichDevice << ", " << std::boolalpha << bProximitySensorTriggered
        << ")";
}

void ServerDriverHost::VendorSpecificEvent(uint32_t unWhichDevice,
                                           vr::EVREventType eventType,
                                           const VREvent_Data_t &eventData,
                                           double eventTimeOffset) {
    logger_->info("VendorSpecificEvent(")
        << unWhichDevice << ", eventType, eventData, " << eventTimeOffset
        << ")";
}

bool ServerDriverHost::IsExiting() {
    logger_->info("IsExiting()");
    return isExiting_;
}

bool ServerDriverHost::PollNextEvent(VREvent_t *pEvent, uint32_t uncbVREvent) {
    logger_->debug("PollNextEvent(") << uncbVREvent << ")";
    return false;
}

void ServerDriverHost::GetRawTrackedDevicePoses(float fPredictedSecondsFromNow, TrackedDevicePose_t *pTrackedDevicePoseArray, uint32_t unTrackedDevicePoseArrayCount) {
}
