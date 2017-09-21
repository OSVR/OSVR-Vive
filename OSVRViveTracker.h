
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

#ifndef INCLUDED_OSVRViveTracker_h_GUID_BDA684D2_7F2D_4483_660D_C9D679BB1F67
#define INCLUDED_OSVRViveTracker_h_GUID_BDA684D2_7F2D_4483_660D_C9D679BB1F67

// Internal Includes
#include "QuickProcessingDeque.h"
#include "ReturnValue.h"
#include "ServerDriverHost.h"
#include <osvr/PluginKit/AnalogInterfaceC.h>
#include <osvr/PluginKit/ButtonInterfaceC.h>
#include <osvr/PluginKit/PluginKit.h>
#include <osvr/PluginKit/TrackerInterfaceC.h>
#include <osvr/Util/ClientReportTypesC.h>
#include <osvr/Util/Logger.h>
#include <osvr/Util/TimeValue.h>

// Library/third-party includes
#include <osvr/Util/EigenCoreGeometry.h>

// Standard includes
#include <array>
#include <cstdint>
#include <deque>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

using namespace vr;

namespace osvr {
namespace vive {
    struct TrackingReport {
        bool isUniverseChange = false;
        /// Only valid if isUniverseChange = true
        std::uint64_t newUniverse;

        OSVR_TimeValue timestamp;
        OSVR_ChannelCount sensor;
        vr::DriverPose_t report;
    };

    struct ButtonReport {
        OSVR_TimeValue timestamp;
        OSVR_ChannelCount sensor;
        bool buttonState;
    };

    struct AnalogReport {
        OSVR_TimeValue timestamp;
        OSVR_ChannelCount sensor;
        double value;
        bool secondValid = false;
        double value2;
    };

    struct NewDeviceReport {
        std::string serialNumber;
        std::uint32_t id;
    };

    class DriverWrapper;

    class ViveDriverHost : public ServerDriverHost {
      public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
        ViveDriverHost();

        using DevIdReturnValue = ReturnValue<std::uint32_t, bool>;
        enum class StartResult { Success, TemporaryFailure, PermanentFailure };

        /// @return false if we failed to start up for some reason.
        StartResult start(OSVR_PluginRegContext ctx,
                          osvr::vive::DriverWrapper &&inVive);

        /// Standard OSVR device callback
        OSVR_ReturnCode update();

        /// Called when we get a new device from the SteamVR driver that we need
        /// to activate. Delegates the real work - this just displays
        /// information.
        DevIdReturnValue
        activateDevice(const char *serialNumber,
                       vr::ITrackedDeviceServerDriver *dev,
                       vr::ETrackedDeviceClass trackedDeviceClass);

        /// @name ServerDriverHost overrides - called from a tracker thread (not
        /// the main thread)
        /// @{
        void TrackedDevicePoseUpdated(uint32_t unWhichDevice,
                                      const DriverPose_t &newPose,
                                      uint32_t unPoseStructSize) override;

        void ProximitySensorState(uint32_t unWhichDevice,
                                  bool bProximitySensorTriggered) override;

        void TrackedDeviceButtonPressed(uint32_t unWhichDevice,
                                        EVRButtonId eButtonId,
                                        double eventTimeOffset) override;

        void TrackedDeviceButtonUnpressed(uint32_t unWhichDevice,
                                          EVRButtonId eButtonId,
                                          double eventTimeOffset) override;

        void TrackedDeviceButtonTouched(uint32_t unWhichDevice,
                                        EVRButtonId eButtonId,
                                        double eventTimeOffset) override;

        void TrackedDeviceButtonUntouched(uint32_t unWhichDevice,
                                          EVRButtonId eButtonId,
                                          double eventTimeOffset) override;

        void
        TrackedDeviceAxisUpdated(uint32_t unWhichDevice, uint32_t unWhichAxis,
                                 const VRControllerAxis_t &axisState) override;

        IVRSettings *GetSettings(const char *) { return nullptr; }
        /// @}

        /// Add new Vive Tracker aka Puck to the device descriptor
        /// Called when more than 1 puck is connected
        /// todo Can be expanded to add controllers
        void AddDeviceToDevDescriptor(const char *serialNumber,
                                    uint32_t deviceIndex);

        void DeviceDescriptorUpdated();

      private:
        std::ostream &msg() const;
        /// called from tracker thread, handles locking.
        void recordBaseStationSerial(const char *serial);

        /// Gets a driver pointer - may not be activated, since if it's not
        /// handled by the vive object, we'll go back to the server tracked
        /// driver provider and ask there. The second return value is whether it
        /// is activated.
        std::pair<vr::ITrackedDeviceServerDriver *, bool>
        getDriverPtr(uint32_t unWhichDevice);

        void getUniverseUpdateFromDevice(uint32_t unWhichDevice);

        /// joint logic for TrackedDeviceButtonPressed and
        /// TrackedDeviceButtonUnpressed
        void handleTrackedButtonPressUnpress(uint32_t unWhichDevice,
                                             EVRButtonId eButtonId,
                                             double eventTimeOffset,
                                             bool state);
        /// joint logic for TrackedDeviceButtonTouched and
        /// TrackedDeviceButtonUntouched
        void handleTrackedButtonTouchUntouch(uint32_t unWhichDevice,
                                             EVRButtonId eButtonId,
                                             double eventTimeOffset,
                                             bool state);

        /// Does the real work of adding a new device.
        DevIdReturnValue
        activateDeviceImpl(const char *serialNumber,
                           vr::ITrackedDeviceServerDriver *dev,
                           vr::ETrackedDeviceClass trackedDeviceClass);

        osvr::pluginkit::DeviceToken m_dev;
        OSVR_TrackerDeviceInterface m_tracker;
        OSVR_AnalogDeviceInterface m_analog;
        OSVR_ButtonDeviceInterface m_button;

        std::unique_ptr<osvr::vive::DriverWrapper> m_vive;

        osvr::util::log::LoggerPtr m_logger;

        /// Cached copy of the universe ID only touched from tracking thread
        /// callbacks
        std::uint64_t m_trackingThreadUniverseId = 0;

        /// Can be called from steamvr thread.
        void submitTrackingReport(uint32_t unWhichDevice,
                                  OSVR_TimeValue const &tv,
                                  const DriverPose_t &newPose);

        void submitUniverseChange(std::uint64_t newUniverse);

        void submitButton(OSVR_ChannelCount sensor, bool state,
                          double eventTimeOffset = 0.);

        void submitAnalog(OSVR_ChannelCount sensor, double value);
        /// Submit both axes for a single mutex lock.
        void submitAnalogs(OSVR_ChannelCount sensor, double value1,
                           double value2);

        /// @name Mutex-controlled
        /// @{
        std::mutex m_mutex;
        QuickProcessingDeque<TrackingReport> m_trackingReports;
        QuickProcessingDeque<ButtonReport> m_buttonReports;
        QuickProcessingDeque<AnalogReport> m_analogReports;
        QuickProcessingDeque<NewDeviceReport> m_newDevices;
        /// @}

        bool m_gotBaseStation = false;
        /// @name Base station serials (mutex controlled)
        /// @{
        std::mutex m_baseStationMutex;
        std::vector<std::string> m_baseStationSerials;
        /// @}

        /// @name Main-thread only
        /// @{
        /// Current reports - main thread only
        /// Called from main thread only!
        void convertAndSendTracker(OSVR_TimeValue const &tv,
                                   OSVR_ChannelCount sensor,
                                   const DriverPose_t &newPose);
        void handleUniverseChange(std::uint64_t newUniverse);

        OSVR_PluginRegContext m_ctx;

        std::uint64_t m_universeId = 0;
        Eigen::Isometry3d m_universeXform;
        Eigen::Quaterniond m_universeRotation;
        std::vector<vr::ETrackingResult> m_trackingResults;

        std::uint32_t m_puckIdx;
        std::string m_devDescriptor;
        /// @}
    };
    using DriverHostPtr = std::unique_ptr<ViveDriverHost>;
    using DriverWrapperPtr = std::unique_ptr<DriverWrapper>;

} // namespace vive
} // namespace osvr
#endif // INCLUDED_OSVRViveTracker_h_GUID_BDA684D2_7F2D_4483_660D_C9D679BB1F67
