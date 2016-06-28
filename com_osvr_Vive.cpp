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

class HardwareDetection {

  public:
    HardwareDetection()
        : m_inactiveDriverHost(new osvr::vive::ViveDriverHost) {}
    OSVR_ReturnCode operator()(OSVR_PluginRegContext ctx) {
        if (m_driverHost) {
            // Already found a Vive.
            /// @todo what are the semantics of the return value from a hardware
            /// detect?
            return OSVR_RETURN_SUCCESS;
        }
        if (!m_shouldAttemptDetection) {
            /// We said we shouldn't and wouldn't try again.
            return OSVR_RETURN_FAILURE;
        }

        auto vivePtr = startupAndGetVive();
        if (!vivePtr) {
            /// There was trouble in early startup
            return OSVR_RETURN_FAILURE;
        }

        if (!vivePtr->isHMDPresent()) {
            /// Didn't detect anything - leave the driver DLL loaded,
            /// though, to make things faster next time around.
            /// Silent failure, to avoid annoying users.
            return OSVR_RETURN_FAILURE;
        }

        std::cout << PREFIX << "Vive is connected." << std::endl;

        /// Hand the Vive object off to the OSVR driver.
        auto startResult = finishViveStartup(ctx);
        if (startResult) {
            /// and it started up the rest of the way just fine!
            /// We'll keep the driver around!
            std::cout << PREFIX << "Vive driver finished startup successfully!"
                      << std::endl;
            return OSVR_RETURN_SUCCESS;
        }

        std::cout << "\n" << PREFIX << "Vive driver startup failed.";
        if (m_shouldAttemptDetection) {
            std::cout << " Unloading to perhaps try again later.";
        }
        std::cout << std::endl;

        unloadTemporaries();
        return OSVR_RETURN_FAILURE;
    }

    bool finishViveStartup(OSVR_PluginRegContext ctx) {
        auto startResult =
            m_inactiveDriverHost->start(ctx, std::move(*m_viveWrapper));
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

    /// Attempts the first part of startup, if required.
    osvr::vive::DriverWrapper *startupAndGetVive() {
        if (!m_viveWrapper) {
            m_viveWrapper.reset(
                new osvr::vive::DriverWrapper(&getInactiveHost()));

            if (m_viveWrapper->foundDriver()) {
                std::cout << PREFIX << "Found the Vive driver at "
                          << m_viveWrapper->getDriverFileLocation()
                          << std::endl;
            }

            if (!m_viveWrapper->haveDriverLoaded()) {
                std::cout << PREFIX << "Could not open driver." << std::endl;
                stopAttemptingDetection();
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
    osvr::vive::ViveDriverHost &getInactiveHost() {
        if (m_driverHost) {
            throw std::logic_error("Can't get an inactive host - we already "
                                   "have a live device driver!");
        }
        if (!m_inactiveDriverHost) {
            m_inactiveDriverHost.reset(new osvr::vive::ViveDriverHost);
        }
        return *m_inactiveDriverHost.get();
    }

    void stopAttemptingDetection() {
        std::cerr << PREFIX << "Will not re-attempt detecting Vive."
                  << std::endl;
        m_shouldAttemptDetection = false;
        unloadTemporaries();
        m_driverHost.reset();
    }

    void unloadTemporaries() {
        m_viveWrapper.reset();
        m_inactiveDriverHost.reset();
    }

  private:
    /// This is the OSVR driver object, which also serves as the "SteamVR"
    /// driver host. We can only run one Vive at a time.
    osvr::vive::DriverHostPtr m_driverHost;

    /// A Vive object that we hang on to if we don't have a fully-started-up
    /// device, to save time in hardware detection.
    std::unique_ptr<osvr::vive::DriverWrapper> m_viveWrapper;
    /// Populated only when we don't have an active driver - we keep it around
    /// so we don't have to re-load the full driver every time we get hit with a
    /// hardware detect request.
    osvr::vive::DriverHostPtr m_inactiveDriverHost;

    bool m_shouldAttemptDetection = true;
};
} // namespace

OSVR_PLUGIN(com_osvr_Vive) {
    osvr::pluginkit::PluginContext context(ctx);

    /// Register a detection callback function object.
    context.registerHardwareDetectCallback(new HardwareDetection());

    return OSVR_RETURN_SUCCESS;
}
