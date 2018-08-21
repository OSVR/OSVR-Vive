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

#ifndef INCLUDED_Properties_h_GUID_A9CAB07C_6501_48B4_49B3_902355FD43B5
#define INCLUDED_Properties_h_GUID_A9CAB07C_6501_48B4_49B3_902355FD43B5

// Internal Includes
#include <osvr/Util/Logger.h>
#include <vendor/util-headers/util/KVDataStore.h>

// Library/third-party includes
#include <openvr_driver.h>

// Standard includes
#include <cstdint>
#include <vector>

/// These are the only value types used by publicly-declared properties.
using PropertiesStoreVariant =
    boost::variant<bool, float, std::int32_t, std::uint64_t, std::string,
                   vr::HmdMatrix34_t, vr::HmdVector2_t>;
using PropertiesStore =
    osvr::util::KVDataStore<vr::ETrackedDeviceProperty, PropertiesStoreVariant>;

namespace vr {

class Properties : public vr::IVRProperties {
  public:
    Properties();
    /** Reads a set of properties atomically. See the PropertyReadBatch_t struct
     * for more information. */
    ETrackedPropertyError
    ReadPropertyBatch(PropertyContainerHandle_t ulContainerHandle,
                      PropertyRead_t *pBatch,
                      uint32_t unBatchEntryCount) override;

    /** Writes a set of properties atomically. See the PropertyWriteBatch_t
     * struct for more information. */
    ETrackedPropertyError
    WritePropertyBatch(PropertyContainerHandle_t ulContainerHandle,
                       PropertyWrite_t *pBatch,
                       uint32_t unBatchEntryCount) override;

    /** returns a string that corresponds with the specified property error. The
     * string will be the name of the error enum value for all valid error codes
     */
    const char *GetPropErrorNameFromEnum(ETrackedPropertyError error) override;

    /** Returns a container handle given a tracked device index */
    PropertyContainerHandle_t
    TrackedDeviceToPropertyContainer(TrackedDeviceIndex_t nDevice) override;

  private:
    bool hasDeviceAt(const std::uint64_t idx) const;
    void addDeviceAt(const std::uint64_t idx);
    bool reserveIds(std::uint64_t n);

    osvr::util::log::LoggerPtr m_logger;
    std::vector<PropertiesStore> m_propertyStores;
};

} // namespace vr

#endif // INCLUDED_Properties_h_GUID_A9CAB07C_6501_48B4_49B3_902355FD43B5
