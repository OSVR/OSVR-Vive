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
#include <Settings.h>

// Library/third-party includes
#include <openvr_driver.h>
#include <osvr/Util/Logger.h>

// Standard includes
#include <functional>

// refer to IVRServerDriverHost for details on each function
namespace vr {
class ServerDriverHost : public vr::IVRServerDriverHost {
  public:
    ServerDriverHost();

    /// Sets our "IsExiting()" flag to true.
    void setExiting() { isExiting_ = true; }

    IVRSettings *GetSettings(const char *pchInterfaceVersion) { return vrSettings; }

    virtual bool TrackedDeviceAdded( const char *pchDeviceSerialNumber, 
	                                 ETrackedDeviceClass eDeviceClass, 
									 ITrackedDeviceServerDriver *pDriver );
    std::function<bool(const char *, 
		               ETrackedDeviceClass, 
		               ITrackedDeviceServerDriver *)> onTrackedDeviceAdded;

    virtual void TrackedDevicePoseUpdated(uint32_t unWhichDevice,
                                          const DriverPose_t &newPose,
										  uint32_t unPoseStructSize);

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

    virtual void ProximitySensorState(uint32_t unWhichDevice,
                                      bool bProximitySensorTriggered);

    virtual void VendorSpecificEvent(uint32_t unWhichDevice,
                                     vr::EVREventType eventType,
                                     const VREvent_Data_t &eventData,
                                     double eventTimeOffset);

    virtual bool IsExiting();
	
	virtual bool PollNextEvent( VREvent_t *pEvent, uint32_t uncbVREvent );

    IVRSettings *vrSettings = nullptr;

  private:
    bool isExiting_ = false;
    osvr::util::log::LoggerPtr logger_;
};

} // namespace vr

#endif // INCLUDED_ViveServerDriverHost_h_GUID_CD530D68_1639_42B7_9B06_BA9E59464E9C
