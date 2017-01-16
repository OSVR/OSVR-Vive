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

// Library/third-party includes
#include <VRSettings.h>

// Standard includes
#include <iostream>

using namespace vr;

#ifdef DISABLE_LOG_EVENTS
#define LOG_EVENTS(X)                                                          \
    do {                                                                       \
    } while (0)
#else
#define LOG_EVENTS(X)                                                          \
    do {                                                                       \
        std::cout << X << std::endl;                                           \
    } while (0)
#endif

const char *VRSettings::GetSettingsErrorNameFromEnum(EVRSettingsError eError) {
    LOG_EVENTS("GetSettingsErrorNameFromEnum(" << eError << ")");
    return "Error";
}

bool VRSettings::Sync(bool bForce, EVRSettingsError *peError) {
    LOG_EVENTS("Sync(" << bForce << ", " << peError << ")");
    // retur true for now
    return true;
}

bool VRSettings::GetBool(const char *pchSection, const char *pchSettingsKey,
                         EVRSettingsError *peError) {
    LOG_EVENTS("GetBool(" << pchSection << ", " << pchSettingsKey << ", "
                          << peError << ")");
    // return true for now
    return true;
}
void VRSettings::SetBool(const char *pchSection, const char *pchSettingsKey,
                         bool bValue, EVRSettingsError *peError) {
    LOG_EVENTS("SetBool(" << pchSection << ", " << pchSettingsKey << ", "
                          << bValue << ", " << peError << ")");
}

int32_t VRSettings::GetInt32(const char *pchSection, const char *pchSettingsKey,
                             int32_t nDefaultValue, EVRSettingsError *peError) {
    LOG_EVENTS("SetBool(" << pchSection << ", " << pchSettingsKey << ", "
                          << nDefaultValue << ", " << peError << ")");
    // return non-existing value for now
    return 67;
}
void VRSettings::SetInt32(const char *pchSection, const char *pchSettingsKey,
                          int32_t nValue, EVRSettingsError *peError) {
    LOG_EVENTS("SetInt32(" << pchSection << ", " << pchSettingsKey << ", "
                           << nValue << ", " << peError << ")");
}

float VRSettings::GetFloat(const char *pchSection, const char *pchSettingsKey,
                           EVRSettingsError *peError) {
    LOG_EVENTS("SetInt32(" << pchSection << ", " << pchSettingsKey << ", "
                           << peError << ")");
    return 67.0;
}

void VRSettings::SetFloat(const char *pchSection, const char *pchSettingsKey,
                          float flValue, EVRSettingsError *peError) {
    LOG_EVENTS("SetInt32(" << pchSection << ", " << pchSettingsKey << ", "
                           << flValue << ", " << peError << ")");
}

void GetString(const char *pchSection, const char *pchSettingsKey,
               char *pchValue, uint32_t unValueLen, EVRSettingsError *peError) {
    LOG_EVENTS("SetInt32(" << pchSection << ", " << pchSettingsKey << ", "
                           << unValueLen << ", " << peError << ")");
}

void VRSettings::SetString(const char *pchSection, const char *pchSettingsKey,
                           const char *pchValue, EVRSettingsError *peError) {
    LOG_EVENTS("SetInt32(" << pchSection << ", " << pchSettingsKey << ", "
                           << pchValue << ", " << peError << ")");
}

void VRSettings::RemoveSection(const char *pchSection,
                               EVRSettingsError *peError) {
    LOG_EVENTS("SetInt32(" << pchSection << ", " << peError << ")");
}

void VRSettings::RemoveKeyInSection(const char *pchSection,
                                    const char *pchSettingsKey,
                                    EVRSettingsError *peError) {
    LOG_EVENTS("SetInt32(" << pchSection << ", " << pchSettingsKey << ", "
                           << peError << ")");
}
