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
    logger_->info("TrackedDeviceAdded(") << pchDeviceSerialNumber << ")";
    if (onTrackedDeviceAdded) {
        return onTrackedDeviceAdded(pchDeviceSerialNumber, eDeviceClass,
                                    pDriver);
    }
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

void ServerDriverHost::GetRawTrackedDevicePoses(
    float fPredictedSecondsFromNow,
    TrackedDevicePose_t *pTrackedDevicePoseArray,
    uint32_t unTrackedDevicePoseArrayCount) {
    logger_->debug("GetRawTrackedDevicePoses(")
        << fPredictedSecondsFromNow << ", " << unTrackedDevicePoseArrayCount
        << ")";
}

void ServerDriverHost::TrackedDeviceDisplayTransformUpdated(
    uint32_t unWhichDevice, HmdMatrix34_t eyeToHeadLeft,
    HmdMatrix34_t eyeToHeadRight) {
    logger_->debug("TrackedDeviceDisplayTransformUpdated(")
        << unWhichDevice << ", eyeToHeadLeft, eyeToHeadRight)";
}
