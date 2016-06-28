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
#include "ChaperoneData.h"
#include "DriverWrapper.h"
#include "GetComponent.h"
#include "ServerPropertyHelper.h"

// Library/third-party includes
#include <openvr_driver.h>

// Standard includes
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace osvr {
namespace vive {} // namespace vive
} // namespace osvr

static const auto PREFIX = "[ViveLoader] ";
static void whatIsThisDevice(vr::ITrackedDeviceServerDriver *dev) {
    {
        auto disp = osvr::vive::getComponent<vr::IVRDisplayComponent>(dev);
        if (disp) {
            std::cout << PREFIX << "-- it's a display, too!" << std::endl;
            vr::ETrackedPropertyError err;
            uint64_t universe = 0;
            std::tie(universe, err) =
                osvr::vive::getProperty<osvr::vive::Props::CurrentUniverseId>(
                    dev);
            if (vr::TrackedProp_Success == err) {
                std::cout << PREFIX << " -- In tracking universe " << universe
                          << std::endl;
            } else if (vr::TrackedProp_NotYetAvailable == err) {
                std::cout << PREFIX << " -- Tracking universe not yet known"
                          << std::endl;
            } else {
                std::cout << PREFIX << " -- Some other error trying to figure "
                                       "out the tracking universe: "
                          << err << std::endl;
            }
        }
    }

    {
        auto controller =
            osvr::vive::getComponent<vr::IVRControllerComponent>(dev);
        if (controller) {
            std::cout << PREFIX << "-- it's a controller, too!" << std::endl;
        }
    }

    {
        auto cam = osvr::vive::getComponent<vr::IVRCameraComponent>(dev);
        if (cam) {
            std::cout << PREFIX << "-- it's a camera, too!" << std::endl;
        }
    }
    std::cout << "\n";
}

void lookForUniverseData(osvr::vive::DriverWrapper &vive) {
    if (!vive.haveChaperoneData()) {
        std::cout
            << PREFIX
            << "DriverWrapper couldn't get to starting up the chaperone data."
            << std::endl;
        return;
    }
    if (!vive.chaperone()) {
        std::cout << PREFIX
                  << "Chaperone data error: " << vive.chaperone().getMessage()
                  << std::endl;
        return;
    }

    if (vive.chaperone().hasMessages()) {
        std::cout << PREFIX
                  << "Chaperone data warning: " << vive.chaperone().getMessage()
                  << std::endl;
    }

    std::uint64_t universe = 0;
    for (auto &devPtr :
         vive.devices().rawDeviceVectorAccess_NOT_RECOMMENDED_TODO_FIXME()) {
        if (!devPtr) {
            continue;
        }
        vr::ETrackedPropertyError err;
        uint64_t myUniverse = 0;
        std::tie(myUniverse, err) =
            osvr::vive::getProperty<osvr::vive::Props::CurrentUniverseId>(
                devPtr);
        if (vr::TrackedProp_Success != err || myUniverse == 0) {
            continue;
        }
        auto model =
            osvr::vive::getProperty<osvr::vive::Props::ModelNumber>(devPtr)
                .first;
        std::cout << PREFIX << "Got a real universe (" << myUniverse
                  << ") from " << model << std::endl;
        universe = myUniverse;
    }

    if (vive.chaperone().knowUniverseId(universe)) {
        std::cout << PREFIX << "ChaperoneData has info on universe " << universe
                  << std::endl;
        auto data = vive.chaperone().getDataForUniverse(universe);
        std::cout << PREFIX << "Translation: [" << data.translation[0] << ", "
                  << data.translation[1] << ", " << data.translation[2]
                  << "], yaw " << data.yaw << std::endl;
    } else {
        std::cout << PREFIX << "ChaperoneData sadly has no info on universe "
                  << universe << std::endl;
    }
}

int main() {
    auto vive = osvr::vive::DriverWrapper();

    /// These lines are just informational printout - the real check is later -
    /// if (!vive)
    if (vive.foundDriver()) {
        std::cout << PREFIX << "Found the Vive driver at "
                  << vive.getDriverFileLocation() << std::endl;
    }

    if (!vive.haveDriverLoaded()) {
        std::cout << PREFIX << "Could not open driver." << std::endl;
    }

    if (vive.foundConfigDirs()) {
        std::cout << PREFIX
                  << "Driver config dir is: " << vive.getDriverConfigDir()
                  << std::endl;
    }

    if (!vive) {
        std::cerr << PREFIX
                  << "Error in first-stage Vive driver startup. Exiting"
                  << std::endl;
        return 1;
    }

    if (!vive.isHMDPresent()) {
        std::cerr << PREFIX
                  << "Driver loaded, but no Vive is connected. Exiting"
                  << std::endl;
        return 0;
    }

    std::cout << PREFIX << "Vive is connected." << std::endl;

    if (!vive.startServerDeviceProvider()) {
        // can either check return value of this, or do another if (!vive) after
        // calling - equivalent.
        std::cerr << PREFIX
                  << "Error: could not start the server device provider in the "
                     "Vive driver. Exiting."
                  << std::endl;
        return 1;
    }

    /// but now, we can do things with vive.serverDevProvider()

    /// Power the system up.
    vive.serverDevProvider().LeaveStandby();

    std::vector<std::string> knownSerialNumbers;
    auto handleNewDevice = [&](const char *serialNum) {
        auto dev = vive.serverDevProvider().FindTrackedDeviceDriver(
            serialNum, vr::ITrackedDeviceServerDriver_Version);
        if (!dev) {
            std::cout << PREFIX
                      << "Couldn't find the corresponding device driver for "
                      << serialNum << std::endl;
            return false;
        }
        auto ret = vive.devices().addAndActivateDevice(dev);
        if (!ret) {
            std::cout << PREFIX << "Device with serial number " << serialNum
                      << " couldn't be added to the devices vector."
                      << std::endl;
            return false;
        }
        std::cout << "\n"
                  << PREFIX << "Device with s/n " << serialNum
                  << " activated, assigned ID " << ret.value << std::endl;
        whatIsThisDevice(dev);
        return true;
    };

    vive.driverHost().onTrackedDeviceAdded = handleNewDevice;

    {
        auto numDevices = vive.serverDevProvider().GetTrackedDeviceCount();
        std::cout << PREFIX << "Got " << numDevices
                  << " tracked devices at startup" << std::endl;
        for (decltype(numDevices) i = 0; i < numDevices; ++i) {
            auto dev = vive.serverDevProvider().GetTrackedDeviceDriver(
                i, vr::ITrackedDeviceServerDriver_Version);
            vive.devices().addAndActivateDevice(dev);
            std::cout << PREFIX << "Device " << i << std::endl;
            whatIsThisDevice(dev);
        }
    }

    std::cout << "*** Entering dummy mainloop" << std::endl;
    for (int i = 0; i < 3000; ++i) {
        vive.serverDevProvider().RunFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::cout << "*** Done with dummy mainloop" << std::endl;

    lookForUniverseData(vive);

    /// The vive object will automatically deactivate all of them on
    /// destruction, or on a call to stop().
    std::cout << PREFIX << "Stopping the Vive." << std::endl;
    vive.stop();

    /// This line will turn off the wireless wands.
    // vive.serverDevProvider().EnterStandby();

    return 0;
}
