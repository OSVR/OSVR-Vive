/** @file
    @brief Header

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

#ifndef INCLUDED_RGBPoints_h_GUID_6B6CB721_76B9_43AF_C29B_4D1BBA5FEDD0
#define INCLUDED_RGBPoints_h_GUID_6B6CB721_76B9_43AF_C29B_4D1BBA5FEDD0

// Internal Includes
// - none

// Library/third-party includes
// - none

// Standard includes
#include <array>
#include <memory>
#include <string>

namespace osvr {
namespace vive {

    class RGBPoints {
      public:
        RGBPoints();
        ~RGBPoints();

        enum { Red = 0, Green = 1, Blue = 2 };

        enum class Eye { Left = 0, Right = 1 };
        using Point2 = std::array<float, 2>;

        void addSample(Eye eye, Point2 const &inputUV, Point2 const &outR,
                       Point2 const &outG, Point2 const &outB);

        std::string getSeparateFile() const;
        std::string getSeparateFileStyled() const;

      private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

} // namespace vive
} // namespace osvr

#endif // INCLUDED_RGBPoints_h_GUID_6B6CB721_76B9_43AF_C29B_4D1BBA5FEDD0
