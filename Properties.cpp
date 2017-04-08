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
#include "Properties.h"

// Library/third-party includes
// - none

// Standard includes
#include <string>

using namespace vr;
Properties::Properties() : m_logger(osvr::util::log::make_logger("DriverLog")) {}

ETrackedPropertyError Properties::ReadPropertyBatch( 
		PropertyContainerHandle_t ulContainerHandle, 
		PropertyRead_t *pBatch, uint32_t unBatchEntryCount ) {
	return vr::TrackedProp_Success;
}

ETrackedPropertyError Properties::WritePropertyBatch(
		PropertyContainerHandle_t ulContainerHandle, 
		PropertyWrite_t *pBatch, uint32_t unBatchEntryCount ) {
	return vr::TrackedProp_Success;
}

const char *Properties::GetPropErrorNameFromEnum( 
		ETrackedPropertyError error ) {
	return nullptr;
}

PropertyContainerHandle_t Properties::TrackedDeviceToPropertyContainer( 
		TrackedDeviceIndex_t nDevice ) {
	PropertyContainerHandle_t propertyContainer = 
	  vr::VRProperties()->TrackedDeviceToPropertyContainer(nDevice);	
    return 	propertyContainer; 
}
	


