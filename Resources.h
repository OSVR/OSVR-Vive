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

#ifndef INCLUDED_Resources_h_GUID_693D9495_1559_4899_9D4E_7825D29AE2AC
#define INCLUDED_Resources_h_GUID_693D9495_1559_4899_9D4E_7825D29AE2AC

// Internal Includes
#include <osvr/Util/Logger.h>

// Library/third-party includes
#include <openvr_driver.h>

// Standard includes

namespace vr {

class Resources : public vr::IVRResources {
  public:
    Resources();
    virtual uint32_t LoadSharedResource(const char *pchResourceName,
                                        char *pchBuffer, uint32_t unBufferLen);
    virtual uint32_t GetResourceFullPath(const char *pchResourceName,
                                         const char *pchResourceTypeDirectory,
                                         char *pchPathBuffer,
                                         uint32_t unBufferLen);

  private:
    osvr::util::log::LoggerPtr m_logger;
};

} // namespace vr

#endif // INCLUDED_Resources_h_GUID_693D9495_1559_4899_9D4E_7825D29AE2AC
