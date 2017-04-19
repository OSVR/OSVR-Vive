/** @file
    @brief Implementation

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

// Internal Includes
#include "Properties.h"
#include "ValveStrCpy.h"

// Library/third-party includes
// - none

// Standard includes
#include <iostream>
#include <string>
#include <type_traits>

using namespace vr;

namespace {
template <typename T>
struct PropertyTypeTagTrait
    : std::integral_constant<PropertyTypeTag_t, k_unInvalidPropertyTag> {};
/// Semicolon intentionally omitted at the end of this macro definition
#define OSVRVIVE_DEFINE_PROPERTY_TYPE_TAG(TYPE, TAG)                           \
    template <>                                                                \
    struct PropertyTypeTagTrait<TYPE>                                          \
        : std::integral_constant<PropertyTypeTag_t, TAG> {}

OSVRVIVE_DEFINE_PROPERTY_TYPE_TAG(float, k_unFloatPropertyTag);
OSVRVIVE_DEFINE_PROPERTY_TYPE_TAG(std::int32_t, k_unInt32PropertyTag);
OSVRVIVE_DEFINE_PROPERTY_TYPE_TAG(std::uint64_t, k_unUint64PropertyTag);
OSVRVIVE_DEFINE_PROPERTY_TYPE_TAG(bool, k_unBoolPropertyTag);
OSVRVIVE_DEFINE_PROPERTY_TYPE_TAG(std::string, k_unStringPropertyTag);
OSVRVIVE_DEFINE_PROPERTY_TYPE_TAG(vr::HmdMatrix34_t,
                                  k_unHmdMatrix34PropertyTag);

/// Note: if you add a type here, you must also make sure it's added to the
/// PropertyStoreVariant, as well as to Properties::WritePropertyBatch

#undef OSVRVIVE_DEFINE_PROPERTY_TYPE_TAG

struct GetContainedTypeTag : boost::static_visitor<PropertyTypeTag_t> {
    template <typename T> PropertyTypeTag_t operator()(T const &val) {
        return PropertyTypeTagTrait<T>::value;
    }
};
struct ValueSizeGetter : boost::static_visitor<std::size_t> {
    template <typename T> std::size_t operator()(T const &) const {
        return sizeof(T);
    }
    std::size_t operator()(std::string const &s) const {
        // add 1 for null terminator
        return s.size() + 1;
    }
};

template <typename T> inline std::size_t getValueSize(T const &val) {
    return ValueSizeGetter()(val);
}

struct ValueGetter : boost::static_visitor<> {
    ValueGetter(PropertyRead_t &batchEntry) : batchEntry_(&batchEntry) {}
    template <typename T> void operator()(T const &val) const {
        *reinterpret_cast<T *>(batchEntry_->pvBuffer) = val;
    }
    void operator()(std::string const &s) const {
        char *strBuf = reinterpret_cast<char *>(batchEntry_->pvBuffer);
        valveStrCpy(s, strBuf, batchEntry_->unBufferSize);
    }

  private:
    PropertyRead_t *batchEntry_;
};
} // namespace

inline void readProperty(PropertiesStore const &store,
                         PropertyRead_t &batchEntry) {
    if (!store.containsAnyType(batchEntry.prop)) {
        batchEntry.eError = TrackedProp_UnknownProperty;
        return;
    }
    auto &val = store.getVariant(batchEntry.prop);
    auto containedTag = val.apply_visitor(GetContainedTypeTag());
    bool actuallyGet = true;
    if (containedTag != batchEntry.unTag) {
        actuallyGet = false;
        batchEntry.unTag = containedTag;
    }
    batchEntry.unRequiredBufferSize =
        static_cast<std::uint32_t>(val.apply_visitor(ValueSizeGetter()));

    if (batchEntry.unRequiredBufferSize > batchEntry.unBufferSize) {
        return;
    }
    if (!actuallyGet) {
        return;
    }
    val.apply_visitor(ValueGetter(batchEntry));
}
Properties::Properties()
    : m_logger(osvr::util::log::make_logger("Properties")) {

    /*addDeviceAt(0);
    vr::PropertyWrite_t writeBatch;
    writeBatch.prop = Prop_UserConfigPath_String;
    writeBatch.writeType = PropertyWrite_Set;
    std::string userConfigPath = "C:/Program Files(x86)/Steam/config";
    writeBatch.unBufferSize = userConfigPath.length();
    writeBatch.pvBuffer = (char *)userConfigPath.c_str();
    writeBatch.unTag = k_unStringPropertyTag;
    WritePropertyBatch(1, &writeBatch, 1);

    writeBatch.prop = Prop_InstallPath_String;
    writeBatch.writeType = PropertyWrite_Set;
    std::string installPath = "C:/Program Files (x86)/Steam/config/lighthouse";
    writeBatch.unBufferSize = installPath.length();
    writeBatch.pvBuffer = (char*)userConfigPath.c_str();
    writeBatch.unTag = k_unStringPropertyTag;
    WritePropertyBatch(1, &writeBatch, 1);*/
}

ETrackedPropertyError
Properties::ReadPropertyBatch(PropertyContainerHandle_t ulContainerHandle,
                              PropertyRead_t *pBatch,
                              uint32_t unBatchEntryCount) {
    uint64_t deviceId = ulContainerHandle - 1;
    if (!hasDeviceAt(deviceId)) {
        m_logger->error("doesn't have the property container with id: ")
            << ulContainerHandle;
        return TrackedProp_InvalidDevice;
    }
    auto &store = m_propertyStores.at(deviceId);
    ETrackedPropertyError ret = TrackedProp_Success;
    for (std::uint32_t i = 0; i < unBatchEntryCount; ++i) {
        auto &entry = pBatch[i];

        m_logger->info() << "ReadPropertyBatch: Entry " << i << ", Property ID "
                         << entry.prop << ", Supplied tag type " << entry.unTag;
        readProperty(store, pBatch[i]);
        if (pBatch[i].eError != TrackedProp_Success) {
            ret = pBatch[i].eError;
            /// @todo OK to early out on first error?
            return ret;
        }
    }

    return ret;
}

ETrackedPropertyError
Properties::WritePropertyBatch(PropertyContainerHandle_t ulContainerHandle,
                               PropertyWrite_t *pBatch,
                               uint32_t unBatchEntryCount) {
    if (pBatch == nullptr) {
        m_logger->error("pBatch is null");
        return TrackedProp_InvalidOperation;
    }
    uint64_t deviceId = ulContainerHandle - 1;
    if (!hasDeviceAt(deviceId)) {
        addDeviceAt(deviceId);
    }
    PropertiesStore &pStore = m_propertyStores[deviceId];
    for (std::uint32_t i = 0; i < unBatchEntryCount; ++i) {
        auto &entry = pBatch[i];
        if (entry.writeType == PropertyWrite_Set) {
            entry.eError = vr::TrackedProp_Success;
            switch (entry.unTag) {
            case k_unFloatPropertyTag: {
                auto val = *(reinterpret_cast<float *>(entry.pvBuffer));
                pStore.set(pBatch->prop, val);
                break;
            }
            case k_unInt32PropertyTag: {
                auto val = *(reinterpret_cast<int32_t *>(entry.pvBuffer));
                pStore.set(entry.prop, val);
                break;
            }
            case k_unUint64PropertyTag: {
                auto val = *(reinterpret_cast<uint64_t *>(entry.pvBuffer));
                pStore.set(entry.prop, val);
                break;
            }
            case k_unBoolPropertyTag: {
                auto val = *(reinterpret_cast<bool *>(entry.pvBuffer));
                pStore.set(entry.prop, val);
                break;
            }
            case k_unStringPropertyTag: {
                pStore.set(entry.prop, std::string(reinterpret_cast<char *>(
                                           entry.pvBuffer)));
                break;
            }
            case k_unHmdMatrix34PropertyTag: {
                auto val =
                    *(reinterpret_cast<vr::HmdMatrix34_t *>(entry.pvBuffer));
                pStore.set(entry.prop, val);
                break;
            }
            default: {
                m_logger->error()
                    << "In property setting: Unhandled property tag type: "
                    << entry.unTag << " for property ID" << entry.prop;
                entry.eError = vr::TrackedProp_InvalidOperation;
                break;
            }
            }
        } else if (entry.writeType == PropertyWrite_Erase) {
            pStore.erase(entry.prop);
        } else if (entry.writeType == PropertyWrite_SetError) {
            m_logger->info("set error");
        } else {
            m_logger->error("unknown writeType: ") << entry.writeType;
            return vr::TrackedProp_InvalidOperation;
        }
    }

    return vr::TrackedProp_Success;
}

const char *Properties::GetPropErrorNameFromEnum(ETrackedPropertyError error) {
    return nullptr;
}

PropertyContainerHandle_t
Properties::TrackedDeviceToPropertyContainer(TrackedDeviceIndex_t nDevice) {
    m_logger->info("TrackedDeviceToPropertyContainer")
        << " at index " << nDevice;
    if (!hasDeviceAt(nDevice)) {
        addDeviceAt(nDevice);
    }
    PropertyContainerHandle_t containerId = nDevice + 1;
    return containerId;
}

bool Properties::hasDeviceAt(const std::uint64_t idx) const {
    return (idx < m_propertyStores.size());
}

void Properties::addDeviceAt(const std::uint64_t idx) {
    /// @todo this is kind of redundant with reserveIds
    m_logger->info("addDeviceAt ") << idx;
    if (!(idx < m_propertyStores.size())) {
        /// OK, we need to reserve more room
        reserveIds(idx + 1);
    }
}

bool Properties::reserveIds(std::uint64_t n) {
    if (m_propertyStores.size() < n) {
        m_propertyStores.resize(n);
        return true;
    }
    return false;
}
