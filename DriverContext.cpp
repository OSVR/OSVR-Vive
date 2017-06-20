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
#include "DriverContext.h"

// Library/third-party includes
// - none

// Standard includes
// - none

using namespace vr;

DriverContext::DriverContext()
    : m_pServerDriverHost(nullptr), m_pVRProperties(nullptr),
      m_pVRSetting(nullptr), m_pVRDriverLog(nullptr),
      m_pVRDriverManager(nullptr), m_pVRResources(nullptr) {}

DriverContext::DriverContext(vr::ServerDriverHost *serverDriverHost,
                             vr::Settings *settings, vr::DriverLog *driverLog,
                             vr::Properties *properties,
                             vr::DriverManager *driverManager,
                             vr::Resources *resources)
    : m_pServerDriverHost(serverDriverHost), m_pVRProperties(properties),
      m_pVRSetting(settings), m_pVRDriverLog(driverLog),
      m_pVRDriverManager(driverManager), m_pVRResources(resources) {}

// only consider four type of interfaces
void *DriverContext::GetGenericInterface(const char *pchInterfaceVersion,
                                         vr::EVRInitError *peError) {
    std::string interfaceVersion = pchInterfaceVersion;
    if (interfaceVersion.compare(IVRServerDriverHost_Version) == 0) {
        return m_pServerDriverHost;
    } else if (interfaceVersion.compare(IVRSettings_Version) == 0) {
        return m_pVRSetting;
    } else if (interfaceVersion.compare(IVRProperties_Version) == 0) {
        return m_pVRProperties;
    } else if (interfaceVersion.compare(IVRDriverLog_Version) == 0) {
        return m_pVRDriverLog;
    } else if (interfaceVersion.compare(IVRDriverManager_Version) == 0) {
        return m_pVRDriverManager;
    } else if (interfaceVersion.compare(IVRResources_Version) == 0) {
        return m_pVRResources;
    } else {
        std::string errMsg = "Got an unhandled interface type version - " +
                             std::string(pchInterfaceVersion);
        m_pVRDriverLog->Log(errMsg.c_str());
        return nullptr;
    }
}

// not sure which driver handle to return now
vr::DriverHandle_t DriverContext::GetDriverHandle() {
    vr::DriverHandle_t dh = 1;
    return dh;
}
