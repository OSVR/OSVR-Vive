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

#ifndef INCLUDED_DeviceHolder_h_GUID_58F7E011_8D87_49A3_FE26_0DB159405E77
#define INCLUDED_DeviceHolder_h_GUID_58F7E011_8D87_49A3_FE26_0DB159405E77

// Internal Includes
#include "ReturnValue.h"

// Library/third-party includes
#include <openvr_driver.h>

// Standard includes
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

namespace osvr {
namespace vive {

    /// Holds activated vr::ITrackedDeviceServerDriver pointers in an array
    /// according to the ID given at their activation time. For best results,
    /// leave all activation/deactivation to this class.
    class DeviceHolder {
      public:
        DeviceHolder() = default;
        ~DeviceHolder() {
            if (deactivateOnShutdown_) {
                deactivateAll();
            }
        }
        DeviceHolder(DeviceHolder const &) = delete;
        DeviceHolder &operator=(DeviceHolder const &) = delete;

        /// move constructible
        DeviceHolder(DeviceHolder &&other)
            : devices_(std::move(other.devices_)),
              deactivateOnShutdown_(other.deactivateOnShutdown_) {
            other.disableDeactivateOnShutdown();
        }

        /// move assignment
        DeviceHolder &operator=(DeviceHolder &&other) {
            if (&other == this) {
                return *this;
            }
            if (deactivateOnShutdown_) {
                deactivateAll();
            }
            devices_ = std::move(other.devices_);
            deactivateOnShutdown_ = other.deactivateOnShutdown_;
            other.disableDeactivateOnShutdown();
            return *this;
        }

        using IdReturnValue = ReturnValue<std::uint32_t, bool>;

        IdReturnValue
        addAndActivateDevice(vr::ITrackedDeviceServerDriver *dev) {
            /// check to make sure it's not null and not already in there
            if (!dev) {
                return IdReturnValue::makeError();
            }
            auto existing = findDevice(dev);
            if (existing) {
                /// @todo do we return an error or the existing location? This
                /// returns the existing location after re-activating.
                dev->Activate(existing.value);
                return existing;
            }
            auto newId = static_cast<std::uint32_t>(devices_.size());
            devices_.push_back(dev);
            dev->Activate(newId);
            return IdReturnValue::makeValue(newId);
        }

        /// Add and activate a device at a reserved id.
        IdReturnValue
        addAndActivateDeviceAt(vr::ITrackedDeviceServerDriver *dev,
                               std::uint32_t idx) {
            /// check to make sure it's not null and not already in there
            if (!dev) {
                return IdReturnValue::makeError();
            }
            auto existing = findDevice(dev);
            if (existing && existing.value != idx) {
                // if we already found it in there and it's not at the desired
                // index...
                return IdReturnValue(existing.value, false);
            }

            if (existing) {
                // well, in this case, we might need to just activate it again.
                dev->Activate(idx);
                return IdReturnValue::makeValue(idx);
            }

            if (!(idx < devices_.size())) {
                // OK, we need to reserve more room.
                reserveIds(idx + 1);
            }

            if (devices_[idx]) {
                // there's already somebody else there...
                return IdReturnValue::makeError();
            }

            /// Finally, if we made it through that, it's our turn.
            devices_[idx] = dev;
            dev->Activate(idx);

            return IdReturnValue::makeValue(idx);
        }

        /// Reserve the first n ids, if not already allocated, for manual
        /// allocation. (More like a non-shrinking resize() than reserve() in
        /// c++ standard container terminology, so don't let that confuse you)
        /// @return true if this action resulted in any actual addition of null
        /// entries to the device list.
        bool reserveIds(std::uint32_t n) {
            if (devices_.size() < n) {
                devices_.resize(n, nullptr);
                return true;
            }
            return false;
        }

        bool hasDeviceAt(const std::uint32_t idx) const {
            return (idx < devices_.size()) && (nullptr != devices_[idx]);
        }

        vr::ITrackedDeviceServerDriver &
        getDevice(const std::uint32_t idx) const {
            return *devices_.at(idx);
        }

        /// @return a (found, index) pair for a non-null device pointer.
        IdReturnValue findDevice(vr::ITrackedDeviceServerDriver *dev) {

            auto it = std::find(devices_.cbegin(), devices_.cend(), dev);
            if (devices_.cend() == it) {
                return IdReturnValue::makeError();
            }
            return IdReturnValue::makeValue(static_cast<std::uint32_t>(
                std::distance(devices_.cbegin(), it)));
        }

        /// @return the number of allocated/reserved ids
        std::size_t reservedIds() const { return devices_.size(); }

        /// @return the number of active devices (that is, those that were not
        /// deactivated through this container and thus non-nullptr)
        std::size_t numDevices() const {
            return std::count_if(begin(devices_), end(devices_),
                                 [](vr::ITrackedDeviceServerDriver *dev) {
                                     return dev != nullptr;
                                 });
        }

        /// @return false if there was no device there to deactivate.
        bool deactivate(std::uint32_t idx) {
            if (idx < devices_.size() && devices_[idx]) {
                devices_[idx]->Deactivate();
                devices_[idx] = nullptr;
                return true;
            }
            return false;
        }

        void deactivateAll() {
            for (auto &dev : devices_) {
                if (dev) {
                    dev->Deactivate();
                    dev = nullptr;
                }
            }
        }

        /// Set whether all devices should be deactivated on shutdown - defaults
        /// to true, so you might just want to set to false if, for instance,
        /// you deactivate and power off the devices on shutdown yourself.
        void disableDeactivateOnShutdown() { deactivateOnShutdown_ = false; }

        bool shouldDeactivateOnShutdown() const {
            return deactivateOnShutdown_;
        }

        /// Seriously, just avoid using this function and it will be OK.
        std::vector<vr::ITrackedDeviceServerDriver *> const &
        rawDeviceVectorAccess_NOT_RECOMMENDED_TODO_FIXME() const {
            return devices_;
        }

      private:
        bool deactivateOnShutdown_ = true;
        std::vector<vr::ITrackedDeviceServerDriver *> devices_;
    };

} // namespace vive
} // namespace osvr

#endif // INCLUDED_DeviceHolder_h_GUID_58F7E011_8D87_49A3_FE26_0DB159405E77
