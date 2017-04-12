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

using namespace vr;
Properties::Properties()
    : m_logger(osvr::util::log::make_logger("DriverLog")) {}

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
    PropertiesStore *pStore = m_propertyStores[deviceId];
    switch (pBatch->unTag) {
    case k_unFloatPropertyTag: {
        float val = pStore->get<float>(pBatch->prop);
        pBatch->pvBuffer = new float(val);
        break;
    }
    case k_unInt32PropertyTag: {
        uint32_t val = pStore->get<uint32_t>(pBatch->prop);
        pBatch->pvBuffer = new uint32_t(val);
        break;
    }
    case k_unUint64PropertyTag: {
        uint64_t val = pStore->get<uint64_t>(pBatch->prop);
        pBatch->pvBuffer = new uint64_t(val);
        break;
    }
    case k_unBoolPropertyTag: {
        bool val = pStore->get<bool>(pBatch->prop);
        pBatch->pvBuffer = new bool(val);
        break;
    }
    case k_unStringPropertyTag: {

        std::string val = pStore->get<std::string>(pBatch->prop);
        if (pBatch->unBufferSize < val.length()) {
            pBatch->eError = vr::TrackedProp_BufferTooSmall;
        } 
		else {
            auto ret = valveStrCpy(val, (char *)pBatch->pvBuffer,
                                   pBatch->unBufferSize);
            if (!ret) {
                pBatch->eError = vr::TrackedProp_BufferTooSmall;
            }
        }
        break;
    }
    default:
        m_logger->error("unknown tag type: ") << pBatch->unTag;
        break;
    }

    return vr::TrackedProp_Success;
}

ETrackedPropertyError
Properties::WritePropertyBatch(PropertyContainerHandle_t ulContainerHandle,
                               PropertyWrite_t *pBatch,
                               uint32_t unBatchEntryCount) {
    if (pBatch == nullptr) {
        m_logger->error("pBatch is null");
        return TrackedProp_NotYetAvailable;
    }
	uint64_t deviceId = ulContainerHandle - 1;
    if (!hasDeviceAt(deviceId)) {
        addDeviceAt(deviceId);
    }
    PropertiesStore *pStore = m_propertyStores[deviceId];
    if (pBatch->writeType == PropertyWrite_Set) {
        switch (pBatch->unTag) {
        case k_unFloatPropertyTag: {
            float val = *((float *)pBatch->pvBuffer);
            pStore->set<float>(pBatch->prop, val);
            break;
        }
        case k_unInt32PropertyTag: {
			uint32_t val = *((uint32_t *)pBatch->pvBuffer);
            pStore->set<uint32_t>(pBatch->prop, val);
            break;
        }
        case k_unUint64PropertyTag: {
			uint64_t val = *((uint64_t *)pBatch->pvBuffer);
            pStore->set<uint64_t>(pBatch->prop, val);
            break;
        }
        case k_unBoolPropertyTag: {
			bool val = *((bool *)pBatch->pvBuffer);
            pStore->set<bool>(pBatch->prop, val);
            break;
        }
        case k_unStringPropertyTag: {
            pStore->set<std::string>(pBatch->prop, (char *)pBatch->pvBuffer);
            break;
        }
        default: {
            m_logger->error("unknown Tag type: ") << pBatch->unTag;
            break;
        }
        }
    } else if (pBatch->writeType == PropertyWrite_Erase) {
        pStore->erase(pBatch->prop);
    } else if (pBatch->writeType == PropertyWrite_SetError) {
        m_logger->info("set error");
    } else {
        m_logger->error("unknown writeType: ") << pBatch->writeType;
    }

    return vr::TrackedProp_Success;
}

const char *Properties::GetPropErrorNameFromEnum(ETrackedPropertyError error) {
    return nullptr;
}

PropertyContainerHandle_t
Properties::TrackedDeviceToPropertyContainer(TrackedDeviceIndex_t nDevice) {
    if (!hasDeviceAt(nDevice)) {
        addDeviceAt(nDevice);
    }
    PropertyContainerHandle_t containerId = nDevice + 1;
    return containerId;
}

bool Properties::hasDeviceAt(const std::uint64_t idx) const {
    return (idx < m_propertyStores.size()) &&
           (nullptr != m_propertyStores[idx]);
}

void Properties::addDeviceAt(const std::uint64_t idx) {
    if (!(idx < m_propertyStores.size())) {
        /// OK, we need to reserve more room
        reserveIds(idx + 1);
    }
    m_propertyStores[idx] = new PropertiesStore;
}

bool Properties::reserveIds(std::uint64_t n) {
    if (m_propertyStores.size() < n) {
        m_propertyStores.resize(n, nullptr);
        return true;
    }
    return false;
}
