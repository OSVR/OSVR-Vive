/** @file
    @brief Header

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

#ifndef INCLUDED_ServerDriverHost_h_GUID_CD530D68_1639_42B7_9B06_BA9E59464E9C
#define INCLUDED_ServerDriverHost_h_GUID_CD530D68_1639_42B7_9B06_BA9E59464E9C

// Internal Includes
#include <VRSettings.h>

// Library/third-party includes
#include <openvr_driver.h>

// Standard includes
#include <functional>

// refer to IServerDriverHost for details on each function
namespace vr {
class ServerDriverHost : public vr::IServerDriverHost {
  public:
    ServerDriverHost();

    /// Sets our "IsExiting()" flag to true.
    void setExiting() { isExiting_ = true; }

    virtual bool TrackedDeviceAdded(const char *pchDeviceSerialNumber);
    std::function<bool(const char *)> onTrackedDeviceAdded;

    virtual void TrackedDevicePoseUpdated(uint32_t unWhichDevice,
                                          const DriverPose_t &newPose);

    virtual void TrackedDevicePropertiesChanged(uint32_t unWhichDevice);

    virtual void VsyncEvent(double vsyncTimeOffsetSeconds);

    virtual void TrackedDeviceButtonPressed(uint32_t unWhichDevice,
                                            EVRButtonId eButtonId,
                                            double eventTimeOffset);

    virtual void TrackedDeviceButtonUnpressed(uint32_t unWhichDevice,
                                              EVRButtonId eButtonId,
                                              double eventTimeOffset);

    virtual void TrackedDeviceButtonTouched(uint32_t unWhichDevice,
                                            EVRButtonId eButtonId,
                                            double eventTimeOffset);

    virtual void TrackedDeviceButtonUntouched(uint32_t unWhichDevice,
                                              EVRButtonId eButtonId,
                                              double eventTimeOffset);

    virtual void TrackedDeviceAxisUpdated(uint32_t unWhichDevice,
                                          uint32_t unWhichAxis,
                                          const VRControllerAxis_t &axisState);

    virtual void MCImageUpdated();

    virtual IVRSettings *GetSettings(const char *pchInterfaceVersion);

    virtual void PhysicalIpdSet(uint32_t unWhichDevice,
                                float fPhysicalIpdMeters);

    virtual void ProximitySensorState(uint32_t unWhichDevice,
                                      bool bProximitySensorTriggered);

    virtual void VendorSpecificEvent(uint32_t unWhichDevice,
                                     vr::EVREventType eventType,
                                     const VREvent_Data_t &eventData,
                                     double eventTimeOffset);

    virtual bool IsExiting();

    virtual bool PollNextEvent(VREvent_t *pEvent, uint32_t uncbVREvent);

    IVRSettings *vrSettings = nullptr;

  private:
    bool isExiting_ = false;
};

} // namespace vr

#endif // INCLUDED_ViveServerDriverHost_h_GUID_CD530D68_1639_42B7_9B06_BA9E59464E9C
