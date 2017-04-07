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

#ifndef INCLUDED_VRDriverContext_h_GUID_28E4DDCE_CA2A_4987_FA60_A63AF5C87993
#define INCLUDED_VRDriverContext_h_GUID_28E4DDCE_CA2A_4987_FA60_A63AF5C87993


// Internal 
#include "ServerDriverHost.h"
#include "VRSettings.h"

// Library/third-party includes
#include <openvr_driver.h>

// Standard includes
#include <iostream>
#include <memory>
#include <string>
#include <utility>

namespace vr{
	class ViveDriverContext : public vr::IVRDriverContext {
	  public:
	    ViveDriverContext();
		ViveDriverContext(vr::ServerDriverHost *serverDriverHost, vr::VRSettings *settings);
		                 
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
	

#endif // INCLUDED_VRDriverContext_h_GUID_28E4DDCE_CA2A_4987_FA60_A63AF5C87993

