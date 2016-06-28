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
#include "OSVRViveTracker.h"
#include "DriverWrapper.h"
#include "GetComponent.h"
#include "ServerPropertyHelper.h"

// Generated JSON header file
#include "com_osvr_Vive_json.h"

// Library/third-party includes
#include <boost/assert.hpp>
#include <osvr/Util/EigenCoreGeometry.h>
#include <osvr/Util/EigenInterop.h>
#include <osvr/Util/TimeValue.h>

// Standard includes
#include <array>
#include <chrono>

namespace osvr {
namespace vive {

    namespace ei = osvr::util::eigen_interop;

    /// sensor numbers in SteamVR and for tracking
    static const auto HMD_SENSOR = 0;
    static const auto CONTROLLER_SENSORS = {1, 2};
    static const auto MAX_CONTROLLER_ID = 2;

    static const auto NUM_ANALOGS = 7;
    static const auto NUM_BUTTONS = 14;

    /// Analog sensor for the IPD
    static const auto IPD_ANALOG = 0;
    static const auto PROX_SENSOR_BUTTON_OFFSET = 1;

    static const auto PREFIX = "[OSVR-Vive] ";

    /// For the HMD and two controllers
    static const std::array<uint32_t, 3> FIRST_BUTTON_ID = {0, 2, 8};
    static const std::array<uint32_t, 3> FIRST_ANALOG_ID = {0, 1, 4};

    /// Offsets from the first button ID for a controller that a button is
    /// reported.
    static const auto SYSTEM_BUTTON_OFFSET = 0;
    static const auto MENU_BUTTON_OFFSET = 1;
    static const auto GRIP_BUTTON_OFFSET = 2;
    static const auto TRACKPAD_TOUCH_BUTTON_OFFSET = 3;
    static const auto TRACKPAD_CLICK_BUTTON_OFFSET = 4;
    static const auto TRIGGER_BUTTON_OFFSET = 4;

    /// Offsets from the first button ID for a controller that an analog is
    /// reported.
    static const auto TRACKPAD_X_ANALOG_OFFSET = 0;
    static const auto TRACKPAD_Y_ANALOG_OFFSET = 1;
    static const auto TRIGGER_ANALOG_OFFSET = 2;

    /// Add a util::time::TimeValue and a std::chrono::duration, returning a
    /// TimeValue again.
    ///
    /// Originally from OSVR-Core
    /// plugins/unifiedvideoinertialtracker/ThreadsafeBodyReporting.cpp but it
    /// (or something like it) really belongs in a header of its own somewhere
    template <typename Rep, typename Period>
    inline util::time::TimeValue
    operator+(util::time::TimeValue const &tv,
              std::chrono::duration<Rep, Period> additionalTime) {
        using namespace std::chrono;
        using SecondsDuration = duration<OSVR_TimeValue_Seconds>;
        using USecondsDuration =
            duration<OSVR_TimeValue_Microseconds, std::micro>;
        auto ret = tv;
        auto seconds = duration_cast<SecondsDuration>(additionalTime);
        ret.seconds += seconds.count();
        ret.microseconds +=
            duration_cast<USecondsDuration>(additionalTime - seconds).count();
        osvrTimeValueNormalize(&ret);
        return ret;
    }

    /// Single, centralized routine to apply the various event time offsets - so
    /// if we're wrong about the sign to be applied, we only have to fix it in
    /// one place.
    ///
    /// @todo validate the direction of those offsets.
    inline util::time::TimeValue
    correctTimeByOffset(util::time::TimeValue const &tv,
                        double eventTimeOffset) {
        return tv + std::chrono::duration<double>(eventTimeOffset);
    }

    ViveDriverHost::ViveDriverHost()
        : m_universeXform(Eigen::Isometry3d::Identity()),
          m_universeRotation(Eigen::Quaterniond::Identity()) {}

    ViveDriverHost::StartResult
    ViveDriverHost::start(OSVR_PluginRegContext ctx,
                          osvr::vive::DriverWrapper &&inVive) {
        if (!inVive) {
            std::cerr << PREFIX << "Error: called ViveDriverHost::start() with "
                                   "an invalid vive object!"
                      << std::endl;
            return StartResult::TemporaryFailure;
        }
        /// Take ownership of the Vive.
        m_vive.reset(new osvr::vive::DriverWrapper(std::move(inVive)));

        /// Finish setting up the Vive.
        try {
            if (!m_vive->startServerDeviceProvider()) {
                std::cerr << PREFIX << "Error: could not start the server "
                                       "device provider in the "
                                       "Vive driver. Exiting."
                          << std::endl;
                return StartResult::TemporaryFailure;
            }
        } catch (CouldNotGetInterface &e) {
            std::cerr << PREFIX
                      << "Caught exception trying to start Vive server device "
                         "provider: "
                      << e.what() << std::endl;
            std::cerr << "SteamVR interface version may have "
                         "changed, may need to be rebuilt against an updated "
                         "header or use an older SteamVR version. Exiting."
                      << std::endl;
            return StartResult::PermanentFailure;
        }

        /// Check for interface compatibility
        if (DriverWrapper::InterfaceVersionStatus::InterfaceMismatch ==
            m_vive->checkServerDeviceProviderInterfaces()) {
            std::cerr
                << PREFIX
                << "SteamVR lighthouse driver requires unavailable/unsupported "
                   "interface versions - either too old or too new for "
                   "this build. Specifically, the following critical "
                   "mismatches: "
                << std::endl;
            for (auto iface : m_vive->getUnsupportedRequestedInterfaces()) {
                if (isInterfaceNameWeCareAbout(
                        detail::getInterfaceName(iface))) {
                    auto supported =
                        m_vive->getSupportedInterfaceVersions()
                            .findSupportedVersionOfInterface(iface);
                    std::cerr << PREFIX << " - SteamVR lighthouse: " << iface
                              << "\t\t OSVR-Vive: " << supported << std::endl;
                }
            }
            std::cerr << PREFIX << "Cannot continue.\n" << std::endl;
            return StartResult::PermanentFailure;
        }

        /// Power the system up.
        m_vive->serverDevProvider().LeaveStandby();

        auto handleNewDevice = [&](const char *serialNum) {
            auto dev =
                m_vive->serverDevProvider().FindTrackedDeviceDriver(serialNum);
            if (!dev) {
                /// The only devices we usually can't look up by serial number
                /// seem to be the lighthouse base stations.
                if (serialNum[0] == 'L' && serialNum[1] == 'H' &&
                    serialNum[2] == 'B') {
                    /// This is one for sure.
                    std::cout << PREFIX << "Tracked object " << serialNum
                              << " is a Lighthouse base station" << std::endl;
                    recordBaseStationSerial(serialNum);
                    return true;
                }
                std::cout << PREFIX << "Unrecognized tracked object "
                          << serialNum << std::endl;
                return false;
            }
            auto ret = activateDevice(dev);
            if (!ret) {
                std::cout << PREFIX << "Device with serial number " << serialNum
                          << " couldn't be added to the devices vector."
                          << std::endl;
                return false;
            }
            NewDeviceReport out{std::string{serialNum}, ret.value};
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_newDevices.submitNew(std::move(out), lock);
            }
            return true;
        };

        m_vive->driverHost().onTrackedDeviceAdded = handleNewDevice;

        /// Reserve ID 0 for the HMD
        m_vive->devices().reserveIds(1);

        {
            auto numDevices =
                m_vive->serverDevProvider().GetTrackedDeviceCount();
            std::cout << PREFIX << "Got " << numDevices
                      << " tracked devices at startup" << std::endl;
            for (decltype(numDevices) i = 0; i < numDevices; ++i) {
                auto dev = m_vive->serverDevProvider().GetTrackedDeviceDriver(
                    i, vr::ITrackedDeviceServerDriver_Version);
                activateDevice(dev);
            }
        }

        /// Finish setting this up as an OSVR device.
        /// Create the initialization options
        OSVR_DeviceInitOptions opts = osvrDeviceCreateInitOptions(ctx);

        osvrDeviceTrackerConfigure(opts, &m_tracker);
        osvrDeviceAnalogConfigure(opts, &m_analog, NUM_ANALOGS);
        osvrDeviceButtonConfigure(opts, &m_button, NUM_BUTTONS);

        /// Because the callbacks may not come from the same thread that
        /// calls RunFrame, we need to be careful to not send directly from
        /// those callbacks. We can't use an Async device token because the
        /// waits are too long and they goof up the SteamVR Lighthouse driver.
        m_dev.initSync(ctx, "Vive", opts);

        /// Send JSON descriptor
        m_dev.sendJsonDescriptor(com_osvr_Vive_json);

        /// Register update callback
        m_dev.registerUpdateCallback(this);

        return StartResult::Success;
    }
    inline OSVR_ReturnCode ViveDriverHost::update() {
        m_vive->serverDevProvider().RunFrame();
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            /// Copy a fixed number of reports that have been queued up.
            m_trackingReports.grabItems(lock);
            m_buttonReports.grabItems(lock);
            m_analogReports.grabItems(lock);

        } // unlock
        // Now that we're out of that mutex, we can go ahead and actually send
        // the reports.
        for (auto &out : m_trackingReports.accessWorkItems()) {
            if (out.isUniverseChange) {
                handleUniverseChange(out.newUniverse);
            } else {
                convertAndSendTracker(out.timestamp, out.sensor, out.report);
            }
        }
        // then clear this temporary buffer for next time. (done automatically,
        // but doing it manually here since there will usually be lots of
        // tracking reports.
        m_trackingReports.clearWorkItems();

        // Deal with the button reports.
        for (auto &out : m_buttonReports.accessWorkItems()) {
            osvrDeviceButtonSetValueTimestamped(
                m_dev, m_button,
                out.buttonState ? OSVR_BUTTON_PRESSED : OSVR_BUTTON_NOT_PRESSED,
                out.sensor, &out.timestamp);
        }
        m_buttonReports.clearWorkItems();

        // Deal with analog reports
        for (auto &out : m_analogReports.accessWorkItems()) {
            osvrDeviceAnalogSetValueTimestamped(m_dev, m_analog, out.value,
                                                out.sensor, &out.timestamp);
            if (out.secondValid) {
                osvrDeviceAnalogSetValueTimestamped(m_dev, m_analog, out.value2,
                                                    out.sensor + 1,
                                                    &out.timestamp);
            }
        }
        m_analogReports.clearWorkItems();

        /// Try guessing the universe if we don't have an HMD to actually
        /// provide it.
        if (0 == m_universeId && !m_vive->devices().hasDeviceAt(HMD_SENSOR) &&
            m_gotBaseStation) {
            std::vector<std::string> baseStations;
            {
                std::lock_guard<std::mutex> lock(m_baseStationMutex);
                baseStations = m_baseStationSerials;
            }

            auto id = m_vive->chaperone().guessUniverseIdFromBaseStations(
                baseStations);
            if (0 != id) {
                std::cout << PREFIX << "No HMD attached, but guessed universe "
                                       "from sighted base stations..."
                          << std::endl;
                handleUniverseChange(id);
            }
        }
        return OSVR_RETURN_SUCCESS;
    }

    ViveDriverHost::DevIdReturnValue
    ViveDriverHost::activateDevice(vr::ITrackedDeviceServerDriver *dev) {
        auto ret = activateDeviceImpl(dev);
        auto mfrProp = getProperty<Props::ManufacturerName>(dev);
        auto modelProp = getProperty<Props::ModelNumber>(dev);
        auto serialProp = getProperty<Props::SerialNumber>(dev);
        std::cout << PREFIX;
        if (ret) {
            std::cout << "Assigned sensor ID " << ret.value << " to ";
        } else {
            std::cout << "Could not assign a sensor ID to ";
        }
        std::cout << mfrProp.first << " " << modelProp.first << " "
                  << serialProp.first << std::endl;
        return ret;
    }

    ViveDriverHost::DevIdReturnValue
    ViveDriverHost::activateDeviceImpl(vr::ITrackedDeviceServerDriver *dev) {
        auto &devs = m_vive->devices();
        if (getComponent<vr::IVRDisplayComponent>(dev)) {
            /// This is the HMD, since it has the display component.
            /// Always sensor 0.
            return devs.addAndActivateDeviceAt(dev, HMD_SENSOR);
        }
        if (getComponent<vr::IVRControllerComponent>(dev)) {
            /// This is a controller.
            for (auto ctrlIdx : CONTROLLER_SENSORS) {
                if (!devs.hasDeviceAt(ctrlIdx)) {
                    return devs.addAndActivateDeviceAt(dev, ctrlIdx);
                }
            }
        }
        /// This still may be a controller, if somehow there are more than
        /// 2...
        return devs.addAndActivateDevice(dev);
    }

    std::ostream &ViveDriverHost::msg() const {
        std::cout << PREFIX;
        return std::cout;
    }

    void ViveDriverHost::recordBaseStationSerial(const char *serial) {
        {
            std::lock_guard<std::mutex> lock(m_baseStationMutex);
            m_baseStationSerials.emplace_back(serial);
        }
        m_gotBaseStation = true;
    }

    void ViveDriverHost::submitTrackingReport(uint32_t unWhichDevice,
                                              OSVR_TimeValue const &tv,
                                              const DriverPose_t &newPose) {
        TrackingReport out;
        out.timestamp = tv;
        out.sensor = unWhichDevice;
        out.report = newPose;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_trackingReports.submitNew(std::move(out), lock);
        }
    }

    void ViveDriverHost::submitUniverseChange(std::uint64_t newUniverse) {
        TrackingReport out;
        out.isUniverseChange = true;
        out.newUniverse = newUniverse;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_trackingReports.submitNew(std::move(out), lock);
        }
    }

    void ViveDriverHost::submitButton(OSVR_ChannelCount sensor, bool state,
                                      double eventTimeOffset) {
        ButtonReport out;
        out.timestamp =
            correctTimeByOffset(osvr::util::time::getNow(), eventTimeOffset);
        out.sensor = sensor;
        out.buttonState = state ? OSVR_BUTTON_PRESSED : OSVR_BUTTON_NOT_PRESSED;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_buttonReports.submitNew(std::move(out), lock);
        }
    }

    void ViveDriverHost::submitAnalog(OSVR_ChannelCount sensor, double value) {
        AnalogReport out;
        out.timestamp = osvr::util::time::getNow();
        out.sensor = sensor;
        out.value = value;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_analogReports.submitNew(std::move(out), lock);
        }
    }

    void ViveDriverHost::submitAnalogs(OSVR_ChannelCount sensor, double value1,
                                       double value2) {
        AnalogReport out;
        out.timestamp = osvr::util::time::getNow();
        out.sensor = sensor;
        out.value = value1;
        out.secondValid = true;
        out.value2 = value2;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_analogReports.submitNew(std::move(out), lock);
        }
    }
    static inline const char *
    trackingResultToString(vr::ETrackingResult trackingResult) {
        switch (trackingResult) {
        case vr::TrackingResult_Uninitialized:
            return "Uninitialized";
            break;
        case vr::TrackingResult_Calibrating_InProgress:
            return "Calibrating (In Progress)";
            break;
        case vr::TrackingResult_Calibrating_OutOfRange:
            return "Calibrating (Out of Range)";
            break;
        case vr::TrackingResult_Running_OK:
            return "Running (OK)";
            break;
        case vr::TrackingResult_Running_OutOfRange:
            return "Running (Out of Range)";
            break;
        default:
            assert(0 && "Should not happen!");
            return "Unknown";
            break;
        }
    }
    void ViveDriverHost::convertAndSendTracker(OSVR_TimeValue const &tv,
                                               OSVR_ChannelCount sensor,
                                               const DriverPose_t &newPose) {
        if (!(sensor < m_trackingResults.size())) {
            m_trackingResults.resize(sensor + 1, vr::TrackingResult_Uninitialized);
        }

        if (newPose.result != m_trackingResults[sensor]) {
            msg() << "Sensor " << sensor << " changed status from '"
                  << trackingResultToString(m_trackingResults[sensor])
                  << "' to '" << trackingResultToString(newPose.result) << "'"
                  << std::endl;
            m_trackingResults[sensor] = newPose.result;
        }
        if (!newPose.poseIsValid) {
            /// @todo better handle non-valid states?
            return;
        }
        auto quatFromSteamVR = [](vr::HmdQuaternion_t const &q) {
            return Eigen::Quaterniond(q.w, q.x, q.y, q.z);
        };

        using namespace Eigen;

        auto qRotation = quatFromSteamVR(newPose.qRotation);

        auto driverFromHeadRotation =
            quatFromSteamVR(newPose.qDriverFromHeadRotation);

        Translation3d driverFromHeadTranslation(
            Vector3d::Map(newPose.vecDriverFromHeadTranslation));
        Isometry3d driverFromHead =
            driverFromHeadTranslation * driverFromHeadRotation;
#if 0
        driverFromHead.fromPositionOrientationScale(
            Vector3d::Map(newPose.vecDriverFromHeadTranslation),
            driverFromHeadRotation, Vector3d::Ones());
#endif
        auto worldFromDriverRotation =
            quatFromSteamVR(newPose.qWorldFromDriverRotation);
        Translation3d worldFromDriverTranslation(
            Vector3d::Map(newPose.vecWorldFromDriverTranslation));
        Isometry3d worldFromDriver =
            worldFromDriverTranslation * worldFromDriverRotation;
#if 0
        worldFromDriver.fromPositionOrientationScale(
            Eigen::Vector3d::Map(newPose.vecWorldFromDriverTranslation),
            worldFromDriverRotation, Eigen::Vector3d::Ones());
#endif
#if 0
        ei::map(pose.translation) =
            (worldFromDriverTranslation * worldFromDriverRotation *
                Eigen::Translation3d(Eigen::Vector3d::Map(newPose.vecPosition)) *
                driverFromHeadTranslation * worldFromDriverRotation)
            .translation();
#endif

        OSVR_Pose3 pose;
        ei::map(pose.translation) =
            (m_universeXform * worldFromDriver *
             Eigen::Translation3d(Eigen::Vector3d::Map(newPose.vecPosition)) *
             driverFromHeadTranslation)
                .translation();
        ei::map(pose.rotation) = m_universeRotation * worldFromDriverRotation *
                                 qRotation * driverFromHeadRotation;

        auto correctedTimestamp =
            correctTimeByOffset(tv, newPose.poseTimeOffset);
        osvrDeviceTrackerSendPoseTimestamped(m_dev, m_tracker, &pose, sensor,
                                             &correctedTimestamp);
    }

    void ViveDriverHost::handleUniverseChange(std::uint64_t newUniverse) {
        /// Check to see if it's really a change
        if (newUniverse == m_universeId) {
            return;
        }
        if (m_universeId != 0 && newUniverse == 0) {
            /// These are usually tracking glitches, not actual changes in
            /// tracking universes.
            std::cout << PREFIX
                      << "Got loss of universe ID (Change of universe ID from "
                      << m_universeId << " to " << newUniverse
                      << ") but will continue using existing transforms for "
                         "optimum reliability."
                      << std::endl;
            return;
        }
        std::cout << PREFIX << "Change of universe ID from " << m_universeId
                  << " to " << newUniverse << std::endl;
        m_universeId = newUniverse;
        auto known = m_vive->chaperone().knowUniverseId(m_universeId);
        if (!known) {
            std::cout << PREFIX
                      << "No usable information on this universe "
                         "could be found - there may not be a "
                         "calibration for it in your room setup. You may wish "
                         "to complete that then start the OSVR server again. "
                         "Will operate without universe transforms."
                      << std::endl;
            m_universeXform.setIdentity();
            m_universeRotation.setIdentity();
        }

        /// Fetch the data
        auto univData = m_vive->chaperone().getDataForUniverse(m_universeId);
        if (univData.type == osvr::vive::CalibrationType::Seated) {
            std::cout << PREFIX
                      << "Only a seated calibration for this universe ID "
                         "exists: y=0 will not be at floor level."
                      << std::endl;
        }
        using namespace Eigen;
        /// Populate the transforms.
        m_universeXform =
            Translation3d(Vector3d::Map(univData.translation.data())) *
            AngleAxisd(univData.yaw, Vector3d::UnitY());
        m_universeRotation =
            Quaterniond(AngleAxisd(univData.yaw, Vector3d::UnitY()));
    }

    void ViveDriverHost::TrackedDevicePoseUpdated(uint32_t unWhichDevice,
                                                  const DriverPose_t &newPose) {
        submitTrackingReport(unWhichDevice, osvr::util::time::getNow(),
                             newPose);
    }

    void ViveDriverHost::PhysicalIpdSet(uint32_t unWhichDevice,
                                        float fPhysicalIpdMeters) {
        submitAnalog(IPD_ANALOG, fPhysicalIpdMeters);
    }

    void ViveDriverHost::ProximitySensorState(uint32_t unWhichDevice,
                                              bool bProximitySensorTriggered) {
        if (unWhichDevice != 0) {
            return;
        }
        submitButton(PROX_SENSOR_BUTTON_OFFSET, bProximitySensorTriggered, 0);
    }

    void
    ViveDriverHost::TrackedDevicePropertiesChanged(uint32_t unWhichDevice) {
        bool checkUniverse = false;
        if (HMD_SENSOR == unWhichDevice) {
            checkUniverse = true;
        } else if (!m_vive->devices().hasDeviceAt(HMD_SENSOR)) {
            checkUniverse = true;
        }

        if (!checkUniverse) {
            return;
        }
        getUniverseUpdateFromDevice(unWhichDevice);
    }

    std::pair<vr::ITrackedDeviceServerDriver *, bool>
    ViveDriverHost::getDriverPtr(uint32_t unWhichDevice) {
        // return std::pair<vr::ITrackedDeviceServerDriver *, bool>();
        if (m_vive->devices().hasDeviceAt(unWhichDevice)) {
            return std::make_pair(&(m_vive->devices().getDevice(unWhichDevice)),
                                  true);
        }
        return std::make_pair(
            m_vive->serverDevProvider().GetTrackedDeviceDriver(
                unWhichDevice, vr::ITrackedDeviceServerDriver_Version),
            false);
    }

    void ViveDriverHost::getUniverseUpdateFromDevice(uint32_t unWhichDevice) {
        auto devRet = getDriverPtr(unWhichDevice);
        auto dev = devRet.first;
        if (!dev) {
            // couldn't get the device
            return;
        }

        vr::ETrackedPropertyError err;
        uint64_t universe = 0;
        std::tie(universe, err) = getProperty<Props::CurrentUniverseId>(dev);
        switch (err) {
        case vr::TrackedProp_WrongDataType:
        case vr::TrackedProp_StringExceedsMaximumLength:
        case vr::TrackedProp_BufferTooSmall:
        case vr::TrackedProp_UnknownProperty:
        case vr::TrackedProp_CouldNotContactServer:
            /// should not happen
            BOOST_ASSERT_MSG(false, "These property errors should not happen "
                                    "when trying to retrieve the universe ID.");
            return;
            break;
        case vr::TrackedProp_Success:
            /// Very good, let's carry on.
            break;
        case vr::TrackedProp_WrongDeviceClass:
            /// OK, that's realistic. We'll just not update the universe based
            /// on it.
            std::cout << "error: TrackedProp_WrongDeviceClass when getting "
                         "universe ID from "
                      << unWhichDevice << std::endl;
            return;
            break;
        case vr::TrackedProp_ValueNotProvidedByDevice:
            /// OK, that's realistic. We'll just not update the universe based
            /// on it.
            std::cout << "error: TrackedProp_ValueNotProvidedByDevice when "
                         "getting universe ID from "
                      << unWhichDevice << std::endl;
            return;
            break;
        case vr::TrackedProp_InvalidDevice:
            /// A bit strange that we got a message about it, but in any case,
            /// we can ignore it then.
            return;
            break;
        case vr::TrackedProp_NotYetAvailable:
            if (HMD_SENSOR == unWhichDevice) {
                /// Well, here we want to set the universe to 0.
                universe = 0;
            } else {
                std::cout << "error: TrackedProp_NotYetAvailable when getting "
                             "universe ID from "
                          << unWhichDevice << std::endl;
            }
            break;
        default:
            std::cout << "Got unrecognized error " << err
                      << " when getting universe ID from " << unWhichDevice
                      << std::endl;
            break;
        }

        /// If we're still around at this point, then universe contains
        /// something useful.
        /// Check our thread-local copy of the universe ID before submitting
        /// the message.
        if (m_trackingThreadUniverseId != universe) {
            m_trackingThreadUniverseId = universe;
            submitUniverseChange(universe);
        }
    }

    void ViveDriverHost::TrackedDeviceButtonPressed(uint32_t unWhichDevice,
                                                    EVRButtonId eButtonId,
                                                    double eventTimeOffset) {
        handleTrackedButtonPressUnpress(unWhichDevice, eButtonId,
                                        eventTimeOffset, true);
    }
    void ViveDriverHost::TrackedDeviceButtonUnpressed(uint32_t unWhichDevice,
                                                      EVRButtonId eButtonId,
                                                      double eventTimeOffset) {
        handleTrackedButtonPressUnpress(unWhichDevice, eButtonId,
                                        eventTimeOffset, false);
    }

    void ViveDriverHost::TrackedDeviceButtonTouched(uint32_t unWhichDevice,
                                                    EVRButtonId eButtonId,
                                                    double eventTimeOffset) {
        handleTrackedButtonTouchUntouch(unWhichDevice, eButtonId,
                                        eventTimeOffset, true);
    }

    void ViveDriverHost::TrackedDeviceButtonUntouched(uint32_t unWhichDevice,
                                                      EVRButtonId eButtonId,
                                                      double eventTimeOffset) {
        handleTrackedButtonTouchUntouch(unWhichDevice, eButtonId,
                                        eventTimeOffset, false);
    }

    void ViveDriverHost::TrackedDeviceAxisUpdated(
        uint32_t unWhichDevice, uint32_t unWhichAxis,
        const VRControllerAxis_t &axisState) {
        /// Don't have allocated sensors for controllers above 2.
        if (unWhichDevice > MAX_CONTROLLER_ID) {
            return;
        }
        if (HMD_SENSOR == unWhichDevice) {
            /// no axes on the HMD that we know of.
            return;
        }

        const auto firstAnalogId = FIRST_ANALOG_ID[unWhichDevice];
        switch (unWhichAxis) {
        case 0:
            // std::cout << "touchpad " << axisState.x << ", " << axisState.y <<
            // std::endl;
            submitAnalogs(firstAnalogId + TRACKPAD_X_ANALOG_OFFSET, axisState.x,
                          axisState.y);
            break;
        case 1:
            // std::cout << "trigger " << axisState.x << ", " << axisState.y <<
            // std::endl;
            /// trigger only uses x
            submitAnalog(firstAnalogId + TRIGGER_ANALOG_OFFSET, axisState.x);
            break;
        default:
#if 0
            std::cout << "other axis [" << unWhichAxis << "] " << axisState.x
                      << ", " << axisState.y << std::endl;
#endif
            break;
        }
    }

    void ViveDriverHost::handleTrackedButtonPressUnpress(uint32_t unWhichDevice,
                                                         EVRButtonId eButtonId,
                                                         double eventTimeOffset,
                                                         bool state) {
        /// Don't have allocated sensors for controllers above 2.
        if (unWhichDevice > MAX_CONTROLLER_ID) {
            return;
        }
        if (HMD_SENSOR == unWhichDevice) {
            if (eButtonId == k_EButton_System) {
                submitButton(SYSTEM_BUTTON_OFFSET, state, eventTimeOffset);
            }
            return;
        }

        const auto firstButtonId = FIRST_BUTTON_ID[unWhichDevice];
        switch (eButtonId) {
        case vr::k_EButton_System:
            submitButton(firstButtonId + SYSTEM_BUTTON_OFFSET, state,
                         eventTimeOffset);
            break;
        case vr::k_EButton_ApplicationMenu:
            submitButton(firstButtonId + MENU_BUTTON_OFFSET, state,
                         eventTimeOffset);
            break;
        case vr::k_EButton_Grip:
            submitButton(firstButtonId + GRIP_BUTTON_OFFSET, state,
                         eventTimeOffset);
            break;
        case vr::k_EButton_DPad_Left:
            /// n/a on the Vive controller
            break;
        case vr::k_EButton_DPad_Up:
            /// n/a on the Vive controller
            break;
        case vr::k_EButton_DPad_Right:
            /// n/a on the Vive controller
            break;
        case vr::k_EButton_DPad_Down:
            /// n/a on the Vive controller
            break;
        case vr::k_EButton_A:
            /// n/a on the Vive controller
            break;
        case vr::k_EButton_SteamVR_Touchpad:
            submitButton(firstButtonId + TRACKPAD_CLICK_BUTTON_OFFSET, state,
                         eventTimeOffset);
            break;
        case vr::k_EButton_SteamVR_Trigger:
            submitButton(firstButtonId + TRIGGER_BUTTON_OFFSET, state,
                         eventTimeOffset);
            break;
        default:
            break;
        }
    }
    void ViveDriverHost::handleTrackedButtonTouchUntouch(uint32_t unWhichDevice,
                                                         EVRButtonId eButtonId,
                                                         double eventTimeOffset,
                                                         bool state) {
        /// Don't have allocated sensors for controllers above 2.
        if (unWhichDevice > MAX_CONTROLLER_ID) {
            return;
        }

        if (HMD_SENSOR == unWhichDevice) {
            return;
        }
        /// Only mapping touch/untouch to a button for trackpad.
        if (vr::k_EButton_SteamVR_Touchpad == eButtonId) {
            submitButton(FIRST_BUTTON_ID[unWhichDevice] +
                             TRACKPAD_TOUCH_BUTTON_OFFSET,
                         state, eventTimeOffset);
        }
    }
#if 0
    void
    ViveDriverHost::DeviceDescriptorUpdated(std::string const &json) {

        m_dev.sendJsonDescriptor(json);
    }
#endif

} // namespace vive
} // namespace osvr
