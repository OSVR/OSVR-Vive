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
#include "json/reader.h"
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
    static const auto BASE_STATIONS_SENSORS = {3, 4};
    static const auto PUCK_SENSOR = 5;

    static const auto NUM_ANALOGS = 7;
    static const auto NUM_BUTTONS = 14;

    /// Analog sensor for the IPD
    static const auto IPD_ANALOG = 0;
    static const auto PROX_SENSOR_BUTTON_OFFSET = 1;

    static const auto PREFIX = "OSVR-Vive";

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
    static const auto TRIGGER_BUTTON_OFFSET = 5;

    /// Offsets from the first button ID for a controller that an analog is
    /// reported.
    static const auto TRACKPAD_X_ANALOG_OFFSET = 0;
    static const auto TRACKPAD_Y_ANALOG_OFFSET = 1;
    static const auto TRIGGER_ANALOG_OFFSET = 2;

    /// Device descriptor values
    static const auto TRACKER_VALUE = "tracker";
    static const auto PUCK_KEY = "pucks";
    static const auto SEMANTIC_KEY = "semantic";

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
          m_universeRotation(Eigen::Quaterniond::Identity()),
          m_logger(osvr::util::log::make_logger(PREFIX)),
          m_puckIdx(PUCK_SENSOR), m_devDescriptor(com_osvr_Vive_json) {}

    ViveDriverHost::StartResult
    ViveDriverHost::start(OSVR_PluginRegContext ctx,
                          osvr::vive::DriverWrapper &&inVive) {
        if (!inVive) {
            m_logger->error(
                "Called ViveDriverHost::start() with an invalid vive object!");
            return StartResult::TemporaryFailure;
        }
        /// Take ownership of the Vive.
        m_vive.reset(new osvr::vive::DriverWrapper(std::move(inVive)));

        /// define the lambda to handle the ServerDriverHost::TrackedDeviceAdded
        auto handleNewDevice = [&](const char *serialNum,
                                   ETrackedDeviceClass eDeviceClass,
                                   ITrackedDeviceServerDriver *pDriver) {
            auto dev = pDriver;
            if (!dev) {
                m_logger->info("null input device");
                return false;
            }
            auto ret = activateDevice(serialNum, dev, eDeviceClass);
            if (!ret) {
                m_logger->error("Device with serial number ")
                    << serialNum << " couldn't be added to the devices vector.";
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

        /// Finish setting up the Vive.
        try {
            if (!m_vive->startServerDeviceProvider()) {
                m_logger->error("Could not start the server device provider in "
                                "the Vive driver. Exiting.");
                return StartResult::TemporaryFailure;
            }
        } catch (CouldNotGetInterface &e) {
            m_logger->error("Caught exception trying to start Vive server "
                            "device provider: ")
                << e.what();
            m_logger->error("SteamVR interface version may have changed, may "
                            "need to be rebuilt against an updated header or "
                            "use an older SteamVR version. Exiting.");
            return StartResult::PermanentFailure;
        }

        /// Check for interface compatibility
        if (DriverWrapper::InterfaceVersionStatus::InterfaceMismatch ==
            m_vive->checkServerDeviceProviderInterfaces()) {
            m_logger->error(
                "SteamVR lighthouse driver requires unavailable/unsupported "
                "SteamVR lighthouse driver requires unavailable/unsupported "
                "interface versions - either too old or too new for this "
                "build. Specifically, the following critical mismaches: ");
            for (auto iface : m_vive->getUnsupportedRequestedInterfaces()) {
                if (isInterfaceNameWeCareAbout(
                        detail::getInterfaceName(iface))) {
                    auto supported =
                        m_vive->getSupportedInterfaceVersions()
                            .findSupportedVersionOfInterface(iface);
                    m_logger->error(" - SteamVR lighthouse: ")
                        << iface << "\t\t OSVR-Vive: " << supported;
                }
            }
            m_logger->error("Cannot continue.\n");
            return StartResult::PermanentFailure;
        }

        /// Power the system up.
        m_vive->serverDevProvider().LeaveStandby();

        /// Reserve ID 0 for the HMD
        m_vive->devices().reserveIds(1);

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
        m_dev.sendJsonDescriptor(m_devDescriptor);

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
                m_logger->info("No HMD attached, but guessed universe from "
                               "sighted base stations...");
                handleUniverseChange(id);
            }
        }
        return OSVR_RETURN_SUCCESS;
    }

    ViveDriverHost::DevIdReturnValue
    ViveDriverHost::activateDevice(const char *serialNumber,
                                   vr::ITrackedDeviceServerDriver *dev,
                                   vr::ETrackedDeviceClass trackedDeviceClass) {
        auto ret = activateDeviceImpl(serialNumber, dev, trackedDeviceClass);
        vr::TrackedDeviceIndex_t idx = ret.value;
        auto mfrProp = getProperty<Props::ManufacturerName>(idx);
        auto modelProp = getProperty<Props::ModelNumber>(idx);
        auto serialProp = getProperty<Props::SerialNumber>(idx);
        std::string os;
        if (ret) {
            os = "Assigned sensor ID " + std::to_string(ret.value) + " to ";
        } else {
            os = "Could not assign a sensor ID to ";
        }
        os += mfrProp.first + " " + modelProp.first + " " + serialProp.first;
        m_logger->info(os.c_str());
        return ret;
    }

    ViveDriverHost::DevIdReturnValue ViveDriverHost::activateDeviceImpl(
        const char *serialNumber, vr::ITrackedDeviceServerDriver *dev,
        vr::ETrackedDeviceClass trackedDeviceClass) {
        auto &devs = m_vive->devices();
        if (getComponent<vr::IVRDisplayComponent>(dev)) {
            /// This is the HMD, since it has the display component.
            /// Always sensor 0.
            return devs.addAndActivateDeviceAt(dev, HMD_SENSOR);
        }

/// todo need to replace implementation on how to identify
/// controllers
#if 0
        if (getComponent<vr::IVRControllerComponent>(dev)) {
#endif
        /// This is a controller.... or a puck!
        if (trackedDeviceClass ==
            vr::ETrackedDeviceClass::TrackedDeviceClass_GenericTracker) {
            /// find the next available device id because number of pucks
            /// is dynamic
            while (devs.hasDeviceAt(m_puckIdx)) {
                m_puckIdx++;
            }
            auto ret = devs.addAndActivateDeviceAt(dev, m_puckIdx);
            // need to update device descriptor
            AddDeviceToDevDescriptor(serialNumber, m_puckIdx);
            DeviceDescriptorUpdated();
            m_puckIdx++;
            return ret;
        }
        // controllers
        for (auto ctrlIdx : CONTROLLER_SENSORS) {
            if (!devs.hasDeviceAt(ctrlIdx)) {
                return devs.addAndActivateDeviceAt(dev, ctrlIdx);
            }
        }
#if 0
        }
#endif
        /// this is a base station
        if (trackedDeviceClass ==
            vr::ETrackedDeviceClass::TrackedDeviceClass_TrackingReference) {
            for (auto ctrlIdx : BASE_STATIONS_SENSORS) {
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
            m_trackingResults.resize(sensor + 1,
                                     vr::TrackingResult_Uninitialized);
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
            m_logger->info(
                "Got loss of universe ID (Change of universe ID from ")
                << m_universeId << " to " << newUniverse
                << ") but will continue using existing transforms for optimum "
                   "reliability.";
            return;
        }
        m_logger->info() << "Change of universe ID from " << m_universeId
                         << " to " << newUniverse;
        m_universeId = newUniverse;
        auto known = m_vive->chaperone().knowUniverseId(m_universeId);
        if (!known) {
            m_logger->info("No usable information on this universe could be "
                           "found - there may not be a calibration for it in "
                           "your room setup. You may wish to complete that "
                           "then start the OSVR server again. Will operate "
                           "without universe transforms.");
            m_universeXform.setIdentity();
            m_universeRotation.setIdentity();
        }

        /// Fetch the data
        auto univData = m_vive->chaperone().getDataForUniverse(m_universeId);
        if (univData.type == osvr::vive::CalibrationType::Seated) {
            m_logger->info("Only a seated calibration for this universe ID "
                           "exists: y=0 will not be at floor level.");
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
                                                  const DriverPose_t &newPose,
                                                  uint32_t unPoseStructSize) {
        submitTrackingReport(unWhichDevice, osvr::util::time::getNow(),
                             newPose);
    }

/// todo new to get proximity state
#if 0
    void ViveDriverHost::ProximitySensorState(uint32_t unWhichDevice,
                                              bool bProximitySensorTriggered) {
        if (unWhichDevice != 0) {
            return;
        }
        submitButton(PROX_SENSOR_BUTTON_OFFSET, bProximitySensorTriggered, 0);
    }
#endif

    std::pair<vr::ITrackedDeviceServerDriver *, bool>
    ViveDriverHost::getDriverPtr(uint32_t unWhichDevice) {
        if (m_vive->devices().hasDeviceAt(unWhichDevice)) {
            return std::make_pair(&(m_vive->devices().getDevice(unWhichDevice)),
                                  true);
        }
        return std::make_pair(nullptr, false);
    }

    // this is not called anywhere when TrackedDevicePropertiesChanged is gone
    void ViveDriverHost::getUniverseUpdateFromDevice(uint32_t unWhichDevice) {
        auto devRet = getDriverPtr(unWhichDevice);
        auto dev = devRet.first;
        if (!dev) {
            // couldn't get the device
            return;
        }

        vr::ETrackedPropertyError err;
        uint64_t universe = 0;
        std::tie(universe, err) =
            getProperty<Props::CurrentUniverseId>(unWhichDevice);
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
            m_logger->info("Error: TrackedProp_WrongDeviceClass when getting "
                           "universe ID from ")
                << unWhichDevice;
            return;
            break;
        case vr::TrackedProp_ValueNotProvidedByDevice:
            /// OK, that's realistic. We'll just not update the universe based
            /// on it.
            m_logger->info("Error: TrackedProp_ValueNotProvidedByDevice when "
                           "getting universe ID from ")
                << unWhichDevice;
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
                m_logger->info("Error: TrackedProp_NotYetAvailable when "
                               "getting universe ID from")
                    << unWhichDevice;
            }
            break;
        default:
            m_logger->info(
                "Got unrecognized error  when getting universe ID from ")
                << unWhichDevice;
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

    /// todo add new implementation to handle DriverInput
#if 0
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
            submitAnalogs(firstAnalogId + TRACKPAD_X_ANALOG_OFFSET, axisState.x,
                          axisState.y);
            break;
        case 1:
            /// trigger only uses x
            submitAnalog(firstAnalogId + TRIGGER_ANALOG_OFFSET, axisState.x);
            break;
        default:
#if 0
            m_logger->debug() << "other axis [" << unWhichAxis << "] " << axisState.x
                      << ", " << axisState.y << std::endl;
#endif
            break;
        }
    }
#endif
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

    void ViveDriverHost::DeviceDescriptorUpdated() {
        try {
            m_dev.sendJsonDescriptor(m_devDescriptor);
        } catch (std::logic_error &e) {
            /// dev token hasn't been activated yet
            /// we already update device descriptor so it will be sent once
            /// token is activated
            m_logger->info(e.what());
        }
    }

    void ViveDriverHost::AddDeviceToDevDescriptor(const char *serialNumber,
                                                  uint32_t deviceIndex) {

        m_logger->debug("AddDeviceToDevDescriptor, serialNumber: ")
            << std::string(serialNumber);

        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(m_devDescriptor, root)) {
            m_logger->error("Could not parse device descriptor");
        } else {
            std::ostringstream trackerVal;
            trackerVal << TRACKER_VALUE << "/" << deviceIndex;
            std::string puckAlias = serialNumber;
            /// in case serial number is not available
            if (puckAlias.empty()) {
                m_logger->info("Serial number not available. Using device "
                               "index to enumerate Vive Tracker (Puck) in "
                               "device descriptor");
                uint32_t puckIdx = deviceIndex - PUCK_SENSOR;
                puckAlias = std::to_string(puckIdx);
            }
            root[SEMANTIC_KEY][PUCK_KEY][puckAlias] = trackerVal.str();
            m_logger->info("Added Vive Tracker (Puck) : "
                           "/com_osvr_Vive/Vive/semantic/pucks/")
                << puckAlias << " -> /com_osvr_Vive/Vive/" << trackerVal.str();
            m_devDescriptor = root.toStyledString();
        }
    }

} // namespace vive
} // namespace osvr
