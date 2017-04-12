/** @file
    @brief Header

    @date 2017

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2017 Sensics, Inc.
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

#ifndef INCLUDED_KVDataStore_h_GUID_39D8D8D5_AC1B_4095_608F_75B74F37B460
#define INCLUDED_KVDataStore_h_GUID_39D8D8D5_AC1B_4095_608F_75B74F37B460

// Internal Includes
// - none

// Library/third-party includes
// - none

// Standard includes
#include <boost/variant.hpp>

#include <cstdint>
#include <string>
#include <unordered_map>

namespace osvr {
namespace util {
    template <typename KeyType, typename ValueVariant> class KVDataStore {
      public:
        using key_type = KeyType;
        using value_variant_type = ValueVariant;
        using container_type = std::unordered_map<key_type, value_variant_type>;
        using value_type = typename container_type::value_type;

        /// Sets the value associated with the key. Overwrites any existing
        /// value stored at that key.
        template <typename T> void set(key_type const &key, T const &val) {
            value_variant_type wrappedVal = val;
            data_[key] = wrappedVal;
        }

        /// Checks to see that a value has been associated with key, and
        /// furthermore that the value is of type T.
        template <typename T> bool contains(key_type const &key) const {
            auto it = data_.find(key);
            if (it == data_.end()) {
                // No data named by this key
                return false;
            }
            // OK, there's something named by this key...

            // This signature of get() on variant returns nullptr if the
            // contents are not of that type.
            T *contents = boost::get<T *>(it->second);
            return contents != nullptr;
        }

        /// Gets the value, of the type specified, identified by the key from
        /// the store. Assumes that contains<T>(key) is true: if it's not, it
        /// will return a default-constructed T.
        template <typename T> T get(key_type const &key) const {
            T ret;
            if (!contains<T>(key)) {
                // hey, not there!
                return ret;
            }
            auto it = data_.find(key);
            assert(it != data_.end() &&
                   "Should be handled by the above check for contains");

            ret = boost::get<T>(it->second);
            return ret;
        }

        /// Erase any value identified by the key from the store.
        void erase(key_type const &key) {
            auto it = data_.find(key);
            if (it == data_.end()) {
                // No data named by this key... so erase is a no-op.
                return;
            }
            data_.erase(it);
        }

      private:
        container_type data_;
    };
} // namespace util
} // namespace osvr

#endif // INCLUDED_KVDataStore_h_GUID_39D8D8D5_AC1B_4095_608F_75B74F37B460
