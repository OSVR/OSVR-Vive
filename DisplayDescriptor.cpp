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
#include "DisplayDescriptor.h"
#include "osvr/Util/PlatformConfig.h"

// Library/third-party includes
#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>

// Standard includes
#include <algorithm>
#include <cfloat> // FLT_EPSILON
#include <cmath>
#include <iostream>

namespace osvr {
namespace vive {

    static const auto PREFIX = "[DisplayDescriptor] ";

    template <typename T> inline T radiansToDegrees(T const &rads) {
        return static_cast<T>(rads * 180 / M_PI /*3.141592653589*/);
    }

    inline float clipPlaneToHalfFov(float d) {
        return radiansToDegrees(std::atan(std::abs(d)));
    }

    /// Given clipping planes at unit distance, return the half-field of views
    /// in degrees.
    HalfFieldsOfViewDegrees clipPlanesToHalfFovs(UnitClippingPlane const &r) {
        HalfFieldsOfViewDegrees ret;
        ret.left = clipPlaneToHalfFov(r.left);
        ret.right = clipPlaneToHalfFov(r.right);
        ret.top = clipPlaneToHalfFov(r.top);
        ret.bottom = clipPlaneToHalfFov(r.bottom);
        return ret;
    }

    inline bool approxEqual(float a, float b, float maxDiff,
                            float maxRelDiff = FLT_EPSILON) {
        /// Inspired by Bruce Dawson's AlmostEqualRelativeAndAbs
        /// https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
        auto diff = std::abs(a - b);
        if (diff <= maxDiff) {
            return true;
        }
        auto larger = std::max(std::abs(a), std::abs(b));
        return diff <= larger * maxRelDiff;
    }

    std::pair<bool, Fovs>
    twoEyeFovsToMonoWithOverlap(HalfFieldsOfViewDegrees const &leftFov,
                                HalfFieldsOfViewDegrees const &rightFov,
                                const bool verbose) {
        Fovs ret;

        /// Lambda for spitting an error message, and returning an error state
        /// and the partial answer we had, for more concise error handling.
        auto withError = [&](const char *msg) {
            if (verbose) {
                std::cerr << "***ERROR: Eyes are " << msg
                          << " - cannot be represented in this display schema!"
                          << std::endl;
            }
            return std::make_pair(false, ret);
        };

        // lambda sets a max absolute difference for angles in degrees to be
        // considered equal in this entire function.
        auto notEqual = [](float a, float b) {
            return !approxEqual(a, b, 0.25);
        };

        if (verbose) {
            std::cout << PREFIX << "left: " << leftFov << std::endl;
            std::cout << PREFIX << "right: " << rightFov << std::endl;
        }

        if (notEqual(leftFov.left, rightFov.right) ||
            notEqual(rightFov.left, leftFov.right)) {
            return withError(
                "not symmetrical in their frusta across the YZ plane");
        }

        ret.monoHoriz = leftFov.left + leftFov.right;
        if (notEqual(static_cast<float>(ret.monoHoriz),
                     (rightFov.left + rightFov.right))) {
            return withError("not matching in horizontal field of view");
        }
        if (notEqual(leftFov.top, rightFov.top) ||
            notEqual(leftFov.bottom, rightFov.bottom)) {
            return withError(
                "not symmetrical in their vertical half fields of view");
        }
        ret.monoVert = leftFov.top + leftFov.bottom;
#if 0
        // check is redundant
        if (ret.monoVert != (rightFov.top + rightFov.bottom)) {
            return withError("not matching in vertical field of view");
        }
#endif

        {
            /// Related to the code in helper.cpp in angles_to_config.
            auto horizHalfFov = ret.monoHoriz / 2.;
            auto rotateEyesApart = leftFov.left - horizHalfFov;
            auto overlapFrac = 1. - 2. * rotateEyesApart / ret.monoHoriz;
            ret.overlapPercent = overlapFrac * 100;
        }
        ret.pitch = leftFov.bottom - (ret.monoVert / 2.);
        return std::make_pair(true, ret);
    }

    template <typename T> inline void averageInPlace(T &a, T &b) {
        auto avg = (a + b) / T(2);
        a = avg;
        b = avg;
    }

    void averageAndSymmetrize(HalfFieldsOfViewDegrees &leftFov,
                              HalfFieldsOfViewDegrees &rightFov) {
        // inner
        averageInPlace(leftFov.right, rightFov.left);
        // outer
        averageInPlace(leftFov.left, rightFov.right);
        // top
        averageInPlace(leftFov.top, rightFov.top);
        // bottom
        averageInPlace(leftFov.bottom, rightFov.bottom);
    }

    struct DisplayDescriptor::Impl {
        Json::Value descriptor;
        Json::Value &get(const char *member) {
            return descriptor["hmd"][member];
        }
    };

    DisplayDescriptor::DisplayDescriptor(
        std::string const &templateDisplayDescriptor)
        : impl_(new Impl) {
        /// Load the starting point for the display descriptor.
        Json::Reader reader;
        if (!reader.parse(templateDisplayDescriptor, impl_->descriptor)) {
            std::cerr
                << PREFIX
                << "Could not parse template for display descriptor. Error: "
                << reader.getFormattedErrorMessages() << std::endl;
        }
        valid_ = true;
    }

    DisplayDescriptor::~DisplayDescriptor() = default;
    void DisplayDescriptor::updateFovs(Fovs const &fovs) {
        Json::Value &fov = impl_->get("field_of_view");
        fov["monocular_horizontal"] = fovs.monoHoriz;
        fov["monocular_vertical"] = fovs.monoVert;
        fov["overlap_percent"] = fovs.overlapPercent;
        fov["pitch_tilt"] = fovs.pitch;
    }

    void DisplayDescriptor::updateCenterOfProjection(
        std::size_t eyeIndex, std::array<float, 2> const &center) {
        if (eyeIndex > 1) {
            return;
        }

        Json::Value &eye =
            impl_->get("eyes")[static_cast<Json::ArrayIndex>(eyeIndex)];
        eye["center_proj_x"] = center[0];
        eye["center_proj_y"] = center[1];
    }

    void DisplayDescriptor::updateCentersOfProjection(
        std::array<float, 2> const &left, std::array<float, 2> const &right) {
        updateCenterOfProjection(LeftEye, left);
        updateCenterOfProjection(RightEye, right);
    }

    void DisplayDescriptor::setResolution(std::uint32_t width,
                                          std::uint32_t height) {
        Json::Value &firstRes = impl_->get("resolutions")[0];
        firstRes["width"] = width;
        firstRes["height"] = height;
    }

    void DisplayDescriptor::setRGBMeshExternalFile(std::string const &fn) {
        Json::Value &distortion = impl_->get("distortion");
        distortion["rgb_point_samples_external_file"] = fn;
    }

    void DisplayDescriptor::setVendor(std::string const &vendor) {
        Json::Value &dev = impl_->get("device");
        dev["vendor"] = vendor;
    }

    void DisplayDescriptor::setModel(std::string const &model) {
        Json::Value &dev = impl_->get("device");
        dev["model"] = model;
    }

    void DisplayDescriptor::setNote(std::string const &note) {
        Json::Value &dev = impl_->get("device");
        dev["Note"] = note;
    }

    std::string DisplayDescriptor::getDescriptor() const {
        return impl_->descriptor.toStyledString();
    }

} // namespace vive
} // namespace osvr
