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
#include "Settings.h"
#include "ValveStrCpy.h"

// Standard includes
#include <iostream>

using namespace vr;

Settings::Settings() : m_logger(osvr::util::log::make_logger("Settings")) {

    m_settingStore.set<bool>("disableimu", false);
    m_settingStore.set<std::string>("usedisambiguation", "tdm");
    m_settingStore.set<int32_t>("disambiguationdebug", 0);
    m_settingStore.set<int32_t>("primarybasestation", 0);
    m_settingStore.set<bool>("dbhistory", false);
    m_settingStore.set<bool>("trackedCamera", true);
    m_settingStore.set<float>("cameraFrameCaptureOffsetTime", 0.0);
    m_settingStore.set<int32_t>("cameraFrameRate", 30);
    m_settingStore.set<int32_t>("cameraSensorFrameRate", 30);
    m_settingStore.set<bool>("cameraEdgeEnhancement", true);
    m_settingStore.set<int32_t>("cameraISPSyncDivisor", 1);
    m_settingStore.set<bool>("enableCamera", true);
    m_settingStore.set<bool>("fakeHtcHmdMainboard", false);
    m_settingStore.set<int32_t>("deactivateStandbyOverride", 1);
}

const char *
Settings::GetSettingsErrorNameFromEnum(vr::EVRSettingsError eError) {
    m_logger->debug("GetSettingsErrorNameFromEnum: ") << eError;
    return "Error";
}

bool Settings::Sync(bool bForce, vr::EVRSettingsError *peError) {
    m_logger->debug("Sync: ") << bForce << ", " << peError;
    return true;
}

bool Settings::GetBool(const char *pchSection, const char *pchSettingsKey,
                       vr::EVRSettingsError *peError) {
    m_logger->debug("GetBool: ") << pchSection << ", " << pchSettingsKey;
    if (m_settingStore.contains<bool>(pchSettingsKey)) {
        return m_settingStore.get<bool>(pchSettingsKey);
    }
    m_logger->info("GetBool doesn't contain the settingsKey: ")
        << pchSettingsKey;
    if (peError != nullptr) {
        *peError = VRSettingsError_ReadFailed;
    }
    return false;
}
void Settings::SetBool(const char *pchSection, const char *pchSettingsKey,
                       bool bValue, vr::EVRSettingsError * /*peError*/) {
    m_logger->debug("SetBool: ") << pchSection << ", " << pchSettingsKey;
    m_settingStore.set<bool>(pchSettingsKey, bValue);
}

int32_t Settings::GetInt32(const char *pchSection, const char *pchSettingsKey,
                           vr::EVRSettingsError *peError) {
    m_logger->debug("GetInt32: ") << pchSection << ", " << pchSettingsKey;
    if (m_settingStore.contains<int32_t>(pchSettingsKey)) {
        return m_settingStore.get<int32_t>(pchSettingsKey);
    }
    m_logger->info("GetInt32 doesn't contain the settingsKey: ")
        << pchSettingsKey;
    if (peError != nullptr) {
        *peError = VRSettingsError_ReadFailed;
    }
    return 0;
}

void Settings::SetInt32(const char *pchSection, const char *pchSettingsKey,
                        int32_t nValue, vr::EVRSettingsError * /*peError*/) {
    m_logger->debug("SetInt32: ") << pchSection << ", " << pchSettingsKey;
    m_settingStore.set<int32_t>(pchSettingsKey, nValue);
}

float Settings::GetFloat(const char *pchSection, const char *pchSettingsKey,
                         vr::EVRSettingsError *peError) {
    m_logger->debug("GetFloat: ") << pchSection << ", " << pchSettingsKey;
    if (m_settingStore.contains<float>(pchSettingsKey)) {
        return m_settingStore.get<float>(pchSettingsKey);
    }
    m_logger->info("GetFloat doesn't contain the settingsKey: ")
        << pchSettingsKey;
    if (peError != nullptr) {
        *peError = VRSettingsError_ReadFailed;
    }
    return 0;
}

void Settings::SetFloat(const char *pchSection, const char *pchSettingsKey,
                        float flValue, vr::EVRSettingsError * /*peError*/) {
    m_logger->debug("SetFloat: ") << pchSection << ", " << pchSettingsKey;
    m_settingStore.set<float>(pchSettingsKey, flValue);
}

void Settings::GetString(const char *pchSection, const char *pchSettingsKey,
                         char *pchValue, uint32_t unValueLen,
                         vr::EVRSettingsError *peError) {
    m_logger->debug("GetString: ")
        << pchSection << ", " << pchSettingsKey << ", " << unValueLen;
    if (!m_settingStore.contains<std::string>(pchSettingsKey)) {
        m_logger->info("GetString doesn't contain the settingsKey: ")
            << pchSettingsKey;
        if (peError != nullptr) {
            *peError = VRSettingsError_ReadFailed;
        }
        return;
    }
    std::string val = m_settingStore.get<std::string>(pchSettingsKey);
    if (unValueLen < val.length() + 1) {
        if (peError != nullptr) {
            *peError = VRSettingsError_ReadFailed;
        }
        return;
    }
    auto ret = valveStrCpy(val, pchValue, unValueLen);
    if (ret == 0u && peError != nullptr) {
        *peError = VRSettingsError_ReadFailed;
    }
}

void Settings::SetString(const char *pchSection, const char *pchSettingsKey,
                         const char *pchValue,
                         vr::EVRSettingsError * /*peError*/) {
    m_logger->debug("SetString: ") << pchSection << ", " << pchSettingsKey;
    m_settingStore.set<std::string>(pchSettingsKey, pchValue);
}

void Settings::RemoveSection(const char *pchSection,
                             vr::EVRSettingsError * /*peError*/) {
    m_logger->debug("We don't remove section: ") << pchSection;
}

void Settings::RemoveKeyInSection(const char *pchSection,
                                  const char *pchSettingsKey,
                                  vr::EVRSettingsError * /*peError*/) {
    m_logger->debug("RemoveKeyInSection: ")
        << pchSection << ", " << pchSettingsKey;
    m_settingStore.erase(pchSettingsKey);
}
