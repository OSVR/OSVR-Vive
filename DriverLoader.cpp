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
#include "DriverLoader.h"
#include "InterfaceTraits.h"
#include "SearchPathExtender.h"

// Library/third-party includes
#include <osvr/Util/PlatformConfig.h>

// Standard includes
#include <iostream>

#if defined(OSVR_WINDOWS)

#define WIN32_LEAN_AND_MEAN
#define NO_MINMAX
#include <windows.h>

#elif defined(OSVR_LINUX) || defined(OSVR_MACOSX)

#include <dlfcn.h>

#endif

namespace osvr {
namespace vive {

    static const auto ENTRY_POINT_FUNCTION_NAME = "HmdDriverFactory";

    struct DriverLoader::Impl {
/// Platform-specific handle to dynamic library
#if defined(OSVR_WINDOWS)
        HMODULE driver_ = nullptr;
#elif defined(OSVR_MACOSX) || defined(OSVR_LINUX)
        void *driver_ = nullptr;
#endif

        /// Destructor: should contain platform-specific code to unload dynamic
        /// library.
        ~Impl() {
#if defined(OSVR_WINDOWS)
            if (driver_) {
                FreeLibrary(driver_);
            }
#elif defined(OSVR_MACOSX) || defined(OSVR_LINUX)
            if (driver_) {
                dlclose(driver_);
            }
#endif
        }
    };

    /// Constructor should contain platform-specific code to load dynamic
    /// library to populate handle in pimpl struct and extract the entry point
    /// function pointer.
    DriverLoader::DriverLoader(std::string const &driverRoot,
                               std::string const &driverFile)
        : impl_(new Impl),
          logger_(osvr::util::log::make_logger("DriverLoader")) {
        /// Set the PATH to include the driver directory so it can
        /// find its deps.
        SearchPathExtender extender(driverRoot);
#if defined(OSVR_WINDOWS)
        impl_->driver_ = LoadLibraryA(driverFile.c_str());
        if (!impl_->driver_) {
            reset();
            throw CouldNotLoadDriverModule();
        }

        auto proc = GetProcAddress(impl_->driver_, ENTRY_POINT_FUNCTION_NAME);
        if (!proc) {
            reset();
            throw CouldNotLoadEntryPoint();
        }
        factory_ = reinterpret_cast<DriverFactory>(proc);
#elif defined(OSVR_LINUX) || defined(OSVR_MACOSX)
        impl_->driver_ = dlopen(driverFile.c_str(), RTLD_NOW | RTLD_GLOBAL);
        if (!impl_->driver_) {
            reset();
            throw CouldNotLoadDriverModule(dlerror());
        }

        auto proc = dlsym(impl_->driver_, ENTRY_POINT_FUNCTION_NAME);
        if (!proc) {
            reset();
            throw CouldNotLoadEntryPoint(dlerror());
        }
        factory_ = reinterpret_cast<DriverFactory>(proc);
#endif
    }

    std::unique_ptr<DriverLoader>
    DriverLoader::make(std::string const &driverRoot,
                       std::string const &driverFile) {
        std::unique_ptr<DriverLoader> ret(
            new DriverLoader(driverRoot, driverFile));
        return ret;
    }

    DriverLoader::~DriverLoader() { reset(); }

    DriverLoader::operator bool() const {
        /// Presence of a valid private impl object is equivalent to validity of
        /// the object overall.
        return static_cast<bool>(impl_);
    }

    void DriverLoader::reset() {
        if (cleanup_) {
            logger_->debug("osvr::vive::DriverLoader::reset() - cleaning up "
                           "main provider ");
            cleanup_();
            cleanup_ = std::function<void()>{};
        }
        if (impl_) {
            logger_->debug(
                "osvr::vive::DriverLoader::reset() - unloading driver");
            impl_.reset();
        }
    }
} // namespace vive
} // namespace osvr
