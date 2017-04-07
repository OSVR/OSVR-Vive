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
#include "VRDriverContext.h"

// Library/third-party includes
// - none

// Standard includes
// - none

namespace osvr {
namespace vive {
	ViveDriverContext::ViveDriverContext() 
		: m_pServerDriverHost(nullptr), 
		  m_pVRProperties(nullptr),
		  m_pVRSetting(nullptr), 
		  m_pVRDriverLog(nullptr) {}
		
	ViveDriverContext::ViveDriverContext(vr::ServerDriverHost *serverDriverHost, vive::VRSettings *settings)
	    : m_pServerDriverHost(serverDriverHost),
		  m_pVRProperties(nullptr),
		  m_pVRSetting(settings),
		  m_pVRDriverLog(nullptr) {}
		 
    // only consider four type of interfaces		 
    void *ViveDriverContext::GetGenericInterface( const char *pchInterfaceVersion, 
		vr::EVRInitError *peError) 
	{
		std::string interfaceVersion = pchInterfaceVersion;
		if (interfaceVersion.compare("IVRServerDriverHost_004") == 0) {
			std::cout << "Return Server Driver Host" << std::endl;
			return m_pServerDriverHost;
		}
		else if (interfaceVersion.compare("IVRSettings_002") == 0) {
			std::cout << "Return m_pVRSetting" << std::endl;
			return m_pVRSetting;
		}
		else if (interfaceVersion.compare("IVRProperties_001") == 0) {
			std::cout << "Return m_pVRProperties" << std::endl;
			return m_pVRProperties;
		}
		else if (interfaceVersion.compare("IVRDriverLog_001") == 0) {
			std::cout << "Return m_pVRDriverLog" << std::endl;
			return m_pVRDriverLog;
		}
		else {
			std::cout << "Got an unhandled type: " 
			          << pchInterfaceVersion << std::endl;
			return nullptr;
		}
	}
	
	// not sure which driver handle to return now
	vr::DriverHandle_t ViveDriverContext::GetDriverHandle() 
	{
		vr::DriverHandle_t dh = 1;
		return dh;
	}

} // namespace vive
} // namespace osvr
	

