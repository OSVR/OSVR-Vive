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

#define _USE_MATH_DEFINES

// Internal Includes
#include "ComputeOutputFiles.h"
#include "DisplayDescriptor.h"
#include "DriverWrapper.h"
#include "GetComponent.h"
#include "RGBPoints.h"
#include "ServerPropertyHelper.h"

#include "viveDisplayInput.h"

// Library/third-party includes
#include <openvr_driver.h>
#include <osvr/Util/StringLiteralFileToString.h>

// Standard includes
#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <memory>
#include <thread>
#include <tuple>

/// the number of steps in each dimension
static const auto MESH_STEPS = 15;

static const auto PREFIX = "[DisplayExtractor] ";

using namespace osvr::vive;

std::unique_ptr<DisplayDescriptor> g_descriptor;
bool g_gotDisplay = false;

inline UnitClippingPlane getClippingPlanes(vr::IVRDisplayComponent *display,
                                           vr::EVREye eye) {
    UnitClippingPlane ret;
    /// Note that work with the steamvr-osvr driver has indicated that SteamVR
    /// apparently flips the top and bottom clipping plane location?
    display->GetProjectionRaw(eye, &ret.left, &ret.right, &ret.bottom,
                              &ret.top);
    return ret;
}

void updateCenterOfProjection(DisplayDescriptor & /*descriptor*/,
                              vr::TrackedDeviceIndex_t idx) {
    /// Get the two eye/lens center of projections.
    using CenterOfProjectionIndices =
        std::tuple<std::size_t, osvr::vive::Props, osvr::vive::Props>;
    using std::get;
    for (auto &data : {CenterOfProjectionIndices{0, Props::LensCenterLeftU,
                                                 Props::LensCenterLeftV},
                       CenterOfProjectionIndices{1, Props::LensCenterRightU,
                                                 Props::LensCenterRightV}}) {
        auto x = getPropertyOfType<float>(get<1>(data), idx).first;
        auto y = getPropertyOfType<float>(get<2>(data), idx).first;

        g_descriptor->updateCenterOfProjection(get<0>(data), {{x, y}});
    }
}

bool updateFOV(DisplayDescriptor &descriptor,
               vr::IVRDisplayComponent *display) {
    /// Back-calculate the display parameters based on the projection
    /// clipping planes.
    auto leftClip = getClippingPlanes(display, vr::Eye_Left);
    auto rightClip = getClippingPlanes(display, vr::Eye_Right);
#if 0
    std::cout << PREFIX << "leftClip: " << leftClip << std::endl;
    std::cout << PREFIX << "rightClip: " << rightClip << std::endl;
#endif

    auto leftFovs = clipPlanesToHalfFovs(leftClip);
    auto rightFovs = clipPlanesToHalfFovs(rightClip);

    auto fovsResult = twoEyeFovsToMonoWithOverlap(leftFovs, rightFovs);
    if (fovsResult.first) {
        // we successfully computed the conversion!
        descriptor.updateFovs(fovsResult.second);
        return true;
    }

    // Couldn't compute the conversion - must not be symmetrical enough
    // at the moment.
    std::cout << PREFIX
              << "First attempt was not symmetrical, will symmetrize "
                 "and re-convert the field of view."
              << std::endl;
    averageAndSymmetrize(leftFovs, rightFovs);
    fovsResult = twoEyeFovsToMonoWithOverlap(leftFovs, rightFovs);
    if (fovsResult.first) {
        // we successfully computed the conversion (of an
        // approximation)!
        descriptor.updateFovs(fovsResult.second);
        return true;
    }
    std::cout << PREFIX << "Still failed to produce a conversion." << std::endl;
    return false;
}

inline void addMeshPoint(vr::IVRDisplayComponent *display, RGBPoints &mesh,
                         const float u, const float v) {
    for (std::size_t eye = 0; eye < 2; ++eye) {
        auto steamEye = eye == 0 ? vr::Eye_Left : vr::Eye_Right;
        auto meshEye = eye == 0 ? RGBPoints::Eye::Left : RGBPoints::Eye::Right;

        auto ret = display->ComputeDistortion(steamEye, u, v);

        mesh.addSample(meshEye, {{u, v}}, {{ret.rfRed[0], ret.rfRed[1]}},
                       {{ret.rfGreen[0], ret.rfGreen[1]}},
                       {{ret.rfBlue[0], ret.rfBlue[1]}});
    }
}

std::string generateMeshFileContents(vr::IVRDisplayComponent *display,
                                     const std::size_t steps = 3) {
    RGBPoints mesh;
    const auto realSteps = steps - 1;
    const float stepSize = 1.f / static_cast<float>(steps);
    for (std::size_t uInt = 0; uInt < realSteps; ++uInt) {
        const auto u = uInt * stepSize;
        for (std::size_t vInt = 0; vInt < realSteps; ++vInt) {
            const auto v = vInt * stepSize;
            addMeshPoint(display, mesh, u, v);
        }
        // make sure we get v = 1.
        addMeshPoint(display, mesh, u, 1);
    }
    // and get (1, 1) as well.
    addMeshPoint(display, mesh, 1, 1);
    return mesh.getSeparateFile();
}

void handleDisplay(vr::TrackedDeviceIndex_t idx,
                   vr::IVRDisplayComponent *display) {

    g_gotDisplay = true;

    /// Set the human-readable parts of the device descriptor.
    {
        vr::ETrackedPropertyError err;
        std::string mfr;
        std::tie(mfr, err) = getProperty<Props::ManufacturerName>(idx);
        if (mfr.empty() || err != vr::TrackedProp_Success) {
            std::cerr << "Error trying to read the manufacturer of the "
                         "attached HMD..."
                      << std::endl;
            mfr.clear();
        } else {
            g_descriptor->setVendor(mfr);
        }
        std::string model;
        std::tie(model, err) = getProperty<Props::ModelNumber>(idx);
        if (model.empty() || err != vr::TrackedProp_Success) {
            std::cerr << "Error trying to read the model of the attached HMD..."
                      << std::endl;
            model.clear();
        } else {
            if (model == "Vive DVT") {
                // The Vive Design Verification Test is better known as...
                model = "Vive PRE";
            }
            g_descriptor->setModel(model);
        }

        std::string serial;
        std::tie(serial, err) = getProperty<Props::SerialNumber>(idx);
        std::string unit;
        if (serial.empty() || err != vr::TrackedProp_Success) {
            std::cerr << "Error trying to read the serial number of the "
                         "attached HMD..."
                      << std::endl;
            unit = mfr + " " + model;
            g_descriptor->setNote("Specific to an individual " + unit +
                                  " unit.");
        } else {
            unit = mfr + " " + model + " serial number " + serial;
            g_descriptor->setNote("Specific to " + unit + ".");
        }

        std::cout << "\n" << PREFIX << unit << "\n" << std::endl;
    }
    /// Verify/check the resolution
    {
        int32_t x, y;
        uint32_t width, height;
        display->GetWindowBounds(&x, &y, &width, &height);
        g_descriptor->setResolution(width, height);
    }

    updateCenterOfProjection(*g_descriptor, idx);

    if (!updateFOV(*g_descriptor, display)) {
        g_gotDisplay = false;
        return;
    }

    std::cout << "\n"
              << PREFIX
              << "Successfully extracted display parameters from Vive.\n"
              << std::endl;

    auto outputFiles =
        computeOutputFiles("HTC_Vive.json", "HTC_Vive_meshdata.json");
    auto meshContents = generateMeshFileContents(display, MESH_STEPS);

    std::cout << PREFIX << "Writing distortion mesh data file:\n"
              << outputFiles.meshFilePath << "\n"
              << std::endl;
    {
        std::ofstream os(outputFiles.meshFilePath);
        os << meshContents << std::flush;
        os.close();
    }
    g_descriptor->setRGBMeshExternalFile(outputFiles.meshFilePath);
    std::cout << PREFIX << "Writing display descriptor file:\n"
              << outputFiles.displayDescriptorPath << "\n"
              << std::endl;
    {
        std::ofstream os(outputFiles.displayDescriptorPath);
        os << g_descriptor->getDescriptor() << std::flush;
        os.close();
    }
    std::cout << PREFIX
              << "Be sure to re-run this tool if you move your OSVR Server "
                 "installation, as the generated files include absolute paths."
              << std::endl;
}

int main() {
    {
        g_descriptor = std::make_unique<DisplayDescriptor>(
            osvr::util::makeString(viveDisplayInput));
        if (!(*g_descriptor)) {
            std::cerr << PREFIX
                      << "Could not parse template for display descriptor."
                      << std::endl;
            return 1;
        }
    }
    {
        auto vive = osvr::vive::DriverWrapper();
        if (!vive) {
            std::cerr << PREFIX
                      << "Error in first-stage Vive driver startup. Exiting"
                      << std::endl;
            return 1;
        }

        auto handleNewDevice = [&](const char *serialNum,
                                   vr::ETrackedDeviceClass eDeviceClass,
                                   vr::ITrackedDeviceServerDriver *pDriver) {
            auto dev = pDriver;
            if (dev == nullptr) {
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

            return true;
        };

        vive.driverHost().onTrackedDeviceAdded = handleNewDevice;

        if (!vive.startServerDeviceProvider()) {
            // can either check return value of this, or do another if (!vive)
            // after calling - equivalent.
            std::cerr
                << PREFIX
                << "Error: could not start the server device provider in the "
                   "Vive driver. Exiting."
                << std::endl;
            return 1;
        }

        if (osvr::vive::DriverWrapper::InterfaceVersionStatus::
                InterfaceMismatch ==
            vive.checkServerDeviceProviderInterfaces()) {
            std::cerr << PREFIX
                      << "SteamVR driver requires unavailable/unsupported "
                         "interface versions - either too old or too new for "
                         "this build. Cannot continue."
                      << std::endl;
            for (const auto &iface : vive.getUnsupportedRequestedInterfaces()) {
                if (osvr::vive::isInterfaceNameWeCareAbout(iface)) {
                    auto supported =
                        vive.getSupportedInterfaceVersions()
                            .findSupportedVersionOfInterface(iface);
                    std::cerr << PREFIX << " - Driver requested " << iface
                              << " but we support " << supported << std::endl;
                }
            }
            return 1;
        }

        /// Power the system up.
        vive.serverDevProvider().LeaveStandby();

        {
            DeviceHolder devHolder = std::move(vive.devices());
            auto numDevices = devHolder.numDevices();
            std::cout << PREFIX << "Got " << numDevices
                      << " tracked devices at startup" << std::endl;
            for (std::uint32_t i = 0; i < numDevices; ++i) {
                vr::ITrackedDeviceServerDriver *dev = &(devHolder.getDevice(i));
                auto disp =
                    osvr::vive::getComponent<vr::IVRDisplayComponent>(dev);
                if (disp != nullptr) {
                    std::cout << PREFIX
                              << "-- it's a display, too! We'll extract its "
                                 "display parameters now."
                              << std::endl;
                    vr::TrackedDeviceIndex_t idx = i;
                    handleDisplay(idx, disp);
                    break;
                }
            }
        }

        vive.stop();
        if (vive.serverDevProvider().ShouldBlockStandbyMode()) {
            std::cout << PREFIX
                      << "Driver is reporting that it is busy and "
                         "should block standby mode, so we will wait "
                         "until it is finished to exit..."
                      << std::endl;
            do {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            } while (vive.serverDevProvider().ShouldBlockStandbyMode());
        }
        std::cout << PREFIX << "Press enter to quit..." << std::endl;
        std::cin.ignore();
    }

    return 0;
}
