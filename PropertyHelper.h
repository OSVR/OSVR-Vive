/** @file
    @brief Header designed to build on the generated PropertyTraits.h header.

    BSD-3-Clause to match the "OpenVR" and thus PropertyTraits.h license.

    @date 2016

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

/*
SPDX-License-Identifier: BSD-3-Clause

Copyright (c) 2016, Razer Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef INCLUDED_PropertyHelper_h_GUID_08BEA00F_2E0C_4FA5_DF5B_31BE33EF27A3
#define INCLUDED_PropertyHelper_h_GUID_08BEA00F_2E0C_4FA5_DF5B_31BE33EF27A3

#include <cstddef>

// Include the generated file
#include "PropertyTraits.h"

// Standard includes
#include <assert.h>
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace osvr {
namespace vive {

    namespace detail {
        // k_unTrackingStringSize defined in the vr:: is no more, defined here
        static const uint32_t k_unTrackingStringSize = 32;

        template <typename PropertyType>
        using PropertyGetterReturn =
            std::pair<PropertyType, vr::ETrackedPropertyError>;

        template <std::size_t EnumVal>
        using EnumGetterReturn = PropertyGetterReturn<PropertyType<EnumVal>>;

        template <Props EnumVal>
        struct EnumClassToSizeT
            : std::integral_constant<std::size_t,
                                     static_cast<std::size_t>(
                                         static_cast<int>(EnumVal))> {};

        template <Props EnumVal>
        using EnumClassGetterReturn =
            EnumGetterReturn<EnumClassToSizeT<EnumVal>::value>;

        inline vr::ETrackedDeviceProperty castToProperty(std::size_t prop) {
            return static_cast<vr::ETrackedDeviceProperty>(prop);
        }
        inline vr::ETrackedDeviceProperty castToProperty(Props val) {
            return static_cast<vr::ETrackedDeviceProperty>(
                static_cast<int>(val));
        }

        template <typename EnumType> struct PropertyGetter;

        template <> struct PropertyGetter<bool> {
            static PropertyGetterReturn<bool>
            get(vr::ETrackedDeviceProperty prop,
                vr::TrackedDeviceIndex_t unObjectId) {
                vr::ETrackedPropertyError err = vr::TrackedProp_Success;
                auto propertyContainer =
                    vr::VRProperties()->TrackedDeviceToPropertyContainer(
                        unObjectId);
                bool val = vr::VRProperties()->GetBoolProperty(
                    propertyContainer, prop, &err);
                return std::make_pair(val, err);
            }
        };

        template <> struct PropertyGetter<float> {
            static PropertyGetterReturn<float>
            get(vr::ETrackedDeviceProperty prop,
                vr::TrackedDeviceIndex_t unObjectId) {
                vr::ETrackedPropertyError err = vr::TrackedProp_Success;
                auto propertyContainer =
                    vr::VRProperties()->TrackedDeviceToPropertyContainer(
                        unObjectId);
                float val = vr::VRProperties()->GetFloatProperty(
                    propertyContainer, prop, &err);
                return std::make_pair(val, err);
            }
        };

        template <> struct PropertyGetter<int32_t> {
            static PropertyGetterReturn<int32_t>
            get(vr::ETrackedDeviceProperty prop,
                vr::TrackedDeviceIndex_t unObjectId) {
                vr::ETrackedPropertyError err = vr::TrackedProp_Success;
                auto propertyContainer =
                    vr::VRProperties()->TrackedDeviceToPropertyContainer(
                        unObjectId);
                int32_t val = vr::VRProperties()->GetInt32Property(
                    propertyContainer, prop, &err);
                return std::make_pair(val, err);
            }
        };

        /// SYQ-1: comment this out because VRProperties() doesn't have
        /// GetMaxtrix34Property
        //    template <> struct PropertyGetter<vr::HmdMatrix34_t> {
        //        static PropertyGetterReturn<vr::HmdMatrix34_t>
        //        get(vr::ETrackedDeviceProperty prop, vr::TrackedDeviceIndex_t
        //        unObjectId) {
        //            vr::ETrackedPropertyError err = vr::TrackedProp_Success;
        // auto propertyContainer =
        // vr::VRProperties()->TrackedDeviceToPropertyContainer(unObjectId);
        //            vr::HmdMatrix34_t val = vr::VRProperties()->
        //                dev->GetMatrix34TrackedDeviceProperty(args..., prop,
        //                &err);
        //            return std::make_pair(val, err);
        //        }
        //    };

        template <> struct PropertyGetter<std::string> {
            static PropertyGetterReturn<std::string>
            get(vr::ETrackedDeviceProperty prop,
                vr::TrackedDeviceIndex_t unObjectId) {
                static const auto INITIAL_BUFFER_SIZE = k_unTrackingStringSize;
                /// Start with a buffer of k_unTrackingStringSize as suggested.
                std::vector<char> buf(INITIAL_BUFFER_SIZE, '\0');
                vr::ETrackedPropertyError err = vr::TrackedProp_Success;
                auto propertyContainer =
                    vr::VRProperties()->TrackedDeviceToPropertyContainer(
                        unObjectId);
                uint32_t ret = vr::VRProperties()->GetStringProperty(
                    propertyContainer, prop, buf.data(),
                    static_cast<uint32_t>(buf.size()), &err);
                if (0 == ret) {
                    // property not available
                    return std::make_pair(std::string{}, err);
                }
                if (ret > buf.size()) {
                    std::cout
                        << "[getStringProperty] Got an initial return value "
                           "larger than the buffer size: ret = "
                        << ret << ", buf.size() = " << buf.size() << std::endl;
                }
                if (vr::TrackedProp_BufferTooSmall == err) {
                    // first buffer was too small, but now we know how big it
                    // should be, per the docs.
                    /// @todo remove this debug print
                    std::cout << "[getStringProperty] Initial buffer size: "
                              << buf.size() << ", return value: " << ret
                              << std::endl;
                    buf.resize(ret + 1, '\0');
                    ret = 1;
                    ret = vr::VRProperties()->GetStringProperty(
                        propertyContainer, prop, buf.data(),
                        static_cast<uint32_t>(buf.size()), &err);
                }
                if (ret > buf.size()) {
                    std::cout
                        << "[getStringProperty] THIS SHOULDN'T HAPPEN: Got a "
                           "return value larger than the buffer size: ret = "
                        << ret << ", buf.size() = " << buf.size() << std::endl;
                    return std::make_pair(std::string{}, err);
                }
                return std::make_pair(std::string{buf.data()}, err);
            }
        };

        template <> struct PropertyGetter<uint64_t> {
            static PropertyGetterReturn<uint64_t>
            get(vr::ETrackedDeviceProperty prop,
                vr::TrackedDeviceIndex_t unObjectId) {
                vr::ETrackedPropertyError err = vr::TrackedProp_Success;
                auto propertyContainer =
                    vr::VRProperties()->TrackedDeviceToPropertyContainer(
                        unObjectId);
                uint64_t val = vr::VRProperties()->GetUint64Property(
                    propertyContainer, prop, &err);
                return std::make_pair(val, err);
            }
        };

        template <std::size_t EnumVal>
        using PropertyGetterFromSizeT = PropertyGetter<PropertyType<EnumVal>>;

        template <Props EnumVal>
        using PropertyGetterFromEnumClass =
            PropertyGetterFromSizeT<EnumClassToSizeT<EnumVal>::value>;
    } // namespace detail

    namespace generic {
        /// Get a property - pass the vr::Prop_.... as the template parameter,
        /// and the object (vr::ITrackedDeviceServerDriver *, vr::IVRSystem*)
        /// and any additional parameters required (vr::TrackedDeviceIndex_t
        /// for IVRSystem, for instance), and get back a pair with your result,
        /// correctly typed, and your error code.
        template <std::size_t EnumVal>
        inline detail::EnumGetterReturn<EnumVal>
        getProperty(vr::TrackedDeviceIndex_t unObjectId) {
            return detail::PropertyGetterFromSizeT<EnumVal>::get(
                detail::castToProperty(EnumVal), unObjectId);
        }

        /// @overload
        ///
        /// Get a property - pass the Props:: enum class shortened name as the
        /// template parameter, and the object (vr::ITrackedDeviceServerDriver
        /// *, vr::IVRSystem*) and any additional parameters required
        /// (vr::TrackedDeviceIndex_t for IVRSystem, for instance), and get back
        /// a pair with your result, correctly typed, and your error code.
        template <Props EnumVal>
        inline detail::EnumClassGetterReturn<EnumVal>
        getProperty(vr::TrackedDeviceIndex_t unObjectId) {
            return detail::PropertyGetterFromEnumClass<EnumVal>::get(
                detail::castToProperty(EnumVal), unObjectId);
        }

        /// Get a property when you only have the type at compile time, not the
        /// enum itself (not as asfe as getProperty())
        ///
        /// Pass the type you'd like to receive as the template parameter (one
        /// of std::string, bool, int32_t, uint64_t, and vr::HmdMatrix34_t), and
        /// pass the and the object (vr::ITrackedDeviceServerDriver *,
        /// vr::IVRSystem*), the vr::Prop_.... enum, and any additional
        /// parameters required (vr::TrackedDeviceIndex_t for IVRSystem, for
        /// instance), and get back a pair with your result as requested, and
        /// your error code.
        template <typename T>
        inline detail::PropertyGetterReturn<T>
        getPropertyOfType(vr::ETrackedDeviceProperty prop,
                          vr::TrackedDeviceIndex_t unObjectId) {
            return detail::PropertyGetter<T>::get(prop, unObjectId);
        }

        /// @overload
        ///
        /// Takes a Props:: enum class shortened name instead of a vr::Prop_...
        /// enum.
        template <typename T>
        inline detail::PropertyGetterReturn<T>
        getPropertyOfType(Props prop, vr::TrackedDeviceIndex_t unObjectId) {
            return detail::PropertyGetter<T>::get(detail::castToProperty(prop),
                                                  unObjectId);
        }
    } // namespace generic
} // namespace vive
} // namespace osvr

#endif // INCLUDED_PropertyHelper_h_GUID_08BEA00F_2E0C_4FA5_DF5B_31BE33EF27A3
