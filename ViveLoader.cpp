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

static const auto PREFIX = "[ViveLoader] ";
static void whatIsThisDevice(vr::ITrackedDeviceServerDriver *dev,
                             vr::TrackedDeviceIndex_t idx) {
    {
        auto disp = osvr::vive::getComponent<vr::IVRDisplayComponent>(dev);
        if (disp) {
            std::cout << PREFIX << "-- it's a display, too!" << std::endl;
            vr::ETrackedPropertyError err;
            uint64_t universe = 0;
            std::tie(universe, err) =
                osvr::vive::getProperty<osvr::vive::Props::CurrentUniverseId>(
                    idx);
            if (vr::TrackedProp_Success == err) {
                std::cout << PREFIX << " -- In tracking universe " << universe
                          << std::endl;
            } else if (vr::TrackedProp_NotYetAvailable == err) {
                std::cout << PREFIX << " -- Tracking universe not yet known"
                          << std::endl;
            } else {
                std::cout << PREFIX
                          << " -- Some other error trying to figure "
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
    vr::TrackedDeviceIndex_t idx = 0;
    for (auto &devPtr :
         vive.devices().rawDeviceVectorAccess_NOT_RECOMMENDED_TODO_FIXME()) {
        if (!devPtr) {
            continue;
        }
        vr::ETrackedPropertyError err;
        uint64_t myUniverse = 0;
        std::tie(myUniverse, err) =
            osvr::vive::getProperty<osvr::vive::Props::CurrentUniverseId>(idx);
        if (vr::TrackedProp_Success != err || myUniverse == 0) {
            continue;
        }
        auto model =
            osvr::vive::getProperty<osvr::vive::Props::ModelNumber>(idx).first;
        std::cout << PREFIX << "Got a real universe (" << myUniverse
                  << ") from " << model << std::endl;
        universe = myUniverse;
        idx++;
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

    std::cout << PREFIX << "Vive is connected!!!" << std::endl;

    auto handleNewDevice = [&](const char *serialNum,
                               vr::ETrackedDeviceClass eDeviceClass,
                               vr::ITrackedDeviceServerDriver *pDriver) {
        auto dev = pDriver;
        if (!dev) {
            std::cout << PREFIX << "null input device" << std::endl;
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
        vr::TrackedDeviceIndex_t idx = ret.value;
        whatIsThisDevice(dev, idx);
        return true;
    };

    vive.driverHost().onTrackedDeviceAdded = handleNewDevice;

    if (!vive.startServerDeviceProvider()) {
        // can either check return value of this, or do another if (!vive) after
        // calling - equivalent.
        std::cerr << PREFIX
                  << "Error: could not start the server device provider in the "
                     "Vive driver. Exiting."
                  << std::endl;
        return 1;
    }

    std::cout << "Vive has started." << std::endl;

    /// but now, we can do things with vive.serverDevProvider()

    /// first thing we should do is check driver compatibility.
    auto interfaceStatus = vive.checkServerDeviceProviderInterfaces();
    switch (interfaceStatus) {
    case osvr::vive::DriverWrapper::InterfaceVersionStatus::AllInterfacesOK:
        std::cerr << PREFIX
                  << "All interface versions mentioned by the driver "
                     "are available and supported."
                  << std::endl;
        break;
    case osvr::vive::DriverWrapper::InterfaceVersionStatus::AllUsedInterfacesOK:
        std::cerr << PREFIX
                  << "Not all interface versions mentioned by the "
                     "driver are available and supported, but the "
                     "ones used by this code match."
                  << std::endl;
        break;
    case osvr::vive::DriverWrapper::InterfaceVersionStatus::InterfaceMismatch:
        std::cerr
            << PREFIX
            << "Driver requires unavailable/unsupported interface versions."
            << std::endl;
        break;
    default:
        break;
    }

    switch (interfaceStatus) {
    case osvr::vive::DriverWrapper::InterfaceVersionStatus::AllUsedInterfacesOK:
    case osvr::vive::DriverWrapper::InterfaceVersionStatus::InterfaceMismatch:
        std::cerr << PREFIX
                  << "Unavailable interface version strings:" << std::endl;
        for (auto iface : vive.getUnsupportedRequestedInterfaces()) {
            std::cerr << PREFIX << " - " << iface << std::endl;
        }
        break;
    case osvr::vive::DriverWrapper::InterfaceVersionStatus::AllInterfacesOK:
    default:
        break;
    }

    if (osvr::vive::DriverWrapper::InterfaceVersionStatus::InterfaceMismatch ==
        interfaceStatus) {
        std::cerr << PREFIX << "Cannot continue." << std::endl;
        return 1;
    }

    /// Power the system up.
    vive.serverDevProvider().LeaveStandby();

#if 0
    {
        osvr::vive::DeviceHolder devHolder = std::move(vive.devices());
        auto numDevices = devHolder.numDevices();
        std::cout << PREFIX << "Got " << numDevices
                  << " tracked devices at startup" << std::endl;
        for (decltype(numDevices) i = 0; i < numDevices; ++i) {
            vr::ITrackedDeviceServerDriver *dev = &(devHolder.getDevice(i));
            std::cout << PREFIX << "Device " << i << std::endl;
            whatIsThisDevice(dev, i);
        }
    }
#endif
    std::cout << "*** Entering dummy mainloop" << std::endl;
#if 1
    for (int i = 0; i < 3000; ++i) {
        vive.serverDevProvider().RunFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
#else
    std::this_thread::sleep_for(std::chrono::seconds(10));
#endif
    std::cout << "*** Done with dummy mainloop" << std::endl;

    lookForUniverseData(vive);

    /// The vive object will automatically deactivate all of them on
    /// destruction, or on a call to stop().
    std::cout << PREFIX << "Stopping the Vive." << std::endl;
    vive.stop();

#if 0
    /// This line will turn off the wireless wands.
    vive.serverDevProvider().EnterStandby();
#endif
    return 0;
}
