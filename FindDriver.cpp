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
#include "FindDriver.h"

// Library/third-party includes
#include <boost/iostreams/stream.hpp>
#include <json/reader.h>
#include <json/value.h>
#include <osvr/Util/PlatformConfig.h>

// Standard includes
#include <climits>
#include <fstream> // std::ifstream
#include <iostream>
#include <vector>

#if defined(OSVR_USING_FILESYSTEM_HEADER)
#include <filesystem>
#elif defined(OSVR_USING_BOOST_FILESYSTEM)
#include <boost/filesystem.hpp>
#endif

#if defined(OSVR_WINDOWS)
#include <shlobj.h>
#else
#include <cstdlib> // for getenv
#endif

#undef VIVELOADER_VERBOSE

namespace osvr {
namespace vive {
#if defined(OSVR_WINDOWS)
    static const auto PLATFORM_DIRNAME_BASE = "win";
    static const auto DRIVER_EXTENSION = ".dll";
    static const auto TOOL_EXTENSION = ".exe";
    static const auto PATH_SEP = "\\";
#elif defined(OSVR_MACOSX)
    /// @todo Note that there are no 64-bit steamvr runtimes or drivers on OS X
    static const auto PLATFORM_DIRNAME_BASE = "osx";
    static const auto DRIVER_EXTENSION = ".dylib";
    static const auto TOOL_EXTENSION = "";
    static const auto PATH_SEP = "/";
#elif defined(OSVR_LINUX)
    static const auto PLATFORM_DIRNAME_BASE = "linux";
    static const auto DRIVER_EXTENSION = ".so";
    static const auto TOOL_EXTENSION = "";
    static const auto PATH_SEP = "/";
#else
#error "Sorry, Valve does not produce a SteamVR runtime for your platform."
#endif

#if defined(OSVR_USING_FILESYSTEM_TR2)
    using std::tr2::sys::exists;
    using std::tr2::sys::path;
    using std::tr2::sys::wpath;
#elif defined(OSVR_USING_FILESYSTEM_EXPERIMENTAL)
    using std::experimental::filesystem::path;
#ifdef _WIN32
    /// Some Windows functions deal in wide strings, easier to just let it cope
    /// with it.
    using wpath = path;
#endif
    using std::experimental::filesystem::exists;
#elif defined(OSVR_USING_BOOST_FILESYSTEM)
    using boost::filesystem::exists;
    using boost::filesystem::path;
#endif

#ifdef OSVR_MACOSX
    inline std::string getPlatformBitSuffix() {
        // they use universal binaries but stick them in "32"
        return "32";
    }
#else
    inline std::string getPlatformBitSuffix() {
        return std::to_string(sizeof(void *) * CHAR_BIT);
    }
#endif

    inline std::string getPlatformDirname() {
        return PLATFORM_DIRNAME_BASE + getPlatformBitSuffix();
    }

    inline void parsePathConfigFile(std::istream &is, Json::Value &ret) {
        Json::Reader reader;
        if (!reader.parse(is, ret)) {
            std::cerr << "Error parsing file containing path configuration - "
                         "have you run SteamVR yet?"
                      << std::endl;
            std::cerr << reader.getFormattedErrorMessages() << std::endl;
        }
    }

#if defined(OSVR_WINDOWS)
    inline Json::Value getPathConfig() {
        PWSTR outString = nullptr;
        Json::Value ret;
        // It's OK to use Vista+ stuff here, since openvr_api.dll uses Vista+
        // stuff too.
        auto hr =
            SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &outString);
        if (!SUCCEEDED(hr)) {
            std::cerr << "Could not get local app data directory!" << std::endl;
            return ret;
        }
        // Free the string returned when we're all done.
        /// @todo Shouldn't this be in a "finally()" call?
        auto freeString = [&] { CoTaskMemFree(outString); };
        // Build the path to the file.
        auto vrPaths =
            wpath(outString) / wpath(L"openvr") / wpath(L"openvrpaths.vrpath");
        std::ifstream is(vrPaths.string());
        if (!is) {
            std::wcerr << L"Could not open file containing path configuration "
                          L"- have you run SteamVR yet? "
                       << vrPaths << L"\n";
            return ret;
        }
        parsePathConfigFile(is, ret);
        return ret;
    }

#elif defined(OSVR_LINUX)
    inline Json::Value getPathConfig() {
        auto home = std::getenv("HOME");
        path homePath =
            (nullptr == home
                 ? path{"~"}
                 /*that's weird, should have been in environment...*/
                 : path{home});
        auto vrPaths = homePath / path{".config"} / path{"openvr"} /
                       path{"openvrpaths.vrpath"};

        std::ifstream is(vrPaths.string());
        Json::Value ret;
        if (!is) {
            std::cerr << "Could not open file containing path configuration "
                         "- have you run SteamVR yet? "
                      << vrPaths << "\n";
            return ret;
        }
        parsePathConfigFile(is, ret);
        return ret;
    }
#elif defined(OSVR_MACOSX)
    inline Json::Value getPathConfig() {
        auto home = std::getenv("HOME");
        path homePath =
            (nullptr == home
                 ? path{"~"}
                 /*that's weird, should have been in environment...*/
                 : path{home});
        auto vrPaths = homePath / path{".openvr"} / path{"openvrpaths.vrpath"};

        std::ifstream is(vrPaths.string());
        Json::Value ret;
        if (!is) {
            std::cerr << "Could not open file containing path configuration "
                         "- have you run SteamVR yet? "
                      << vrPaths << "\n";
            return ret;
        }
        parsePathConfigFile(is, ret);
        return ret;
    }
#endif

    inline std::vector<std::string> getSteamVRRoots(Json::Value const &json) {
        std::vector<std::string> ret;
        auto &runtimes = json["runtime"];
        if (!runtimes.isArray()) {
            return ret;
        }
        for (auto &runtime : runtimes) {
            ret.emplace_back(runtime.asString());
        }
        return ret;
    }

    inline std::vector<std::string> getSteamVRRoots() {
        return getSteamVRRoots(getPathConfig());
    }

    inline void computeDriverRootAndFilePath(DriverLocationInfo &info,
                                             std::string const &driver) {
        auto p = path{info.steamVrRoot};
        p /= "drivers";
        p /= driver;
        p /= "bin";
        p /= getPlatformDirname();
        info.driverRoot = p.string();
        p /= ("driver_" + driver + DRIVER_EXTENSION);
        info.driverFile = p.string();
    }

    /// Underlying implementation - hand it the preloaded json.
    inline DriverLocationInfo findDriver(Json::Value const &json,
                                         std::string const &driver) {

        DriverLocationInfo info;
        auto &runtimes = json["runtime"];
        if (!runtimes.isArray()) {
            return info;
        }

        for (auto &root : runtimes) {
            info.steamVrRoot = root.asString();
            info.driverName = driver;

            computeDriverRootAndFilePath(info, driver);

#ifdef VIVELOADER_VERBOSE
            std::cout << "Will try to load driver from:\n"
                      << info.driverFile << std::endl;
            if (exists(path{info.driverRoot})) {
                std::cout << "Driver root exists" << std::endl;
            }
            if (exists(path{info.driverFile})) {
                std::cout << "Driver file exists" << std::endl;
            }
#endif

            if (exists(path{info.driverRoot}) &&
                exists(path{info.driverFile})) {
                info.found = true;
                return info;
            }
        }
        info = DriverLocationInfo{};
        return info;
    }

    DriverLocationInfo findDriver(std::string const &driver) {
        return findDriver(getPathConfig(), driver);
    }

    std::string getToolLocation(std::string const &toolName,
                                std::string const &steamVrRoot) {
        std::vector<std::string> searchPath;
        if (!steamVrRoot.empty()) {
            searchPath = {steamVrRoot};
        } else {
            searchPath = getSteamVRRoots();
        }

        for (auto &root : searchPath) {
            auto p = path{root};
            p /= "bin";
            p /= getPlatformDirname();
            p /= (toolName + TOOL_EXTENSION);
            if (exists(p)) {
                return p.string();
            }
        }
        return std::string{};
    }

    /// Underlying implementation - hand it the preloaded json.
    inline ConfigDirs findConfigDirs(Json::Value const &json,
                                     std::string const &driver) {
        ConfigDirs ret;
        auto const &configLocations = json["config"];
        if (!configLocations.isArray()) {
            return ret;
        }
        for (auto &configDir : configLocations) {
            auto configPath = path{configDir.asString()};
            if (!exists(configPath)) {
                continue;
            }
            ret.rootConfigDir = configPath.string();
            ret.driverConfigDir = (configPath / path{driver}).string();
            ret.valid = true;
            return ret;
        }
        ret = ConfigDirs{};
        return ret;
    }
#if 0
    ConfigDirs findConfigDirs(std::string const & /*steamVrRoot*/,
                              std::string const &driver) {
        return findConfigDirs(getPathConfig(), driver);
    }
#endif

    LocationInfo findLocationInfoForDriver(std::string const &driver) {
        auto json = getPathConfig();
        LocationInfo ret;

        auto config = findConfigDirs(json, driver);
        if (config.valid) {
            ret.configFound = true;
            ret.rootConfigDir = config.rootConfigDir;
            ret.driverConfigDir = config.driverConfigDir;
        }
        auto driverLoc = findDriver(json, driver);
        if (driverLoc.found) {
            ret.driverFound = true;
            ret.steamVrRoot = driverLoc.steamVrRoot;
            ret.driverRoot = driverLoc.driverRoot;
            ret.driverName = driverLoc.driverName;
            ret.driverFile = driverLoc.driverFile;
        }
        ret.found = (driverLoc.found && config.valid);
        return ret;
    }

} // namespace vive
} // namespace osvr
