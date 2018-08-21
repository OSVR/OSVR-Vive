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
#include "RGBPoints.h"
#include "PointHelpers.h"

// Library/third-party includes
#include <json/value.h>
#include <json/writer.h>

// Standard includes
// - none

namespace osvr {
namespace vive {
    static const std::array<std::string, 3> COLOR_NAMES = {
        "red_point_samples", "green_point_samples", "blue_point_samples"};
    struct RGBPoints::Impl {
        Impl()
            : root(Json::objectValue),
              distortion_(&(root["display"]["hmd"]["distortion"] =
                                Json::Value(Json::objectValue))) {
            // So inside the distortion object, there are named arrays for each
            // color. they each contain an array for each eye, and in each eye
            // array goes each [ [inu, inv], [outu, outv] ] pair.
            for (auto colorIdx : {Red, Green, Blue}) {
                /// Create the array for this color
                colors_[colorIdx] = &(distortion()[COLOR_NAMES[colorIdx]] =
                                          Json::Value(Json::arrayValue));
                /// create its first (left) eye array
                colors_[colorIdx]->append(Json::Value(Json::arrayValue));
                /// create its second (right) eye array
                colors_[colorIdx]->append(Json::Value(Json::arrayValue));
            }
        }

        Json::Value root;

        Json::Value &distortion() { return *distortion_; }

        Json::Value &getColorAndEye(Eye eye, size_t color) {
            return (*colors_[color])[static_cast<int>(eye)];
        }

        void addSample(Eye eye, size_t color, Json::Value const &inputUV,
                       Point2 const &outUV) {
            getColorAndEye(eye, color)
                .append(pointhelpers::makeSample(inputUV, outUV));
        }

        void addSample(Eye eye, size_t color, Point2 const &inputUV,
                       Point2 const &outUV) {
            getColorAndEye(eye, color)
                .append(pointhelpers::makeSample(inputUV, outUV));
        }

        void addSample(Eye eye, Point2 const &inputUV, Point2 const &outR,
                       Point2 const &outG, Point2 const &outB) {
            auto inUV = pointhelpers::arrayToJson(inputUV);
            addSample(eye, Red, inUV, outR);
            addSample(eye, Green, inUV, outG);
            addSample(eye, Blue, inUV, outB);
        }

        Json::Value *distortion_ = nullptr;
        std::array<Json::Value *, 3> colors_{};
    };

    RGBPoints::RGBPoints() : impl_(new Impl) {}
    RGBPoints::~RGBPoints() = default;
    void RGBPoints::addSample(Eye eye, Point2 const &inputUV,
                              Point2 const &outR, Point2 const &outG,
                              Point2 const &outB) {
        impl_->addSample(eye, inputUV, outR, outG, outB);
    }
    std::string RGBPoints::getSeparateFile() const {
        Json::FastWriter writer;
        return writer.write(impl_->root);
    }
    std::string RGBPoints::getSeparateFileStyled() const {
        return impl_->root.toStyledString();
    }
} // namespace vive
} // namespace osvr
