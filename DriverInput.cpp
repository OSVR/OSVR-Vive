/** @file
    @brief Implementation

    @date 2018

    @author
    Georgiy Frolov
    <http://github.com/mars979>
*/

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
#include "DriverInput.h"

// Library/third-party includes
// - none

// Standard includes
// - none

using namespace vr;

DriverInput::DriverInput()
    : m_logger(osvr::util::log::make_logger("IOBuffer")) {}

EVRInputError
DriverInput::CreateBooleanComponent(PropertyContainerHandle_t ulContainer,
                                    const char *pchName,
                                    VRInputComponentHandle_t * /*pHandle*/) {
    m_logger->info("CreateBooleanComponent(")
        << ulContainer << ", " << pchName << ")";

    return VRInputError_InvalidHandle;
}

EVRInputError
DriverInput::UpdateBooleanComponent(VRInputComponentHandle_t ulComponent,
                                    bool bNewValue, double fTimeOffset) {
    m_logger->info("UpdateBooleanComponent(")
        << ulComponent << ", " << bNewValue << ", " << fTimeOffset << ")";

    return VRInputError_InvalidHandle;
}

EVRInputError
DriverInput::CreateScalarComponent(PropertyContainerHandle_t ulContainer,
                                   const char *pchName,
                                   VRInputComponentHandle_t *pHandle,
                                   EVRScalarType eType, EVRScalarUnits eUnits) {

    m_logger->info("CreateScalarComponent(")
        << ulContainer << ", " << pchName << ", " << pHandle << ", " << eType
        << ", " << eUnits << ")";

    return VRInputError_InvalidHandle;
}

EVRInputError
DriverInput::UpdateScalarComponent(VRInputComponentHandle_t ulComponent,
                                   float fNewValue, double fTimeOffset) {
    m_logger->info("UpdateScalarComponent(")
        << ulComponent << ", " << fNewValue << ", " << fTimeOffset << ")";

    return VRInputError_InvalidHandle;
}

EVRInputError
DriverInput::CreateHapticComponent(PropertyContainerHandle_t ulContainer,
                                   const char *pchName,
                                   VRInputComponentHandle_t *pHandle) {
    m_logger->info("CreateHapticComponent(")
        << ulContainer << ", " << pchName << ", " << pHandle << ")";

    return VRInputError_InvalidHandle;
}

EVRInputError DriverInput::CreateSkeletonComponent(
    PropertyContainerHandle_t ulContainer, const char *pchName,
    const char *pchSkeletonPath, const char *pchBasePosePath,
    const VRBoneTransform_t * /*pGripLimitTransforms*/,
    uint32_t unGripLimitTransformCount, VRInputComponentHandle_t *pHandle) {
    m_logger->info("CreateSkeletonComponent(")
        << ulContainer << ", " << pchName << ", " << pchSkeletonPath << ", "
        << pchBasePosePath << ", " << unGripLimitTransformCount << ", "
        << pHandle << ")";

    return VRInputError_InvalidHandle;
}

EVRInputError
DriverInput::UpdateSkeletonComponent(VRInputComponentHandle_t ulComponent,
                                     const VRBoneTransform_t * /*pTransforms*/,
                                     uint32_t unTransformCount) {
    m_logger->info("UpdateSkeletonComponent(")
        << ulComponent << ", " << unTransformCount << ")";

    return VRInputError_InvalidHandle;
}
