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
#include <Settings.h>

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

const char *
Settings::GetSettingsErrorNameFromEnum(vr::EVRSettingsError eError) {
    LOG_EVENTS("GetSettingsErrorNameFromEnum(" << eError << ")");
    return "Error";
}

bool Settings::Sync(bool bForce, vr::EVRSettingsError *peError) {
    LOG_EVENTS("Sync(" << bForce << ", " << peError << ")");
    // retur true for now
    return true;
}

bool Settings::GetBool(const char *pchSection, const char *pchSettingsKey,
                       vr::EVRSettingsError *peError) {
    LOG_EVENTS("GetBool(" << pchSection << ", " << pchSettingsKey << ", "
                          << peError << ")");
    // return true for now
    return true;
}
void Settings::SetBool(const char *pchSection, const char *pchSettingsKey,
                       bool bValue, vr::EVRSettingsError *peError) {
    LOG_EVENTS("SetBool(" << pchSection << ", " << pchSettingsKey << ", "
                          << bValue << ", " << peError << ")");
}

int32_t Settings::GetInt32(const char *pchSection, const char *pchSettingsKey,
                           vr::EVRSettingsError *peError) {
    LOG_EVENTS("SetBool(" << pchSection << ", " << pchSettingsKey << ", "
                          << peError << ")");
    // return non-existing value for now
    return 67;
}
void Settings::SetInt32(const char *pchSection, const char *pchSettingsKey,
                        int32_t nValue, vr::EVRSettingsError *peError) {
    LOG_EVENTS("SetInt32(" << pchSection << ", " << pchSettingsKey << ", "
                           << nValue << ", " << peError << ")");
}

float Settings::GetFloat(const char *pchSection, const char *pchSettingsKey,
                         vr::EVRSettingsError *peError) {
    LOG_EVENTS("SetInt32(" << pchSection << ", " << pchSettingsKey << ", "
                           << peError << ")");
    return 67.0;
}

void Settings::SetFloat(const char *pchSection, const char *pchSettingsKey,
                        float flValue, vr::EVRSettingsError *peError) {
    LOG_EVENTS("SetInt32(" << pchSection << ", " << pchSettingsKey << ", "
                           << flValue << ", " << peError << ")");
}

void Settings::GetString(const char *pchSection, const char *pchSettingsKey,
                         char *pchValue, uint32_t unValueLen,
                         vr::EVRSettingsError *peError) {
    LOG_EVENTS("SetInt32(" << pchSection << ", " << pchSettingsKey << ", "
                           << unValueLen << ", " << peError << ")");
}

void Settings::SetString(const char *pchSection, const char *pchSettingsKey,
                         const char *pchValue, vr::EVRSettingsError *peError) {
    LOG_EVENTS("SetInt32(" << pchSection << ", " << pchSettingsKey << ", "
                           << pchValue << ", " << peError << ")");
}

void Settings::RemoveSection(const char *pchSection,
                             vr::EVRSettingsError *peError) {
    LOG_EVENTS("SetInt32(" << pchSection << ", " << peError << ")");
}

void Settings::RemoveKeyInSection(const char *pchSection,
                                  const char *pchSettingsKey,
                                  vr::EVRSettingsError *peError) {
    LOG_EVENTS("SetInt32(" << pchSection << ", " << pchSettingsKey << ", "
                           << peError << ")");
}
