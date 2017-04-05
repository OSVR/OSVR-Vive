/** @date 2015

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2015 Vuzix Corporation.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Internal Includes
#include "DriverWrapper.h"
#include "InterfaceTraits.h"
#include "OSVRViveTracker.h"
#include "ServerPropertyHelper.h"
#include <osvr/PluginKit/PluginKit.h>
#include <osvr/Util/PlatformConfig.h>

// "com_Sensics_Callback_json.h"

// Library/third-party includes
#include <math.h>
#include <openvr_driver.h>

// Standard includes
#include <chrono>
#include <deque>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

// Anonymous namespace to avoid symbol collision
namespace {

static const auto PREFIX = "[OSVR-Vive] ";

class CallbackDevice {
  public:
    CallbackDevice::CallbackDevice(OSVR_PluginRegContext ctx,
                                   osvr::vive::DriverWrapperPtr inVive,
                                   osvr::vive::DriverHostPtr inHost)
        : m_ctx(ctx), m_startedInSuccess(false),
          m_shouldAttemptDetection(true) {
        m_viveWrapper = std::move(inVive);
        m_inactiveDriverHost = std::move(inHost);

        OSVR_DeviceInitOptions opts = osvrDeviceCreateInitOptions(ctx);
        m_dev.initAsync(ctx, "Callback", opts);
        // need to create json here?
        // m_dev.sendJsonDescriptor(com_Sensics_Callback_json);
        m_dev.registerUpdateCallback(this);
    }

    CallbackDevice::~CallbackDevice() {
        if (m_startedInSuccess) {
            m_driverHost.reset();
        } else {
            unloadTemporaries();
        }
    }

    OSVR_ReturnCode CallbackDevice::update() {
        if (m_startedInSuccess) {
            // alread started, return here
            OSVR_RETURN_SUCCESS;
        }

        if (!m_shouldAttemptDetection) {
            /// We said we shouldn't and wouldn't try again.
            return OSVR_RETURN_FAILURE;
        }

        /// Hand the Vive object off to the OSVR driver.
        auto startResult = finishViveStartup();
        if (startResult) {
            /// and it started up the rest of the way just fine!
            /// We'll keep the driver around!
            std::cout << PREFIX << "Vive driver finished startup successfully!"
                      << std::endl;
            m_startedInSuccess = true;
            return OSVR_RETURN_SUCCESS;
        }

        std::cout << "\n" << PREFIX << "Vive driver startup failed.";
        if (m_shouldAttemptDetection) {
            std::cout << " Unloading to perhaps try again later.";
        }
        std::cout << std::endl;

        return OSVR_RETURN_FAILURE;
    }

    bool CallbackDevice::finishViveStartup() {
        auto startResult =
            m_inactiveDriverHost->start(m_ctx, std::move(*m_viveWrapper));
        m_viveWrapper.reset();
        if (osvr::vive::ViveDriverHost::StartResult::Success == startResult) {
            m_driverHost = std::move(m_inactiveDriverHost);
            return true;
        }
        if (osvr::vive::ViveDriverHost::StartResult::PermanentFailure ==
            startResult) {
            stopAttemptingDetection();
        }
        return false;
    }

    void CallbackDevice::stopAttemptingDetection() {
        std::cerr << PREFIX << "Will not re-attempt detecting Vive."
                  << std::endl;
        m_shouldAttemptDetection = false;
        unloadTemporaries();
        m_driverHost.reset();
    }

    void CallbackDevice::unloadTemporaries() {
        m_viveWrapper.reset();
        m_inactiveDriverHost.reset();
    }

  private:
    /// This is the OSVR driver object, which also serves as the "SteamVR"
    /// driver host. We can only run one Vive at a time.
    osvr::vive::DriverHostPtr m_driverHost;

    /// A Vive object that we hang on to if we don't have a fully-started-up
    /// device, to save time in hardware detection.
    osvr::vive::DriverWrapperPtr m_viveWrapper;

    /// Populated only when we don't have an active driver - we keep it around
    /// so we don't have to re-load the full driver every time we get hit with a
    /// hardware detect request.
    osvr::vive::DriverHostPtr m_inactiveDriverHost;

    bool m_startedInSuccess;

    bool m_shouldAttemptDetection;

    OSVR_PluginRegContext m_ctx;

    osvr::pluginkit::DeviceToken m_dev;
};

class HardwareDetection {

  public:
    HardwareDetection() : m_startedInSuccess(false) {}

    OSVR_ReturnCode operator()(OSVR_PluginRegContext ctx) {
        if (m_startedInSuccess) {
            return OSVR_RETURN_SUCCESS;
        }

        auto vivePtr = startupAndGetVive();
        if (!vivePtr) {
            /// There was trouble in early startup
            return OSVR_RETURN_FAILURE;
        }

        // complete the first stage startup, and now pass
        // the vive and driveHost to callback device to complete
        // the second stage startup
        // create a fake call-back device for device detection
        // this is to replace the unavailable isHMDPresent func
        osvr::pluginkit::registerObjectForDeletion(
            ctx, new CallbackDevice(ctx, std::move(m_viveWrapper),
                                    std::move(m_driverHost)));

        m_startedInSuccess = true;
        return OSVR_RETURN_SUCCESS;
    }

    /// Attempts the first part of startup, if required.
    osvr::vive::DriverWrapper *startupAndGetVive() {
        if (!m_viveWrapper) {
            m_viveWrapper.reset(new osvr::vive::DriverWrapper(&getDriveHost()));

            if (m_viveWrapper->foundDriver()) {
                std::cout << PREFIX << "Found the Vive driver at "
                          << m_viveWrapper->getDriverFileLocation()
                          << std::endl;
            }

            if (!m_viveWrapper->haveDriverLoaded()) {
                std::cout << PREFIX << "Could not open driver." << std::endl;
                m_viveWrapper.reset();
                m_driverHost.reset();
                return nullptr;
            }

            if (m_viveWrapper->foundConfigDirs()) {
                std::cout << PREFIX << "Driver config dir is: "
                          << m_viveWrapper->getDriverConfigDir() << std::endl;
            }

            if (!(*m_viveWrapper)) {
                std::cerr << PREFIX
                          << "Error in first-stage Vive driver startup."
                          << std::endl;
                m_viveWrapper.reset();
                return nullptr;
            }
        }
        return m_viveWrapper.get();
    }

    /// returns a reference because it will never be null.
    /// Creates one if needed.
    osvr::vive::ViveDriverHost &getDriveHost() {
        if (!m_driverHost) {
            m_driverHost.reset(new osvr::vive::ViveDriverHost);
        }
        return *m_driverHost.get();
    }

  private:
    // after first stage startup, we will pass the vive and drivehost
    // to the CallbackDevice to complete the second stage startup.
    osvr::vive::DriverWrapperPtr m_viveWrapper;
    osvr::vive::DriverHostPtr m_driverHost;

    bool m_startedInSuccess;
};
} // namespace

OSVR_PLUGIN(com_osvr_Vive) {
    osvr::pluginkit::PluginContext context(ctx);

    /// Register a detection callback function object.
    context.registerHardwareDetectCallback(new HardwareDetection());

    return OSVR_RETURN_SUCCESS;
}
