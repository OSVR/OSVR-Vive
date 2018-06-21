/** @file
    @brief Header

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

#ifndef INCLUDED_DriverInput_h_GUID_EC9410DD_A0AF_41AA_08D2_ED35BEAD26E1
#define INCLUDED_DriverInput_h_GUID_EC9410DD_A0AF_41AA_08D2_ED35BEAD26E1

// Internal Includes
#include "Properties.h"
#include <osvr/Util/Logger.h>

// Library/third-party includes
#include <openvr_driver.h>

// Standard includes

namespace vr {

class DriverInput : public vr::IVRDriverInput {
  public:
    DriverInput();

    /** Creates a boolean input component for the device */
    virtual EVRInputError
    CreateBooleanComponent(PropertyContainerHandle_t ulContainer,
                           const char *pchName,
                           VRInputComponentHandle_t *pHandle);

    /** Updates a boolean component */
    virtual EVRInputError
    UpdateBooleanComponent(VRInputComponentHandle_t ulComponent, bool bNewValue,
                           double fTimeOffset);

    /** Creates a scalar input component for the device */
    virtual EVRInputError
    CreateScalarComponent(PropertyContainerHandle_t ulContainer,
                          const char *pchName,
                          VRInputComponentHandle_t *pHandle,
                          EVRScalarType eType, EVRScalarUnits eUnits);

    /** Updates a boolean component */
    virtual EVRInputError
    UpdateScalarComponent(VRInputComponentHandle_t ulComponent, float fNewValue,
                          double fTimeOffset);

    /** Creates a haptic component for the device */
    virtual EVRInputError
    CreateHapticComponent(PropertyContainerHandle_t ulContainer,
                          const char *pchName,
                          VRInputComponentHandle_t *pHandle);

    /** Creates a skeleton component. */
    virtual EVRInputError CreateSkeletonComponent(
        PropertyContainerHandle_t ulContainer, const char *pchName,
        const char *pchSkeletonPath, const char *pchBasePosePath,
        const VRBoneTransform_t *pGripLimitTransforms,
        uint32_t unGripLimitTransformCount, VRInputComponentHandle_t *pHandle);

    /** Updates a skeleton component. */
    virtual EVRInputError
    UpdateSkeletonComponent(VRInputComponentHandle_t ulComponent,
                            const VRBoneTransform_t *pTransforms,
                            uint32_t unTransformCount);

  private:
    osvr::util::log::LoggerPtr m_logger;
    vr::Properties m_properties;
};

} // namespace vr

#endif // INCLUDED_DriverInput_h_GUID_EC9410DD_A0AF_41AA_08D2_ED35BEAD26E1
