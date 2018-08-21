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

// Library/third-party includes
#include <json/reader.h>
#include <json/value.h>

#include <osvr/Util/Finally.h>

// Standard includes
#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NO_MINMAX
#include <windows.h>

static inline std::string formatLastErrorAsString() {
    char *lpMsgBuf = nullptr;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                   nullptr, GetLastError(),
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   // Default language
                   reinterpret_cast<LPSTR>(&lpMsgBuf), 0, nullptr);
    /// Free that buffer when we're out of scope.
    auto cleanupBuf = osvr::util::finally([&] { LocalFree(lpMsgBuf); });
    auto errorMessage = std::string(lpMsgBuf);
    return errorMessage;
}
static inline std::string
getFile(std::string const &fn,
        std::function<void(std::string const &)> const &errorReport) {
    /// Had trouble with "permission denied" errors using standard C++ iostreams
    /// on the chaperone data, so had to go back down to Win32 API.
    HANDLE f = CreateFileA(fn.c_str(), GENERIC_READ,
                           FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (INVALID_HANDLE_VALUE == f) {
        errorReport("Could not open file: " + formatLastErrorAsString());
        return std::string{};
    }
    auto closer = osvr::util::finally([&f] { CloseHandle(f); });

    std::ostringstream os;
    static const auto BUFSIZE = 1024;
    std::array<char, BUFSIZE + 1> buf;
    /// Null terminate for ease of use.
    buf.back() = '\0';
    bool keepReading = true;
    while (1) {
        DWORD bytesRead;
        auto ret = ReadFile(f, buf.data(), static_cast<DWORD>(buf.size() - 1),
                            &bytesRead, nullptr);
        if (ret) {
            if (bytesRead == 0) {
                // end of file
                break;
            } else {
                // Null terminate this block and slide it in.
                // std::cout << "read " << bytesRead << " bytes this time "
                //          << std::endl;
                buf[bytesRead] = '\0';
                os << buf.data();
            }
        } else {

            errorReport("Error after reading " +
                        std::to_string(os.str().size()) +
                        " bytes: " + formatLastErrorAsString());
            return std::string{};
        }
    }
    return os.str();
}
#else
#include <cstring> // strerror
#include <utility>

std::string
getFile(std::string const &fn,
        std::function<void(std::string const &)> const &errorReport) {
    std::ifstream s(fn, std::ios::in | std::ios::binary);
    if (!s) {
        std::ostringstream os;
        os << "Could not open file ";

        /// Sadly errno is far more useful in its error messages than
        /// failbit-triggered exceptions, etc.
        auto theErrno = errno;
        os << " (Error code: " << theErrno << " - " << strerror(theErrno)
           << ")";
        errorReport(os.str());
        return std::string{};
    }
    std::ostringstream os;
    std::string temp;
    while (std::getline(s, temp)) {
        os << temp;
    }
    return os.str();
}
#endif

namespace osvr {
namespace vive {
    static const auto PREFIX = "[ChaperoneData] ";
    static const auto CHAPERONE_DATA_FILENAME = "chaperone_info.vrchap";
#ifdef _WIN32
    static const auto PATH_SEPARATOR = "\\";
#else
    static const auto PATH_SEPARATOR = "/";
#endif

    using UniverseDataMap =
        std::map<std::uint64_t, ChaperoneData::UniverseData>;

    using UniverseBaseSerials = std::vector<
        std::pair<std::uint64_t, ChaperoneData::BaseStationSerials>>;
    struct ChaperoneData::Impl {
        Json::Value chaperoneInfo;
        UniverseDataMap universes;
        UniverseBaseSerials baseSerials;
    };

    void loadJsonIntoUniverseData(Json::Value const &obj,
                                  ChaperoneData::UniverseData &data) {
        data.yaw = obj["yaw"].asDouble();
        auto &xlate = obj["translation"];
        for (Json::Value::ArrayIndex i = 0; i < 3; ++i) {
            data.translation[i] = xlate[i].asDouble();
        }
    }
    ChaperoneData::ChaperoneData(std::string steamConfigDir)
        : impl_(new Impl), configDir_(std::move(steamConfigDir)) {
        {
            Json::Reader reader;
            auto chapInfoFn =
                configDir_ + PATH_SEPARATOR + CHAPERONE_DATA_FILENAME;
#if 0
            std::ifstream chapInfoFile(configDir_,
                                       std::ios::in | std::ios::binary);
            if (!chapInfoFile) {
                std::ostringstream os;
                os << "Could not open chaperone info file, expected at "
                   << chapInfoFn;

                /// Sadly errno is far more useful in its error messages than
                /// failbit-triggered exceptions, etc.
                auto theErrno = errno;
                os << " (Error code: " << theErrno << " - "
                   << strerror(theErrno) << ")";
                errorOut_(os.str());
                return;
            }

            if (!chapInfoFile.good()) {
                errorOut_("Could not open chaperone info file, expected at " +
                          chapInfoFn);
                return;
            }
#endif
            std::string fileData =
                getFile(chapInfoFn, [&](std::string const &message) {
                    std::ostringstream os;
                    os << "Could not open chaperone info file, expected at "
                       << chapInfoFn;
                    os << " - details [" << message << "]";
                    errorOut_(os.str());
                });
            if (!valid()) {
                /// this means our fail handler got called.
                return;
            }
            if (!reader.parse(fileData, impl_->chaperoneInfo)) {
                errorOut_("Could not parse JSON in chaperone info file at " +
                          chapInfoFn + ": " +
                          reader.getFormattedErrorMessages());
                return;
            }

            /// Basic sanity checks
            if (impl_->chaperoneInfo["jsonid"] != "chaperone_info") {
                errorOut_("Chaperone info file at " + chapInfoFn +
                          " did not match expected format (no element "
                          "\"jsonid\": \"chaperone_info\" in top level "
                          "object)");
                return;
            }

            if (impl_->chaperoneInfo["universes"].empty()) {
                errorOut_("Chaperone info file at " + chapInfoFn +
                          " did not contain any known chaperone universes - "
                          "user must run Room Setup at least once");
                return;
            }
        }

        for (auto const &univ : impl_->chaperoneInfo["universes"]) {
            auto univIdString = univ["universeID"].asString();
            UniverseData data;
            auto &standing = univ["standing"];
            if (standing.isNull()) {
                warn_("No standing calibration data for universe " +
                      univIdString + ", so had to look for seated data.");
                auto &seated = univ["seated"];
                if (seated.isNull()) {
                    warn_(
                        "No seated or standing calibration data for universe " +
                        univIdString + ", so had to skip it.");
                    continue;
                }
                data.type = CalibrationType::Seated;
                loadJsonIntoUniverseData(seated, data);

            } else {
                data.type = CalibrationType::Standing;
                loadJsonIntoUniverseData(standing, data);
            }
            /// Convert universe ID (64-bit int) from string, in JSON, to an int
            /// again.
            UniverseId id;
            std::istringstream is(univIdString);
            is >> id;
            /// Add the universe data in.
            impl_->universes.insert(std::make_pair(id, data));

            BaseStationSerials serials;
            for (auto const &tracker : univ["trackers"]) {
                auto &serial = tracker["serial"];
                if (serial.isString()) {
                    serials.push_back(serial.asString());
                }
            }

            /// Add the serial data in.
            impl_->baseSerials.emplace_back(id, std::move(serials));
        }
    }

    ChaperoneData::~ChaperoneData() = default;

    bool ChaperoneData::valid() const { return static_cast<bool>(impl_); }

    bool ChaperoneData::knowUniverseId(UniverseId universe) const {
        if (0 == universe) {
            return false;
        }
        return (impl_->universes.find(universe) != end(impl_->universes));
    }

    ChaperoneData::UniverseData
    ChaperoneData::getDataForUniverse(UniverseId universe) const {
        auto it = impl_->universes.find(universe);
        if (it == end(impl_->universes)) {
            return {};
        }
        return it->second;
    }

    std::size_t ChaperoneData::getNumberOfKnownUniverses() const {
        return impl_->universes.size();
    }

    ChaperoneData::UniverseId ChaperoneData::guessUniverseIdFromBaseStations(
        BaseStationSerials const &bases) {
        auto providedSize = bases.size();
        UniverseId ret = 0;
        using UniverseRank = std::pair<float, UniverseId>;
        /// Compare function for heap.
        auto compare = [](UniverseRank const &a, UniverseRank const &b) {
            return a.first < b.first;
        };

        /// Will contain heap of potential universes and their value (a fraction
        /// of their base stations and provided base stations that were included
        /// in the base station list provided to the function)
        std::vector<UniverseRank> potentialUniverses;
        auto push = [&](float value, UniverseId id) {
            potentialUniverses.emplace_back(value, id);
            std::push_heap(begin(potentialUniverses), end(potentialUniverses),
                           compare);
        };

        for (auto &univBaseSerial : impl_->baseSerials) {
            auto const &baseSerials = univBaseSerial.second;
            std::size_t hits = 0;
            auto b = begin(baseSerials);
            auto e = end(baseSerials);
            /// Count the number of entries that we were given that are also in
            /// this universe's list.
            auto found = std::count_if(begin(bases), end(bases),
                                       [&](std::string const &needle) {
                                           return std::find(b, e, needle) != e;
                                       });
            if (found > 0) {
                /// This is meant to combine the influence of "found" in both
                /// providedSize and universe size, and the +1 in the
                /// denominator is to avoid division by zero.
                auto weight = 2.f * static_cast<float>(found) /
                              (baseSerials.size() + providedSize + 1);
#if 0
                std::cout << "Guessing produced weight of " << weight << " for "
                          << univBaseSerial.first << std::endl;
#endif
                push(weight, univBaseSerial.first);
            }
        }
        if (!potentialUniverses.empty()) {
            /// it's a heap, so the best one is always on top.
            return potentialUniverses.front().second;
        }
        return ret;
    }

    void ChaperoneData::errorOut_(std::string const &message) {
        /// This reset may be redundant in some cases, but better to not miss
        /// it, since it's also our error flag.
        impl_.reset();

        warn_("ERROR: " + message);
    }

    void ChaperoneData::warn_(std::string const &message) {

        if (err_.empty()) {
            /// First error
            err_ = message;
            return;
        }

        static const auto BEGIN_LIST = "[";
        static const auto BEGIN_LIST_CH = BEGIN_LIST[0];
        static const auto END_LIST = "]";
        static const auto MIDDLE_LIST = "][";
        if (BEGIN_LIST_CH == err_.front()) {
            /// We've already started a list of errors, just tack one more on.
            err_ += BEGIN_LIST + message + END_LIST;
            return;
        }

        /// OK, so this is our exactly second error, wrap the first and second.
        err_ = BEGIN_LIST + err_ + MIDDLE_LIST + message + END_LIST;
    }

} // namespace vive
} // namespace osvr
