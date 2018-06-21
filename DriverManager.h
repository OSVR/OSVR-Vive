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

#ifndef INCLUDED_DriverManager_h_GUID_BC08BDF5_CBD0_4DAF_F30F_43DE8357A43B
#define INCLUDED_DriverManager_h_GUID_BC08BDF5_CBD0_4DAF_F30F_43DE8357A43B

// Internal Includes
#include <osvr/Util/Logger.h>

// Library/third-party includes
#include <openvr_driver.h>

// Standard includes
#include <cstdint>

namespace vr {

class DriverManager : public vr::IVRDriverManager {
  public:
    DriverManager();
    /**
    @brief This function always returns 1, since we're only using lighthouse
    driver
    */
    virtual uint32_t GetDriverCount() const;
    /**
    @brief Determine and store path to the SteamVR driver for given nDriver id
    The resulting path is stored as placeholder string {pchValue}
    It is used by IVRResources impl to load files specified by path
    */
    virtual uint32_t GetDriverName(vr::DriverId_t nDriver, char *pchValue,
                                   uint32_t unBufferSize);

    virtual DriverHandle_t GetDriverHandle(const char *pchDriverName);

  private:
    osvr::util::log::LoggerPtr m_logger;
};

} // namespace vr

#endif // INCLUDED_DriverManager_h_GUID_BC08BDF5_CBD0_4DAF_F30F_43DE8357A43B
