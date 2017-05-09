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

#ifndef INCLUDED_DriverLog_h_GUID_C621485C_0BE0_45BD_765F_0A1FE4A07D83
#define INCLUDED_DriverLog_h_GUID_C621485C_0BE0_45BD_765F_0A1FE4A07D83

// Internal Includes
#include <osvr/Util/Logger.h>

// Library/third-party includes
#include <openvr_driver.h>

// Standard includes

namespace vr {

class DriverLog : public vr::IVRDriverLog {
  public:
    DriverLog();
    virtual void Log(const char *pchLogMessage);

  private:
    osvr::util::log::LoggerPtr logger_;
};

} // namespace vr

#endif // INCLUDED_DriverLog_h_GUID_C621485C_0BE0_45BD_765F_0A1FE4A07D83
