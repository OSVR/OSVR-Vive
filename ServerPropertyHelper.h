/** @file
    @brief Header based on PropertyHelper for users of openvr_driver.h

    BSD-3-Clause to match the "OpenVR" and thus PropertyTraits.h license.

    @date 2016

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

/*
SPDX-License-Identifier: BSD-3-Clause

Copyright (c) 2016, Sensics, Inc.
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

#ifndef INCLUDED_ServerPropertyHelper_h_GUID_9835023A_E639_409F_3A68_CFB4029FBBE7
#define INCLUDED_ServerPropertyHelper_h_GUID_9835023A_E639_409F_3A68_CFB4029FBBE7

// OpenVR header
#include <openvr_driver.h>

// PropertyHelper (and transitively, the generated traits header)
#include "PropertyHelper.h"

namespace osvr {
namespace vive {
    /// Pass a vr::Prop_... enum value as the template parameter, your tracked
    /// device as the parameter, and get back a pair containing the
    /// correctly-typed result and your error code.
    ///
    /// @sa generic::getProperty()
    template <std::size_t EnumVal>
    inline detail::EnumGetterReturn<EnumVal>
    getProperty(vr::ITrackedDeviceServerDriver *dev) {
        return generic::getProperty<EnumVal>(dev);
    }

    /// @overload
    /// Takes a Props:: enum class value as template parameter instead.
    ///
    /// @sa generic::getProperty()
    template <Props EnumVal>
    inline detail::EnumClassGetterReturn<EnumVal>
    getProperty(vr::ITrackedDeviceServerDriver *dev) {
        return generic::getProperty<EnumVal>(dev);
    }

    /// @overload
    /// @sa getPropertyOfType()
    /// @sa generic::getPropertyOfType()
    ///
    /// This overload would be ambiguous with variadics, but here, it's
    /// perfectly fine.
    template <typename T>
    inline detail::PropertyGetterReturn<T>
    getProperty(vr::ITrackedDeviceServerDriver *dev,
                vr::ETrackedDeviceProperty prop) {
        return generic::getPropertyOfType<T>(dev, prop);
    }
    /// @overload
    /// @sa getPropertyOfType()
    /// @sa generic::getPropertyOfType()
    ///
    /// This overload would be ambiguous with variadics, but here, it's
    /// perfectly fine.
    template <typename T>
    inline detail::PropertyGetterReturn<T>
    getProperty(vr::ITrackedDeviceServerDriver *dev, Props prop) {
        return generic::getPropertyOfType<T>(dev, prop);
    }
    /// For when you must pass the enum at runtime - less safe - the template
    /// parameter is the type, then, and the parameters are the device and the
    /// property enum.
    ///
    /// @sa generic::getPropertyOfType()
    template <typename T>
    inline detail::PropertyGetterReturn<T>
    getPropertyOfType(vr::ITrackedDeviceServerDriver *dev,
                      vr::ETrackedDeviceProperty prop) {
        return generic::getPropertyOfType<T>(dev, prop);
    }
    /// @overload
    /// Takes a Props:: enum class value as a parameter instead.
    ///
    /// @sa generic::getPropertyOfType()
    template <typename T>
    inline detail::PropertyGetterReturn<T>
    getPropertyOfType(vr::ITrackedDeviceServerDriver *dev, Props prop) {
        return generic::getPropertyOfType<T>(dev, prop);
    }
} // namespace vive
} // namespace osvr

#endif // INCLUDED_ServerPropertyHelper_h_GUID_9835023A_E639_409F_3A68_CFB4029FBBE7
