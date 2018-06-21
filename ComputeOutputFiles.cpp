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
#include "ComputeOutputFiles.h"
#include "PathConfig.h"

// Library/third-party includes
#include <boost/filesystem.hpp>

// Standard includes
#include <iostream>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NO_MINMAX
#include <windows.h>
static inline std::string getBinaryLocation() {
    char buf[512] = {0};
    DWORD len = GetModuleFileNameA(nullptr, buf, sizeof(buf));
    std::string ret;
    if (0 != len) {
        ret.assign(buf, len);
    }
    return ret;
}
#else
/// @todo implement or share code from PluginHost
static inline std::string getBinaryLocation() { return std::string{}; }
#endif

namespace fs = boost::filesystem;
using boost::filesystem::exists;
using boost::filesystem::path;

namespace osvr {
namespace vive {
    /// Assumes this is an executable in bin/ or bin/BUILDCONFIGNAME
    static inline path getBinDir() {
        auto binPath = fs::canonical(getBinaryLocation());

        auto parentDir = binPath.branch_path();

#if defined(CMAKE_CFG_INTDIR)
        if (parentDir.has_leaf && parentDir.leaf() == CMAKE_CFG_INTDIR) {
            // strip the config name
            return parentDir.branch_path();
        }
#endif
        return parentDir;
    }

    static inline bool startsWith(std::string const &fullString,
                                  std::string const &prefix) {
        return fullString.find(prefix) == 0;
    }

    static inline path getConfigRootDir(path const &binDir) {
        auto configRootString = std::string{OSVR_CONFIG_ROOT};
        auto localBinDir = std::string{OSVRVIVE_BIN_DIR};
        if (startsWith(configRootString, localBinDir) &&
            configRootString.size() > localBinDir.size()) {
            /// config appears to be a subdir of bin... but to be sure...
            auto nextChar = configRootString[localBinDir.size()];
            if (nextChar == '\\' || nextChar == '/') {
                // OK, the next character was a path separator, so it's true.
                auto relConfigRootString =
                    configRootString.substr(localBinDir.size() + 1);
                auto configPath = binDir / path{relConfigRootString};
#if 0
                std::cout << "I think the config root path (nested inside bin "
                             "dir) is "
                          << configPath.string() << std::endl;
#endif
                return configPath;
            }
        }
        // OK, so we need to go up one more level, then go for the config root.
        auto configPath = binDir.parent_path() / path{configRootString};
#if 0
        std::cout << "I think the config root path (outside of bindir) is "
                  << configPath.string() << std::endl;
#endif
        return configPath;
    }

    OutputFullPaths
    computeOutputFiles(std::string const &displayDescriptorFilename,
                       std::string const &meshFilename) {
        OutputFullPaths ret;
        auto binDir = getBinDir();
        auto configRoot = getConfigRootDir(binDir);
        auto displaysDir = configRoot / path{"displays"};
        if (!exists(displaysDir)) {
            create_directories(displaysDir);
        }
        ret.displayDescriptorPath =
            (displaysDir / path{displayDescriptorFilename}).generic_string();
        ret.meshFilePath = (displaysDir / path{meshFilename}).generic_string();
        return ret;
    }

} // namespace vive
} // namespace osvr
