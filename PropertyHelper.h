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
            template <typename T, typename... Args>
            static PropertyGetterReturn<bool>
            get(T *dev, vr::ETrackedDeviceProperty prop, Args... args) {
                vr::ETrackedPropertyError err = vr::TrackedProp_Success;
                bool val =
                    dev->GetBoolTrackedDeviceProperty(args..., prop, &err);
                return std::make_pair(val, err);
            }
        };

        template <> struct PropertyGetter<float> {
            template <typename T, typename... Args>
            static PropertyGetterReturn<float>
            get(T *dev, vr::ETrackedDeviceProperty prop, Args... args) {
                vr::ETrackedPropertyError err = vr::TrackedProp_Success;
                float val = dev->GetFloatTrackedDeviceProperty(
                    std::forward<Args>(args)..., prop, &err);
                return std::make_pair(val, err);
            }
        };

        template <> struct PropertyGetter<int32_t> {
            template <typename T, typename... Args>
            static PropertyGetterReturn<int32_t>
            get(T *dev, vr::ETrackedDeviceProperty prop, Args... args) {
                vr::ETrackedPropertyError err = vr::TrackedProp_Success;
                int32_t val =
                    dev->GetInt32TrackedDeviceProperty(args..., prop, &err);
                return std::make_pair(val, err);
            }
        };

        template <> struct PropertyGetter<vr::HmdMatrix34_t> {
            template <typename T, typename... Args>
            static PropertyGetterReturn<vr::HmdMatrix34_t>
            get(T *dev, vr::ETrackedDeviceProperty prop, Args... args) {
                vr::ETrackedPropertyError err = vr::TrackedProp_Success;
                vr::HmdMatrix34_t val =
                    dev->GetMatrix34TrackedDeviceProperty(args..., prop, &err);
                return std::make_pair(val, err);
            }
        };

        template <> struct PropertyGetter<std::string> {
            template <typename T, typename... Args>
            static PropertyGetterReturn<std::string>
            get(T *dev, vr::ETrackedDeviceProperty prop, Args... args) {

                assert(dev != nullptr &&
                       "Tried to get a string property from a null device "
                       "pointer.");
                static const auto INITIAL_BUFFER_SIZE =
                    vr::k_unTrackingStringSize;
                /// Start with a buffer of k_unTrackingStringSize as suggested.
                std::vector<char> buf(INITIAL_BUFFER_SIZE, '\0');
                vr::ETrackedPropertyError err = vr::TrackedProp_Success;
                auto ret = dev->GetStringTrackedDeviceProperty(
                    args..., prop, buf.data(),
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
                    ret = dev->GetStringTrackedDeviceProperty(
                        args..., prop, buf.data(),
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
            template <typename T, typename... Args>
            static PropertyGetterReturn<uint64_t>
            get(T *self, vr::ETrackedDeviceProperty prop, Args... args) {
                vr::ETrackedPropertyError err = vr::TrackedProp_Success;
                uint64_t val =
                    self->GetUint64TrackedDeviceProperty(args..., prop, &err);
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
        template <std::size_t EnumVal, typename Obj, typename... Args>
        inline detail::EnumGetterReturn<EnumVal> getProperty(Obj *obj,
                                                             Args... args) {
            return detail::PropertyGetterFromSizeT<EnumVal>::get(
                obj, detail::castToProperty(EnumVal), args...);
        }

        /// @overload
        ///
        /// Get a property - pass the Props:: enum class shortened name as the
        /// template parameter, and the object (vr::ITrackedDeviceServerDriver
        /// *, vr::IVRSystem*) and any additional parameters required
        /// (vr::TrackedDeviceIndex_t for IVRSystem, for instance), and get back
        /// a pair with your result, correctly typed, and your error code.
        template <Props EnumVal, typename Obj, typename... Args>
        inline detail::EnumClassGetterReturn<EnumVal>
        getProperty(Obj *obj, Args... args) {
            return detail::PropertyGetterFromEnumClass<EnumVal>::get(
                obj, detail::castToProperty(EnumVal), args...);
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
        template <typename T, typename Obj, typename... Args>
        inline detail::PropertyGetterReturn<T>
        getPropertyOfType(Obj *obj, vr::ETrackedDeviceProperty prop,
                          Args... args) {
            return detail::PropertyGetter<T>::get(obj, prop, args...);
        }

        /// @overload
        ///
        /// Takes a Props:: enum class shortened name instead of a vr::Prop_...
        /// enum.
        template <typename T, typename Obj, typename... Args>
        inline detail::PropertyGetterReturn<T>
        getPropertyOfType(Obj *obj, Props prop, Args... args) {
            return detail::PropertyGetter<T>::get(
                obj, detail::castToProperty(prop), args...);
        }
    } // namespace generic
} // namespace vive
} // namespace osvr

#endif // INCLUDED_PropertyHelper_h_GUID_08BEA00F_2E0C_4FA5_DF5B_31BE33EF27A3
