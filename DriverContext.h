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

#ifndef INCLUDED_DriverContext_h_GUID_36B19EC5_76DE_49B4_5B1A_37B88E5E6C42
#define INCLUDED_DriverContext_h_GUID_36B19EC5_76DE_49B4_5B1A_37B88E5E6C42

// Internal
#include "ServerDriverHost.h"
#include "Settings.h"
#include "DriverLog.h"

// Library/third-party includes
#include <openvr_driver.h>

// Standard includes
#include <iostream>
#include <memory>
#include <string>
#include <utility>

namespace vr {
class DriverContext : public vr::IVRDriverContext {
  public:
    DriverContext();
    DriverContext(vr::ServerDriverHost *serverDriverHost,
                  vr::Settings *settings);

    /// Returns the requested interface. If the interface was not available
    /// it will return NULL and fill out the error.
    virtual void *GetGenericInterface(const char *pchInterfaceVersion,
                                      vr::EVRInitError *peError = nullptr);

    /// Returns the property container handle for this driver
    virtual vr::DriverHandle_t GetDriverHandle();

  private:
    vr::ServerDriverHost *m_pServerDriverHost;
    vr::IVRProperties *m_pVRProperties;
    vr::IVRSettings *m_pVRSetting;
    vr::IVRDriverLog *m_pVRDriverLog;
};

} // namespace vr

#endif // INCLUDED_DriverContext_h_GUID_36B19EC5_76DE_49B4_5B1A_37B88E5E6C42
