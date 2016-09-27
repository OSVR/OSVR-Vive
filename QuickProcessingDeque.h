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

#ifndef INCLUDED_QuickProcessingDeque_h_GUID_B6819891_863F_4B8A_9024_C0E42E1D21AA
#define INCLUDED_QuickProcessingDeque_h_GUID_B6819891_863F_4B8A_9024_C0E42E1D21AA

// Internal Includes
#include "VerifyLocked.h"

// Library/third-party includes
// - none

// Standard includes
#include <cstddef>
#include <deque>
#include <mutex>
#include <utility>
#include <vector>

namespace osvr {
namespace vive {

    /// A container wrapping a deque (mutex-controlled) and a vector (for main
    /// thread use only), where work is submitted to the deque, then the main
    /// thread, upon entering, grabs a number of items from the deque to the
    /// vector before beginning work on them, allowing shorter lock times.
    template <typename T, typename LockType = std::lock_guard<std::mutex>>
    class QuickProcessingDeque {
      public:
        using value_type = T;
        using deque_type = std::deque<T>;
        using vector_type = std::vector<T>;
        using lock_type = LockType;

        /// Call from the async thread you can't control
        /// Must hold the lock.
        void submitNew(value_type const &v, lock_type &lock) {
            if (verifyLocked<lock_type>(lock)) {
                deque_.push_back(v);
            }
        }

        void submitNew(value_type &&v, lock_type &lock) {
            if (verifyLocked<lock_type>(lock)) {
                deque_.emplace_back(std::move(v));
            }
        }

        /// Call from the main thread to grab a chunk of work to deal with.
        /// Must hold the lock.
        std::size_t grabItems(lock_type &lock) {
            if (verifyLocked<lock_type>(lock)) {
                clearWorkItems();
                /// Copy a fixed number of reports that have been queued up.
                auto numItems = deque_.size();
                for (std::size_t i = 0; i < numItems; ++i) {
                    vector_.push_back(deque_.front());
                    deque_.pop_front();
                }
                return numItems;
            }
            return 0;
        }

        /// Call from the main thread, after calling grabItems then releasing
        /// the lock, to get access to the items you just grabbed. (Cleared
        /// automatically every call to grabItems)
        vector_type const &accessWorkItems() const { return vector_; }

        /// Not necessary, since it's called at the beginning of each grabItems,
        /// but if you really want to reduce time in lock...
        void clearWorkItems() { vector_.clear(); }

      private:
        /// for mutex-controlled use.
        deque_type deque_;

        /// for temporary use by the main thread.
        vector_type vector_;
    };

} // namespace vive
} // namespace osvr

#endif // INCLUDED_QuickProcessingDeque_h_GUID_B6819891_863F_4B8A_9024_C0E42E1D21AA
