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

#ifndef INCLUDED_Settings_h_GUID_94402E2B_13D0_4E97_FA1F_CA2B7F647A38
#define INCLUDED_Settings_h_GUID_94402E2B_13D0_4E97_FA1F_CA2B7F647A38

// Internal Includes
#include <osvr/Util/Logger.h>
#include <vendor/util-headers/util/KVDataStore.h>

// Library/third-party includes
#include <openvr_driver.h>

using SettingsStoreVariant =
    boost::variant<bool, std::int32_t, float, std::string>;
using SettingsStore =
    osvr::util::KVDataStore<std::string, SettingsStoreVariant>;

// Standard includes

namespace vr {

class Settings : public vr::IVRSettings {

  public:
    Settings();

    virtual const char *
    GetSettingsErrorNameFromEnum(vr::EVRSettingsError eError);

    // Returns true if file sync occurred (force or settings dirty)
    virtual bool Sync(bool bForce = false,
                      vr::EVRSettingsError *peError = nullptr);

    virtual bool GetBool(const char *pchSection, const char *pchSettingsKey,
                         vr::EVRSettingsError *peError = nullptr);

    virtual void SetBool(const char *pchSection, const char *pchSettingsKey,
                         bool bValue, vr::EVRSettingsError *peError = nullptr);
    virtual int32_t GetInt32(const char *pchSection, const char *pchSettingsKey,
                             vr::EVRSettingsError *peError = nullptr);
    virtual void SetInt32(const char *pchSection, const char *pchSettingsKey,
                          int32_t nValue,
                          vr::EVRSettingsError *peError = nullptr);
    virtual float GetFloat(const char *pchSection, const char *pchSettingsKey,
                           vr::EVRSettingsError *peError = nullptr);
    virtual void SetFloat(const char *pchSection, const char *pchSettingsKey,
                          float flValue,
                          vr::EVRSettingsError *peError = nullptr);
    virtual void GetString(const char *pchSection, const char *pchSettingsKey,
                           char *pchValue, uint32_t unValueLen,
                           vr::EVRSettingsError *peError = nullptr);
    virtual void SetString(const char *pchSection, const char *pchSettingsKey,
                           const char *pchValue,
                           vr::EVRSettingsError *peError = nullptr);

    virtual void RemoveSection(const char *pchSection,
                               vr::EVRSettingsError *peError = nullptr);
    virtual void RemoveKeyInSection(const char *pchSection,
                                    const char *pchSettingsKey,
                                    vr::EVRSettingsError *peError = nullptr);

  private:
    osvr::util::log::LoggerPtr m_logger;
    SettingsStore m_settingStore;
};

} // namespace vr

#endif // INCLUDED_Settings_h_GUID_94402E2B_13D0_4E97_FA1F_CA2B7F647A38
