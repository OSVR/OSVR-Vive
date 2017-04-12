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

Settings::Settings() : m_logger(osvr::util::log::make_logger("Settings")) {}

const char *
Settings::GetSettingsErrorNameFromEnum(vr::EVRSettingsError eError) {
	m_logger->error("GetSettingsErrorNameFromEnum: ") << eError;
    return "Error";
}

bool Settings::Sync(bool bForce, vr::EVRSettingsError *peError) {
	m_logger->info("Sync: ") << bForce << ", " << peError;
    return true;
}

bool Settings::GetBool(const char *pchSection, const char *pchSettingsKey,
                       vr::EVRSettingsError *peError) {
	m_logger->info("GetBool: ") << pchSection << ", " << pchSettingsKey;
	if (m_settingStore.contains<bool>(pchSettingsKey)) {
		bool val = m_settingStore.get<bool>(pchSettingsKey);
		return val;
	}
	else {
		m_logger->error("GetBool doesn't contain the settingKey: ") << pchSettingsKey;
		peError = new vr::EVRSettingsError(VRSettingsError_ReadFailed);
		return false;
	}
}
void Settings::SetBool(const char *pchSection, const char *pchSettingsKey,
                       bool bValue, vr::EVRSettingsError *peError) {
	m_logger->info("SetBool: ") << pchSection << ", " << pchSettingsKey;
	m_settingStore.set<bool>(pchSettingsKey, bValue);
}

int32_t Settings::GetInt32(const char *pchSection, const char *pchSettingsKey,
                           vr::EVRSettingsError *peError) {
	m_logger->info("GetInt32: ") << pchSection << ", " << pchSettingsKey;
	if (m_settingStore.contains<int32_t>(pchSettingsKey)) {
		int32_t val = m_settingStore.get<int32_t>(pchSettingsKey);
		return val;
	}
	else {
		m_logger->error("GetInt32 doesn't contain the settingKey: ") << pchSettingsKey;
		peError = new vr::EVRSettingsError(VRSettingsError_ReadFailed);
		return 0;
	}
}
void Settings::SetInt32(const char *pchSection, const char *pchSettingsKey,
                        int32_t nValue, vr::EVRSettingsError *peError) {
	m_logger->info("SetInt32: ") << pchSection << ", " << pchSettingsKey;
	m_settingStore.set<int32_t>(pchSettingsKey, nValue);
}

float Settings::GetFloat(const char *pchSection, const char *pchSettingsKey,
                         vr::EVRSettingsError *peError) {
	m_logger->info("GetFloat: ") << pchSection << ", " << pchSettingsKey;
	if (m_settingStore.contains<float>(pchSettingsKey)) {
		float val = m_settingStore.get<float>(pchSettingsKey);
		return val;
	}
	else {
		m_logger->error("GetFloat doesn't contain the settingKey: ") << pchSettingsKey;
		peError = new vr::EVRSettingsError(VRSettingsError_ReadFailed);
		return 0;
	}
}

void Settings::SetFloat(const char *pchSection, const char *pchSettingsKey,
                        float flValue, vr::EVRSettingsError *peError) {
	m_logger->info("SetFloat: ") << pchSection << ", " << pchSettingsKey;
	m_settingStore.set<int32_t>(pchSettingsKey, flValue);
}

void Settings::GetString(const char *pchSection, const char *pchSettingsKey,
                         char *pchValue, uint32_t unValueLen,
                         vr::EVRSettingsError *peError) {
	m_logger->info("GetString: ") << pchSection << ", " << pchSettingsKey << ", " << unValueLen;
	if (m_settingStore.contains<std::string>(pchSettingsKey)) {
		std::string val = m_settingStore.get<std::string>(pchSettingsKey);
		if (unValueLen < val.length()) {
			peError = new vr::EVRSettingsError(VRSettingsError_ReadFailed);
		}
		else {
			auto ret = valveStrCpy(val, pchValue, unValueLen);
			if (!ret) {
				peError = new vr::EVRSettingsError(VRSettingsError_ReadFailed);
			}
		}
	}
	else {
		m_logger->error("GetFloat doesn't contain the settingKey: ") << pchSettingsKey;
		peError = new vr::EVRSettingsError(VRSettingsError_ReadFailed);
	}
}

void Settings::SetString(const char *pchSection, const char *pchSettingsKey,
                         const char *pchValue, vr::EVRSettingsError *peError) {
	m_logger->info("SetString: ") << pchSection << ", " << pchSettingsKey;
	m_settingStore.set<std::string>(pchSettingsKey, pchValue);
}

void Settings::RemoveSection(const char *pchSection,
                             vr::EVRSettingsError *peError) {
	m_logger->info("We don't remove section: ") << pchSection;
}

void Settings::RemoveKeyInSection(const char *pchSection,
                                  const char *pchSettingsKey,
                                  vr::EVRSettingsError *peError) {
	m_logger->info("RemoveKeyInSection: ") << pchSection << ", " << pchSettingsKey;
	m_settingStore.erase(pchSettingsKey);
}
