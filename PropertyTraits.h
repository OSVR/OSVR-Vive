/** @file
    @brief Header - partially generated from parsing openvr_api.json

    @date 2016

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

/*
Copyright 2016 Razer Inc.

SPDX-License-Identifier: BSD-3-Clause

OpenVR input data:
Copyright (c) 2015, Valve Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef INCLUDED_PropertyTraits_h_GUID_6CC473E5_C8B9_46B7_237B_1E0C08E91076
#define INCLUDED_PropertyTraits_h_GUID_6CC473E5_C8B9_46B7_237B_1E0C08E91076

#ifndef _INCLUDE_VRTYPES_H
#error                                                                         \
    "Please include exactly one of openvr.h or openvr_driver.h before including this file"
#endif

#include <cstddef>
#include <string>

namespace osvr {
namespace vive {
    enum class Props {
        TrackingSystemName = vr::Prop_TrackingSystemName_String,
        ModelNumber = vr::Prop_ModelNumber_String,
        SerialNumber = vr::Prop_SerialNumber_String,
        RenderModelName = vr::Prop_RenderModelName_String,
        WillDriftInYaw = vr::Prop_WillDriftInYaw_Bool,
        ManufacturerName = vr::Prop_ManufacturerName_String,
        TrackingFirmwareVersion = vr::Prop_TrackingFirmwareVersion_String,
        HardwareRevision_String = vr::Prop_HardwareRevision_String,
        AllWirelessDongleDescriptions =
            vr::Prop_AllWirelessDongleDescriptions_String,
        ConnectedWirelessDongle = vr::Prop_ConnectedWirelessDongle_String,
        DeviceIsWireless = vr::Prop_DeviceIsWireless_Bool,
        DeviceIsCharging = vr::Prop_DeviceIsCharging_Bool,
        DeviceBatteryPercentage = vr::Prop_DeviceBatteryPercentage_Float,
        StatusDisplayTransform = vr::Prop_StatusDisplayTransform_Matrix34,
        Firmware_UpdateAvailable = vr::Prop_Firmware_UpdateAvailable_Bool,
        Firmware_ManualUpdate = vr::Prop_Firmware_ManualUpdate_Bool,
        Firmware_ManualUpdateURL = vr::Prop_Firmware_ManualUpdateURL_String,
        HardwareRevision_Uint64 = vr::Prop_HardwareRevision_Uint64,
        FirmwareVersion = vr::Prop_FirmwareVersion_Uint64,
        FPGAVersion = vr::Prop_FPGAVersion_Uint64,
        VRCVersion = vr::Prop_VRCVersion_Uint64,
        RadioVersion = vr::Prop_RadioVersion_Uint64,
        DongleVersion = vr::Prop_DongleVersion_Uint64,
        BlockServerShutdown = vr::Prop_BlockServerShutdown_Bool,
        CanUnifyCoordinateSystemWithHmd =
            vr::Prop_CanUnifyCoordinateSystemWithHmd_Bool,
        ContainsProximitySensor = vr::Prop_ContainsProximitySensor_Bool,
        DeviceProvidesBatteryStatus = vr::Prop_DeviceProvidesBatteryStatus_Bool,
        DeviceCanPowerOff = vr::Prop_DeviceCanPowerOff_Bool,
        Firmware_ProgrammingTarget = vr::Prop_Firmware_ProgrammingTarget_String,
        DeviceClass = vr::Prop_DeviceClass_Int32,
        HasCamera = vr::Prop_HasCamera_Bool,
        DriverVersion = vr::Prop_DriverVersion_String,
        Firmware_ForceUpdateRequired =
            vr::Prop_Firmware_ForceUpdateRequired_Bool,
        ViveSystemButtonFixRequired = vr::Prop_ViveSystemButtonFixRequired_Bool,
        ParentDriver = vr::Prop_ParentDriver_Uint64,
        ResourceRoot = vr::Prop_ResourceRoot_String,
        RegisteredDeviceType = vr::Prop_RegisteredDeviceType_String,
        InputProfilePath = vr::Prop_InputProfilePath_String,
        NeverTracked = vr::Prop_NeverTracked_Bool,
        NumCameras = vr::Prop_NumCameras_Int32,
        CameraFrameLayout = vr::Prop_CameraFrameLayout_Int32,
        ReportsTimeSinceVSync = vr::Prop_ReportsTimeSinceVSync_Bool,
        SecondsFromVsyncToPhotons = vr::Prop_SecondsFromVsyncToPhotons_Float,
        DisplayFrequency = vr::Prop_DisplayFrequency_Float,
        UserIpdMeters = vr::Prop_UserIpdMeters_Float,
        CurrentUniverseId = vr::Prop_CurrentUniverseId_Uint64,
        PreviousUniverseId = vr::Prop_PreviousUniverseId_Uint64,
        DisplayFirmwareVersion = vr::Prop_DisplayFirmwareVersion_Uint64,
        IsOnDesktop = vr::Prop_IsOnDesktop_Bool,
        DisplayMCType = vr::Prop_DisplayMCType_Int32,
        DisplayMCOffset = vr::Prop_DisplayMCOffset_Float,
        DisplayMCScale = vr::Prop_DisplayMCScale_Float,
        EdidVendorID = vr::Prop_EdidVendorID_Int32,
        DisplayMCImageLeft = vr::Prop_DisplayMCImageLeft_String,
        DisplayMCImageRight = vr::Prop_DisplayMCImageRight_String,
        DisplayGCBlackClamp = vr::Prop_DisplayGCBlackClamp_Float,
        EdidProductID = vr::Prop_EdidProductID_Int32,
        CameraToHeadTransform = vr::Prop_CameraToHeadTransform_Matrix34,
        DisplayGCType = vr::Prop_DisplayGCType_Int32,
        DisplayGCOffset = vr::Prop_DisplayGCOffset_Float,
        DisplayGCScale = vr::Prop_DisplayGCScale_Float,
        DisplayGCPrescale = vr::Prop_DisplayGCPrescale_Float,
        DisplayGCImage = vr::Prop_DisplayGCImage_String,
        LensCenterLeftU = vr::Prop_LensCenterLeftU_Float,
        LensCenterLeftV = vr::Prop_LensCenterLeftV_Float,
        LensCenterRightU = vr::Prop_LensCenterRightU_Float,
        LensCenterRightV = vr::Prop_LensCenterRightV_Float,
        UserHeadToEyeDepthMeters = vr::Prop_UserHeadToEyeDepthMeters_Float,
        CameraFirmwareVersion = vr::Prop_CameraFirmwareVersion_Uint64,
        CameraFirmwareDescription = vr::Prop_CameraFirmwareDescription_String,
        DisplayFPGAVersion = vr::Prop_DisplayFPGAVersion_Uint64,
        DisplayBootloaderVersion = vr::Prop_DisplayBootloaderVersion_Uint64,
        DisplayHardwareVersion = vr::Prop_DisplayHardwareVersion_Uint64,
        AudioFirmwareVersion = vr::Prop_AudioFirmwareVersion_Uint64,
        CameraCompatibilityMode = vr::Prop_CameraCompatibilityMode_Int32,
        ScreenshotHorizontalFieldOfViewDegrees =
            vr::Prop_ScreenshotHorizontalFieldOfViewDegrees_Float,
        ScreenshotVerticalFieldOfViewDegrees =
            vr::Prop_ScreenshotVerticalFieldOfViewDegrees_Float,
        DisplaySuppressed = vr::Prop_DisplaySuppressed_Bool,
        DisplayAllowNightMode = vr::Prop_DisplayAllowNightMode_Bool,
        DisplayMCImageWidth = vr::Prop_DisplayMCImageWidth_Int32,
        DisplayMCImageHeight = vr::Prop_DisplayMCImageHeight_Int32,
        DisplayMCImageNumChannels = vr::Prop_DisplayMCImageNumChannels_Int32,
        DisplayMCImageData = vr::Prop_DisplayMCImageData_Binary,
        SecondsFromPhotonsToVblank = vr::Prop_SecondsFromPhotonsToVblank_Float,
        DriverDirectModeSendsVsyncEvents =
            vr::Prop_DriverDirectModeSendsVsyncEvents_Bool,
        DisplayDebugMode = vr::Prop_DisplayDebugMode_Bool,
        GraphicsAdapterLuid = vr::Prop_GraphicsAdapterLuid_Uint64,
        DriverProvidedChaperonePath =
            vr::Prop_DriverProvidedChaperonePath_String,
        ExpectedTrackingReferenceCount =
            vr::Prop_ExpectedTrackingReferenceCount_Int32,
        ExpectedControllerCount = vr::Prop_ExpectedControllerCount_Int32,
        NamedIconPathControllerLeftDeviceOff =
            vr::Prop_NamedIconPathControllerLeftDeviceOff_String,
        NamedIconPathControllerRightDeviceOff =
            vr::Prop_NamedIconPathControllerRightDeviceOff_String,
        NamedIconPathTrackingReferenceDeviceOff =
            vr::Prop_NamedIconPathTrackingReferenceDeviceOff_String,
        DoNotApplyPrediction = vr::Prop_DoNotApplyPrediction_Bool,
        DistortionMeshResolution = vr::Prop_DistortionMeshResolution_Int32,
        DriverIsDrawingControllers = vr::Prop_DriverIsDrawingControllers_Bool,
        DriverRequestsApplicationPause =
            vr::Prop_DriverRequestsApplicationPause_Bool,
        DriverRequestsReducedRendering =
            vr::Prop_DriverRequestsReducedRendering_Bool,
        MinimumIpdStepMeters = vr::Prop_MinimumIpdStepMeters_Float,
        AudioBridgeFirmwareVersion = vr::Prop_AudioBridgeFirmwareVersion_Uint64,
        ImageBridgeFirmwareVersion = vr::Prop_ImageBridgeFirmwareVersion_Uint64,
        ImuToHeadTransform = vr::Prop_ImuToHeadTransform_Matrix34,
        ImuFactoryGyroBias = vr::Prop_ImuFactoryGyroBias_Vector3,
        ImuFactoryGyroScale = vr::Prop_ImuFactoryGyroScale_Vector3,
        ImuFactoryAccelerometerBias =
            vr::Prop_ImuFactoryAccelerometerBias_Vector3,
        ImuFactoryAccelerometerScale =
            vr::Prop_ImuFactoryAccelerometerScale_Vector3,
        ConfigurationIncludesLighthouse20Features =
            vr::Prop_ConfigurationIncludesLighthouse20Features_Bool,
        DriverRequestedMuraCorrectionMode =
            vr::Prop_DriverRequestedMuraCorrectionMode_Int32,
        DriverRequestedMuraFeather_InnerLeft =
            vr::Prop_DriverRequestedMuraFeather_InnerLeft_Int32,
        DriverRequestedMuraFeather_InnerRight =
            vr::Prop_DriverRequestedMuraFeather_InnerRight_Int32,
        DriverRequestedMuraFeather_InnerTop =
            vr::Prop_DriverRequestedMuraFeather_InnerTop_Int32,
        DriverRequestedMuraFeather_InnerBottom =
            vr::Prop_DriverRequestedMuraFeather_InnerBottom_Int32,
        DriverRequestedMuraFeather_OuterLeft =
            vr::Prop_DriverRequestedMuraFeather_OuterLeft_Int32,
        DriverRequestedMuraFeather_OuterRight =
            vr::Prop_DriverRequestedMuraFeather_OuterRight_Int32,
        DriverRequestedMuraFeather_OuterTop =
            vr::Prop_DriverRequestedMuraFeather_OuterTop_Int32,
        DriverRequestedMuraFeather_OuterBottom =
            vr::Prop_DriverRequestedMuraFeather_OuterBottom_Int32,
        AttachedDeviceId = vr::Prop_AttachedDeviceId_String,
        SupportedButtons = vr::Prop_SupportedButtons_Uint64,
        Axis0Type = vr::Prop_Axis0Type_Int32,
        Axis1Type = vr::Prop_Axis1Type_Int32,
        Axis2Type = vr::Prop_Axis2Type_Int32,
        Axis3Type = vr::Prop_Axis3Type_Int32,
        Axis4Type = vr::Prop_Axis4Type_Int32,
        ControllerRoleHint = vr::Prop_ControllerRoleHint_Int32,
        FieldOfViewLeftDegrees = vr::Prop_FieldOfViewLeftDegrees_Float,
        FieldOfViewRightDegrees = vr::Prop_FieldOfViewRightDegrees_Float,
        FieldOfViewTopDegrees = vr::Prop_FieldOfViewTopDegrees_Float,
        FieldOfViewBottomDegrees = vr::Prop_FieldOfViewBottomDegrees_Float,
        TrackingRangeMinimumMeters = vr::Prop_TrackingRangeMinimumMeters_Float,
        TrackingRangeMaximumMeters = vr::Prop_TrackingRangeMaximumMeters_Float,
        ModeLabel = vr::Prop_ModeLabel_String,
        IconPathName = vr::Prop_IconPathName_String,
        NamedIconPathDeviceOff = vr::Prop_NamedIconPathDeviceOff_String,
        NamedIconPathDeviceSearching =
            vr::Prop_NamedIconPathDeviceSearching_String,
        NamedIconPathDeviceSearchingAlert =
            vr::Prop_NamedIconPathDeviceSearchingAlert_String,
        NamedIconPathDeviceReady = vr::Prop_NamedIconPathDeviceReady_String,
        NamedIconPathDeviceReadyAlert =
            vr::Prop_NamedIconPathDeviceReadyAlert_String,
        NamedIconPathDeviceNotReady =
            vr::Prop_NamedIconPathDeviceNotReady_String,
        NamedIconPathDeviceStandby = vr::Prop_NamedIconPathDeviceStandby_String,
        NamedIconPathDeviceAlertLow =
            vr::Prop_NamedIconPathDeviceAlertLow_String,
        UserConfigPath = vr::Prop_UserConfigPath_String,
        InstallPath = vr::Prop_InstallPath_String,
        HasDisplayComponent = vr::Prop_HasDisplayComponent_Bool,
        HasControllerComponent = vr::Prop_HasControllerComponent_Bool,
        HasCameraComponent = vr::Prop_HasCameraComponent_Bool,
        HasDriverDirectModeComponent =
            vr::Prop_HasDriverDirectModeComponent_Bool,
        HasVirtualDisplayComponent = vr::Prop_HasVirtualDisplayComponent_Bool,
        HasSpatialAnchorsSupport = vr::Prop_HasSpatialAnchorsSupport_Bool,
        ControllerType = vr::Prop_ControllerType_String,
        LegacyInputProfile = vr::Prop_LegacyInputProfile_String
    };
    namespace detail {
        template <std::size_t EnumVal> struct PropertyTypeTrait;
        template <std::size_t EnumVal>
        using PropertyType = typename PropertyTypeTrait<EnumVal>::type;
        template <>
        struct PropertyTypeTrait<vr::Prop_TrackingSystemName_String> {
            using type = std::string;
        };
        template <> struct PropertyTypeTrait<vr::Prop_ModelNumber_String> {
            using type = std::string;
        };
        template <> struct PropertyTypeTrait<vr::Prop_SerialNumber_String> {
            using type = std::string;
        };
        template <> struct PropertyTypeTrait<vr::Prop_RenderModelName_String> {
            using type = std::string;
        };
        template <> struct PropertyTypeTrait<vr::Prop_WillDriftInYaw_Bool> {
            using type = bool;
        };
        template <> struct PropertyTypeTrait<vr::Prop_ManufacturerName_String> {
            using type = std::string;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_TrackingFirmwareVersion_String> {
            using type = std::string;
        };
        template <> struct PropertyTypeTrait<vr::Prop_HardwareRevision_String> {
            using type = std::string;
        };
        template <>
        struct PropertyTypeTrait<
            vr::Prop_AllWirelessDongleDescriptions_String> {
            using type = std::string;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_ConnectedWirelessDongle_String> {
            using type = std::string;
        };
        template <> struct PropertyTypeTrait<vr::Prop_DeviceIsWireless_Bool> {
            using type = bool;
        };
        template <> struct PropertyTypeTrait<vr::Prop_DeviceIsCharging_Bool> {
            using type = bool;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_DeviceBatteryPercentage_Float> {
            using type = float;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_StatusDisplayTransform_Matrix34> {
            using type = vr::HmdMatrix34_t;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_Firmware_UpdateAvailable_Bool> {
            using type = bool;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_Firmware_ManualUpdate_Bool> {
            using type = bool;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_Firmware_ManualUpdateURL_String> {
            using type = std::string;
        };
        template <> struct PropertyTypeTrait<vr::Prop_HardwareRevision_Uint64> {
            using type = uint64_t;
        };
        template <> struct PropertyTypeTrait<vr::Prop_FirmwareVersion_Uint64> {
            using type = uint64_t;
        };
        template <> struct PropertyTypeTrait<vr::Prop_FPGAVersion_Uint64> {
            using type = uint64_t;
        };
        template <> struct PropertyTypeTrait<vr::Prop_VRCVersion_Uint64> {
            using type = uint64_t;
        };
        template <> struct PropertyTypeTrait<vr::Prop_RadioVersion_Uint64> {
            using type = uint64_t;
        };
        template <> struct PropertyTypeTrait<vr::Prop_DongleVersion_Uint64> {
            using type = uint64_t;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_BlockServerShutdown_Bool> {
            using type = bool;
        };
        template <>
        struct PropertyTypeTrait<
            vr::Prop_CanUnifyCoordinateSystemWithHmd_Bool> {
            using type = bool;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_ContainsProximitySensor_Bool> {
            using type = bool;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_DeviceProvidesBatteryStatus_Bool> {
            using type = bool;
        };
        template <> struct PropertyTypeTrait<vr::Prop_DeviceCanPowerOff_Bool> {
            using type = bool;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_Firmware_ProgrammingTarget_String> {
            using type = std::string;
        };
        template <> struct PropertyTypeTrait<vr::Prop_DeviceClass_Int32> {
            using type = int32_t;
        };
        template <> struct PropertyTypeTrait<vr::Prop_HasCamera_Bool> {
            using type = bool;
        };
        template <> struct PropertyTypeTrait<vr::Prop_DriverVersion_String> {
            using type = std::string;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_Firmware_ForceUpdateRequired_Bool> {
            using type = bool;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_ViveSystemButtonFixRequired_Bool> {
            using type = bool;
        };
        template <> struct PropertyTypeTrait<vr::Prop_ParentDriver_Uint64> {
            using type = uint64_t;
        };
        template <> struct PropertyTypeTrait<vr::Prop_ResourceRoot_String> {
            using type = std::string;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_RegisteredDeviceType_String> {
            using type = std::string;
        };
        template <> struct PropertyTypeTrait<vr::Prop_InputProfilePath_String> {
            using type = std::string;
        };
        template <> struct PropertyTypeTrait<vr::Prop_NeverTracked_Bool> {
            using type = bool;
        };
        template <> struct PropertyTypeTrait<vr::Prop_NumCameras_Int32> {
            using type = int32_t;
        };
        template <> struct PropertyTypeTrait<vr::Prop_CameraFrameLayout_Int32> {
            using type = int32_t;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_ReportsTimeSinceVSync_Bool> {
            using type = bool;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_SecondsFromVsyncToPhotons_Float> {
            using type = float;
        };
        template <> struct PropertyTypeTrait<vr::Prop_DisplayFrequency_Float> {
            using type = float;
        };
        template <> struct PropertyTypeTrait<vr::Prop_UserIpdMeters_Float> {
            using type = float;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_CurrentUniverseId_Uint64> {
            using type = uint64_t;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_PreviousUniverseId_Uint64> {
            using type = uint64_t;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_DisplayFirmwareVersion_Uint64> {
            using type = uint64_t;
        };
        template <> struct PropertyTypeTrait<vr::Prop_IsOnDesktop_Bool> {
            using type = bool;
        };
        template <> struct PropertyTypeTrait<vr::Prop_DisplayMCType_Int32> {
            using type = int32_t;
        };
        template <> struct PropertyTypeTrait<vr::Prop_DisplayMCOffset_Float> {
            using type = float;
        };
        template <> struct PropertyTypeTrait<vr::Prop_DisplayMCScale_Float> {
            using type = float;
        };
        template <> struct PropertyTypeTrait<vr::Prop_EdidVendorID_Int32> {
            using type = int32_t;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_DisplayMCImageLeft_String> {
            using type = std::string;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_DisplayMCImageRight_String> {
            using type = std::string;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_DisplayGCBlackClamp_Float> {
            using type = float;
        };
        template <> struct PropertyTypeTrait<vr::Prop_EdidProductID_Int32> {
            using type = int32_t;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_CameraToHeadTransform_Matrix34> {
            using type = vr::HmdMatrix34_t;
        };
        template <> struct PropertyTypeTrait<vr::Prop_DisplayGCType_Int32> {
            using type = int32_t;
        };
        template <> struct PropertyTypeTrait<vr::Prop_DisplayGCOffset_Float> {
            using type = float;
        };
        template <> struct PropertyTypeTrait<vr::Prop_DisplayGCScale_Float> {
            using type = float;
        };
        template <> struct PropertyTypeTrait<vr::Prop_DisplayGCPrescale_Float> {
            using type = float;
        };
        template <> struct PropertyTypeTrait<vr::Prop_DisplayGCImage_String> {
            using type = std::string;
        };
        template <> struct PropertyTypeTrait<vr::Prop_LensCenterLeftU_Float> {
            using type = float;
        };
        template <> struct PropertyTypeTrait<vr::Prop_LensCenterLeftV_Float> {
            using type = float;
        };
        template <> struct PropertyTypeTrait<vr::Prop_LensCenterRightU_Float> {
            using type = float;
        };
        template <> struct PropertyTypeTrait<vr::Prop_LensCenterRightV_Float> {
            using type = float;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_UserHeadToEyeDepthMeters_Float> {
            using type = float;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_CameraFirmwareVersion_Uint64> {
            using type = uint64_t;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_CameraFirmwareDescription_String> {
            using type = std::string;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_DisplayFPGAVersion_Uint64> {
            using type = uint64_t;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_DisplayBootloaderVersion_Uint64> {
            using type = uint64_t;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_DisplayHardwareVersion_Uint64> {
            using type = uint64_t;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_AudioFirmwareVersion_Uint64> {
            using type = uint64_t;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_CameraCompatibilityMode_Int32> {
            using type = int32_t;
        };
        template <>
        struct PropertyTypeTrait<
            vr::Prop_ScreenshotHorizontalFieldOfViewDegrees_Float> {
            using type = float;
        };
        template <>
        struct PropertyTypeTrait<
            vr::Prop_ScreenshotVerticalFieldOfViewDegrees_Float> {
            using type = float;
        };
        template <> struct PropertyTypeTrait<vr::Prop_DisplaySuppressed_Bool> {
            using type = bool;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_DisplayAllowNightMode_Bool> {
            using type = bool;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_DisplayMCImageWidth_Int32> {
            using type = int32_t;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_DisplayMCImageHeight_Int32> {
            using type = int32_t;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_DisplayMCImageNumChannels_Int32> {
            using type = int32_t;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_DisplayMCImageData_Binary> {
            using type = void *;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_SecondsFromPhotonsToVblank_Float> {
            using type = float;
        };
        template <>
        struct PropertyTypeTrait<
            vr::Prop_DriverDirectModeSendsVsyncEvents_Bool> {
            using type = bool;
        };
        template <> struct PropertyTypeTrait<vr::Prop_DisplayDebugMode_Bool> {
            using type = bool;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_GraphicsAdapterLuid_Uint64> {
            using type = uint64_t;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_DriverProvidedChaperonePath_String> {
            using type = std::string;
        };
        template <>
        struct PropertyTypeTrait<
            vr::Prop_ExpectedTrackingReferenceCount_Int32> {
            using type = int32_t;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_ExpectedControllerCount_Int32> {
            using type = int32_t;
        };
        template <>
        struct PropertyTypeTrait<
            vr::Prop_NamedIconPathControllerLeftDeviceOff_String> {
            using type = std::string;
        };
        template <>
        struct PropertyTypeTrait<
            vr::Prop_NamedIconPathControllerRightDeviceOff_String> {
            using type = std::string;
        };
        template <>
        struct PropertyTypeTrait<
            vr::Prop_NamedIconPathTrackingReferenceDeviceOff_String> {
            using type = std::string;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_DoNotApplyPrediction_Bool> {
            using type = bool;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_DistortionMeshResolution_Int32> {
            using type = int32_t;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_DriverIsDrawingControllers_Bool> {
            using type = bool;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_DriverRequestsApplicationPause_Bool> {
            using type = bool;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_DriverRequestsReducedRendering_Bool> {
            using type = bool;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_MinimumIpdStepMeters_Float> {
            using type = float;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_AudioBridgeFirmwareVersion_Uint64> {
            using type = uint64_t;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_ImageBridgeFirmwareVersion_Uint64> {
            using type = uint64_t;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_ImuToHeadTransform_Matrix34> {
            using type = vr::HmdMatrix34_t;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_ImuFactoryGyroBias_Vector3> {
            using type = vr::HmdVector3d_t;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_ImuFactoryGyroScale_Vector3> {
            using type = vr::HmdVector3d_t;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_ImuFactoryAccelerometerBias_Vector3> {
            using type = vr::HmdVector3d_t;
        };
        template <>
        struct PropertyTypeTrait<
            vr::Prop_ImuFactoryAccelerometerScale_Vector3> {
            using type = vr::HmdVector3d_t;
        };
        template <>
        struct PropertyTypeTrait<
            vr::Prop_ConfigurationIncludesLighthouse20Features_Bool> {
            using type = bool;
        };
        template <>
        struct PropertyTypeTrait<
            vr::Prop_DriverRequestedMuraCorrectionMode_Int32> {
            using type = int32_t;
        };
        template <>
        struct PropertyTypeTrait<
            vr::Prop_DriverRequestedMuraFeather_InnerLeft_Int32> {
            using type = int32_t;
        };
        template <>
        struct PropertyTypeTrait<
            vr::Prop_DriverRequestedMuraFeather_InnerRight_Int32> {
            using type = int32_t;
        };
        template <>
        struct PropertyTypeTrait<
            vr::Prop_DriverRequestedMuraFeather_InnerTop_Int32> {
            using type = int32_t;
        };
        template <>
        struct PropertyTypeTrait<
            vr::Prop_DriverRequestedMuraFeather_InnerBottom_Int32> {
            using type = int32_t;
        };
        template <>
        struct PropertyTypeTrait<
            vr::Prop_DriverRequestedMuraFeather_OuterLeft_Int32> {
            using type = int32_t;
        };
        template <>
        struct PropertyTypeTrait<
            vr::Prop_DriverRequestedMuraFeather_OuterRight_Int32> {
            using type = int32_t;
        };
        template <>
        struct PropertyTypeTrait<
            vr::Prop_DriverRequestedMuraFeather_OuterTop_Int32> {
            using type = int32_t;
        };
        template <>
        struct PropertyTypeTrait<
            vr::Prop_DriverRequestedMuraFeather_OuterBottom_Int32> {
            using type = int32_t;
        };
        template <> struct PropertyTypeTrait<vr::Prop_AttachedDeviceId_String> {
            using type = std::string;
        };
        template <> struct PropertyTypeTrait<vr::Prop_SupportedButtons_Uint64> {
            using type = uint64_t;
        };
        template <> struct PropertyTypeTrait<vr::Prop_Axis0Type_Int32> {
            using type = int32_t;
        };
        template <> struct PropertyTypeTrait<vr::Prop_Axis1Type_Int32> {
            using type = int32_t;
        };
        template <> struct PropertyTypeTrait<vr::Prop_Axis2Type_Int32> {
            using type = int32_t;
        };
        template <> struct PropertyTypeTrait<vr::Prop_Axis3Type_Int32> {
            using type = int32_t;
        };
        template <> struct PropertyTypeTrait<vr::Prop_Axis4Type_Int32> {
            using type = int32_t;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_ControllerRoleHint_Int32> {
            using type = int32_t;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_FieldOfViewLeftDegrees_Float> {
            using type = float;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_FieldOfViewRightDegrees_Float> {
            using type = float;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_FieldOfViewTopDegrees_Float> {
            using type = float;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_FieldOfViewBottomDegrees_Float> {
            using type = float;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_TrackingRangeMinimumMeters_Float> {
            using type = float;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_TrackingRangeMaximumMeters_Float> {
            using type = float;
        };
        template <> struct PropertyTypeTrait<vr::Prop_ModeLabel_String> {
            using type = std::string;
        };
        template <> struct PropertyTypeTrait<vr::Prop_IconPathName_String> {
            using type = std::string;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_NamedIconPathDeviceOff_String> {
            using type = std::string;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_NamedIconPathDeviceSearching_String> {
            using type = std::string;
        };
        template <>
        struct PropertyTypeTrait<
            vr::Prop_NamedIconPathDeviceSearchingAlert_String> {
            using type = std::string;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_NamedIconPathDeviceReady_String> {
            using type = std::string;
        };
        template <>
        struct PropertyTypeTrait<
            vr::Prop_NamedIconPathDeviceReadyAlert_String> {
            using type = std::string;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_NamedIconPathDeviceNotReady_String> {
            using type = std::string;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_NamedIconPathDeviceStandby_String> {
            using type = std::string;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_NamedIconPathDeviceAlertLow_String> {
            using type = std::string;
        };
        template <> struct PropertyTypeTrait<vr::Prop_UserConfigPath_String> {
            using type = std::string;
        };
        template <> struct PropertyTypeTrait<vr::Prop_InstallPath_String> {
            using type = std::string;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_HasDisplayComponent_Bool> {
            using type = bool;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_HasControllerComponent_Bool> {
            using type = bool;
        };
        template <> struct PropertyTypeTrait<vr::Prop_HasCameraComponent_Bool> {
            using type = bool;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_HasDriverDirectModeComponent_Bool> {
            using type = bool;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_HasVirtualDisplayComponent_Bool> {
            using type = bool;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_HasSpatialAnchorsSupport_Bool> {
            using type = bool;
        };
        template <> struct PropertyTypeTrait<vr::Prop_ControllerType_String> {
            using type = std::string;
        };
        template <>
        struct PropertyTypeTrait<vr::Prop_LegacyInputProfile_String> {
            using type = std::string;
        };
    } // namespace detail

} // namespace vive
} // namespace osvr

#endif // INCLUDED_PropertyTraits_h_GUID_6CC473E5_C8B9_46B7_237B_1E0C08E91076
