/** @file
    @brief Header associating the interface name and version strings with the
   types for safe requesting and casting.

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

#ifndef INCLUDED_InterfaceTraits_h_GUID_36EC6818_80C9_4EA1_D86F_517E153574D0
#define INCLUDED_InterfaceTraits_h_GUID_36EC6818_80C9_4EA1_D86F_517E153574D0

// Internal Includes
// - none

// Library/third-party includes
#include <openvr_driver.h>

// Standard includes
#include <type_traits>

namespace osvr {
namespace vive {
    /// Maps from the interface type (as provided by the entry point function or
    /// GetComponent) to the name string used to retrieve it.
    template <typename InterfaceType> struct InterfaceNameTrait;

    template <> struct InterfaceNameTrait<vr::IVRWatchdogProvider> {
        static const char *get() {
            return vr::IVRWatchdogProvider_Version;
        }
    };

    template <> struct InterfaceNameTrait<vr::IServerTrackedDeviceProvider> {
        static const char *get() {
            return vr::IServerTrackedDeviceProvider_Version;
        }
    };

    template <> struct InterfaceNameTrait<vr::IVRDisplayComponent> {
        static const char *get() { return vr::IVRDisplayComponent_Version; }
    };

    template <> struct InterfaceNameTrait<vr::IVRControllerComponent> {
        static const char *get() { return vr::IVRControllerComponent_Version; }
    };

    template <> struct InterfaceNameTrait<vr::IVRCameraComponent> {
        static const char *get() { return vr::IVRCameraComponent_Version; }
    };

    /// Maps from the interface type (as provided by the entry point function)
    /// to the driver host type required by its init function.
    template <typename InterfaceType> struct InterfaceHostTrait;

    template <> struct InterfaceHostTrait<vr::IVRWatchdogProvider> {
        using type = vr::IVRWatchdogHost;
    };
    template <> struct InterfaceHostTrait<vr::IServerTrackedDeviceProvider> {
        using type = vr::IVRServerDriverHost;
    };

    /// Alias for easier use, mapping from interface type to required driver
    /// host type.
    template <typename InterfaceType>
    using InterfaceHost = typename InterfaceHostTrait<InterfaceType>::type;

    /// Identifies whether we should expect an interface to be provided by the
    /// driver entry point.
    template <typename InterfaceType>
    struct InterfaceExpectedFromEntryPointTrait : std::false_type {};

    template <>
    struct InterfaceExpectedFromEntryPointTrait<
        vr::IVRWatchdogProvider> : std::true_type {};

    template <>
    struct InterfaceExpectedFromEntryPointTrait<
        vr::IServerTrackedDeviceProvider> : std::true_type {};

    /// Identifies whether we should expect an interface to be provided by the
    /// driver entry point.
    template <typename InterfaceType>
    struct InterfaceExpectedFromGetComponent : std::false_type {};

    template <>
    struct InterfaceExpectedFromGetComponent<vr::IVRDisplayComponent>
        : std::true_type {};

    template <>
    struct InterfaceExpectedFromGetComponent<vr::IVRControllerComponent>
        : std::true_type {};

    template <>
    struct InterfaceExpectedFromGetComponent<vr::IVRCameraComponent>
        : std::true_type {};

} // namespace vive
} // namespace osvr

#endif // INCLUDED_InterfaceTraits_h_GUID_36EC6818_80C9_4EA1_D86F_517E153574D0
