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

#ifndef INCLUDED_DriverLoader_h_GUID_882F2FD5_F218_42BE_3088_31CF712EC455
#define INCLUDED_DriverLoader_h_GUID_882F2FD5_F218_42BE_3088_31CF712EC455

// Internal Includes
#include "InterfaceTraits.h"
#include "ReturnValue.h"
#include <osvr/Util/Logger.h>

// Library/third-party includes
// - none

// Standard includes
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>

namespace osvr {
namespace vive {
    struct CouldNotLoadDriverModule : std::runtime_error {
        CouldNotLoadDriverModule(const char *errString)
            : std::runtime_error("Could not load driver module: " +
                                 std::string(errString)) {}
        CouldNotLoadDriverModule()
            : std::runtime_error("Could not load driver module.") {}
    };

    struct CouldNotLoadEntryPoint : std::runtime_error {
        CouldNotLoadEntryPoint(const char *errString)
            : std::runtime_error(
                  "Could not load entry point function from driver: " +
                  std::string(errString)) {}
        CouldNotLoadEntryPoint()
            : std::runtime_error(
                  "Could not load entry point function from driver.") {}
    };

    struct CouldNotGetInterface : std::runtime_error {
        CouldNotGetInterface(int errCode)
            : std::runtime_error(
                  "Could not get interface from native SteamVR "
                  "driver - likely version mismatch. SteamVR error code: " +
                  std::to_string(errCode)) {}
    };

    struct DriverNotLoaded : std::logic_error {
        DriverNotLoaded()
            : std::logic_error("Could not get interface: driver not loaded.") {}
    };

    struct AlreadyCleaningUpAnInterface : std::logic_error {
        AlreadyCleaningUpAnInterface()
            : std::logic_error("Already responsible for cleaning up an "
                               "interface: can't clean up two.") {}
    };

    /// Used to load (and own the handle to) a SteamVR driver DLL, as well
    /// as retrieve the main entry point function and handle the its calling and
    /// casting.
    class DriverLoader {
      public:
        /// Factory function to make a driver loader.
        static std::unique_ptr<DriverLoader>
        make(std::string const &driverRoot, std::string const &driverFile);

        /// destructor - out of line to support unique_ptr-based pimpl.
        ~DriverLoader();

        /// non-copyable
        DriverLoader(DriverLoader const &) = delete;
        /// non-copy-assignable
        DriverLoader &operator=(DriverLoader const &) = delete;

        /// Could we load the driver?
        explicit operator bool() const;

        /// Template function to call the driver's main entry point with the
        /// right string and do the right casting. Returns the pointer and
        /// the error code in a pair.
        template <typename InterfaceType>
        ReturnValue<InterfaceType *, int> getInterface() const {
            static_assert(
                InterfaceExpectedFromEntryPointTrait<InterfaceType>::value,
                "Can only use this function for interface types expected to be "
                "provided by the driver entry point.");

            InterfaceType *ret = nullptr;
            int returnCode = 0;
            if (!(*this)) {
                /// We've been reset or could never load.
                return makeError<InterfaceType *>(ret, returnCode);
            }
            void *product =
                factory_(InterfaceNameTrait<InterfaceType>::get(), &returnCode);
            if (product) {
                ret = static_cast<InterfaceType *>(product);
            }
            return makeReturnValue<InterfaceType *>(ret, returnCode);
        }

        /// Similar to the above, except that it throws in case of failure,
        /// instead of returning an error code. Thus, the pointer returned is
        /// always non-null.
        template <typename InterfaceType>
        InterfaceType *getInterfaceThrowing() const {
            auto ret = getInterface<InterfaceType>();
            if (!(*this)) {
                /// we early-out
                throw DriverNotLoaded();
            }
            if (!ret) {
                throw CouldNotGetInterface(ret.errorCode);
            }
            return ret.value;
        }

        std::string const &getDriverRoot() const { return driverRoot_; }

        /// This object can execute Cleanup on one interface during its
        /// destruction. This will set that interface. Usually not called
        /// directly by a user.
        template <typename InterfaceType>
        void cleanupInterfaceOnDestruction(InterfaceType *iface) {
            if (cleanup_) {
                throw AlreadyCleaningUpAnInterface();
            }
            cleanup_ = [iface] { iface->Cleanup(); };
        }

        /// Unload the DLL.
        void reset();

      private:
        DriverLoader(std::string const &driverRoot,
                     std::string const &driverFile);
        using DriverFactory = void *(*)(const char *, int *);
        // typedef void *(DriverFactory)(const char *, int *);

        struct Impl;
        std::unique_ptr<Impl> impl_;
        std::string driverRoot_;
        DriverFactory factory_;
        std::function<void()> cleanup_;
        osvr::util::log::LoggerPtr logger_;
    };

} // namespace vive
} // namespace osvr

#endif // INCLUDED_DriverLoader_h_GUID_882F2FD5_F218_42BE_3088_31CF712EC455
