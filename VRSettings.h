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

#ifndef INCLUDED_VRSettings_h_GUID_138FAF71_763D_4499_62A1_BBD01F8F2567
#define INCLUDED_VRSettings_h_GUID_138FAF71_763D_4499_62A1_BBD01F8F2567

// Internal Includes

// Library/third-party includes
#include <openvr_driver.h>

// Standard includes
// - none
using namespace vr;
namespace osvr {
namespace vive {

    class VRSettings : public vr::IVRSettings {

      public:
        virtual const char *
        GetSettingsErrorNameFromEnum(vr::EVRSettingsError eError);

        // Returns true if file sync occurred (force or settings dirty)
        virtual bool Sync(bool bForce = false,
                          vr::EVRSettingsError *peError = nullptr);

        virtual bool GetBool(const char *pchSection, const char *pchSettingsKey,
                             vr::EVRSettingsError *peError = nullptr);

        virtual void SetBool(const char *pchSection, const char *pchSettingsKey,
                             bool bValue,
                             vr::EVRSettingsError *peError = nullptr);
        virtual int32_t GetInt32(const char *pchSection,
                                 const char *pchSettingsKey,
                                 vr::EVRSettingsError *peError = nullptr);
        virtual void SetInt32(const char *pchSection,
                              const char *pchSettingsKey, int32_t nValue,
                              vr::EVRSettingsError *peError = nullptr);
        virtual float GetFloat(const char *pchSection,
                               const char *pchSettingsKey, 
                               vr::EVRSettingsError *peError = nullptr);
        virtual void SetFloat(const char *pchSection,
                              const char *pchSettingsKey, float flValue,
                              vr::EVRSettingsError *peError = nullptr);
        virtual void GetString(const char *pchSection,
                               const char *pchSettingsKey, char *pchValue,
                               uint32_t unValueLen,
                               vr::EVRSettingsError *peError = nullptr);
        virtual void SetString(const char *pchSection,
                               const char *pchSettingsKey, const char *pchValue,
                               vr::EVRSettingsError *peError = nullptr);

        virtual void RemoveSection(const char *pchSection,
                                   vr::EVRSettingsError *peError = nullptr);
        virtual void
        RemoveKeyInSection(const char *pchSection, const char *pchSettingsKey,
                           vr::EVRSettingsError *peError = nullptr);
    };
}
}

#endif // INCLUDED_IVRSettings_h_GUID_138FAF71_763D_4499_62A1_BBD01F8F2567
