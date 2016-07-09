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

#ifndef INCLUDED_PointHelpers_h_GUID_E89C6FE5_4E67_44BC_9F19_7C8B17D749CD
#define INCLUDED_PointHelpers_h_GUID_E89C6FE5_4E67_44BC_9F19_7C8B17D749CD

// Internal Includes
// - none

// Library/third-party includes
#include <json/value.h>

// Standard includes
#include <array>
#include <utility>

namespace osvr {
namespace vive {
    namespace pointhelpers {
        template <typename T> inline Json::Value arrayToJson(T &&input) {
            Json::Value ret(Json::arrayValue);
            for (auto &&elt : input) {
                ret.append(elt);
            }
            return ret;
        }

        template <typename T, typename U>
        inline Json::Value argsToJsonArray(T &&a, U &&b) {
            Json::Value ret(Json::arrayValue);
            ret.append(std::forward<T>(a));
            ret.append(std::forward<U>(b));
            return ret;
        }

        inline Json::Value makeSample(Json::Value const &inUV,
                                      std::array<float, 2> const &outUV) {
            return argsToJsonArray(inUV, arrayToJson(outUV));
        }

        inline Json::Value makeSample(std::array<float, 2> const &inUV,
                                      std::array<float, 2> const &outUV) {
            return argsToJsonArray(arrayToJson(inUV), arrayToJson(outUV));
        }
    } // namespace pointhelpers
} // namespace vive
} // namespace osvr
#endif // INCLUDED_PointHelpers_h_GUID_E89C6FE5_4E67_44BC_9F19_7C8B17D749CD
