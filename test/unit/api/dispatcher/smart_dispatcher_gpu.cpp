/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>

#include "./smart_dispatcher.h"
#include "api/unit_api.h"

// MFXEnumImplementations
TEST(Dispatcher_GPU_EnumImplementations, ValidInputsReturnValidDesc) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_EnumImplementations_ValidInputsReturnValidDesc(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_EnumImplementations, NullLoaderReturnsErrNull) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_EnumImplementations_NullLoaderReturnsErrNull(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_EnumImplementations, NullDescReturnsErrNull) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_EnumImplementations_NullDescReturnsErrNull(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_EnumImplementations, IndexOutOfRangeReturnsNotFound) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_EnumImplementations_IndexOutOfRangeReturnsNotFound(MFX_IMPL_TYPE_HARDWARE);
}

// MFXCreateSession
TEST(Dispatcher_GPU_CreateSession, SimpleConfigCanCreateSession) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_SimpleConfigCanCreateSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, UnusedCfgCreatesSession) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_UnusedCfgCreatesSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestSWImplCreatesSession) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestSWImplCreatesSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, DoubleLoadersCreatesTwoSWSessions) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_DoubleLoadersCreatesTwoSWSessions(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, DoubleConfigObjsCreatesTwoSessions) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_DoubleConfigObjsCreatesTwoSessions(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, NullLoaderReturnsErrNull) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_NullLoaderReturnsErrNull(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, NullSessionReturnsErrNull) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_NullSessionReturnsErrNull(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, InvalidIndexReturnsErrNotFound) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_InvalidIndexReturnsErrNotFound(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestSupportedDecoderCreatesSession) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_CreateSession_RequestSupportedDecoderCreatesSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestSupportedEncoderCreatesSession) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_CreateSession_RequestSupportedEncoderCreatesSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestSupportedVPPCreatesSession) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_CreateSession_RequestSupportedVPPCreatesSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestUnsupportedDecoderReturnsErrNotFound) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_CreateSession_RequestUnsupportedDecoderReturnsErrNotFound(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestUnsupportedEncoderReturnsErrNotFound) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_CreateSession_RequestUnsupportedEncoderReturnsErrNotFound(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestTwoSupportedDecodersReturnsErrNone) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_CreateSession_RequestTwoSupportedDecodersReturnsErrNone(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestMixedDecodersReturnsErrNotFound) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_CreateSession_RequestMixedDecodersReturnsErrNotFound(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestSupportedAccelModeCreatesSession) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestSupportedAccelModeCreatesSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestAccelInvalidReturnsNotFound) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestUnsupportedAccelModeReturnsNotFound(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestCurrentAPIVersionCreatesSession) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestCurrentAPIVersionCreatesSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestLowerAPIVersionCreatesSession) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestLowerAPIVersionCreatesSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestHigherAPIVersionReturnsNotFound) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestHigherAPIVersionReturnsNotFound(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestImplementedFunctionCreatesSession) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestImplementedFunctionCreatesSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestNotImplementedFunctionReturnsNotFound) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestNotImplementedFunctionReturnsNotFound(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestCurrentAPIMajorMinorCreatesSession) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestCurrentAPIMajorMinorCreatesSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestHigherAPIMajorReturnsNotFound) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestHigherAPIMajorReturnsNotFound(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestHigherAPIMinorReturnsNotFound) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestHigherAPIMinorReturnsNotFound(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestDeviceIDInvalidReturnsErrNotFound) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestDeviceIDInvalidReturnsErrNotFound(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestImplNameValidReturnsErrNone) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_CreateSession_RequestImplNameValidReturnsErrNone(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestImplNameInvalidReturnsErrNotFound) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestImplNameInvalidReturnsErrNotFound(MFX_IMPL_TYPE_HARDWARE);
}

// TO DO - GPU RT has not implemented license yet
TEST(DISABLED_Dispatcher_GPU_CreateSession, RequestLicenseValidReturnsErrNone) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestLicenseValidReturnsErrNone(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestLicenseInvalidReturnsErrNotFound) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestLicenseInvalidReturnsErrNotFound(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestLicenseMixedReturnsErrNotFound) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestLicenseMixedReturnsErrNotFound(MFX_IMPL_TYPE_HARDWARE);
}

// TO DO - GPU RT has not implemented keywords yet
TEST(DISABLED_Dispatcher_GPU_CreateSession, RequestKeywordsValidReturnsErrNone) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestKeywordsValidReturnsErrNone(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestKeywordsMixedReturnsErrNotFound) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestKeywordsMixedReturnsErrNotFound(MFX_IMPL_TYPE_HARDWARE);
}

// TO DO - need to create real device in order to set handle
TEST(DISABLED_Dispatcher_GPU_CreateSession, ConfigHandleReturnsHandle) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_ConfigHandleReturnsHandle(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestValidDXGIAdapterCreatesSession) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestValidDXGIAdapterCreatesSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestInvalidDXGIAdapterReturnsErrNotFound) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestInvalidDXGIAdapterReturnsErrNotFound(MFX_IMPL_TYPE_HARDWARE);
}

// MFXDispReleaseImplDescription
TEST(Dispatcher_GPU_DispReleaseImplDescription, ValidInputReturnsErrNone) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_DispReleaseImplDescription_ValidInputReturnsErrNone(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_DispReleaseImplDescription, NullLoaderReturnsErrNull) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_DispReleaseImplDescription_NullLoaderReturnsErrNull(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_DispReleaseImplDescription, NullDescReturnsErrNull) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_DispReleaseImplDescription_NullDescReturnsErrNull(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_DispReleaseImplDescription, HandleMismatchReturnsInvalidHandle) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_DispReleaseImplDescription_HandleMismatchReturnsInvalidHandle(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_DispReleaseImplDescription, ReleaseTwiceReturnsErrNone) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_DispReleaseImplDescription_ReleaseTwiceReturnsErrNone(MFX_IMPL_TYPE_HARDWARE);
}

// MFXSetConfigFilterProperty - multiple props per cfg object

TEST(Dispatcher_GPU_MultiProp, DecEncValid) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_MultiProp_DecEncValid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, DecEncInvalid) {
    // MSDK will not test invalid enc
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_MultiProp_DecEncInvalid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, APIMajorMinorValid) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_MultiProp_APIMajorMinorValid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, APIMajorInvalid) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_MultiProp_APIMajorInvalid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, APIMinorInvalid) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_MultiProp_APIMinorInvalid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, APIPartialValid) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_MultiProp_APIPartialValid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, MultiConfigMultiPropValid) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_MultiProp_MultiConfigMultiPropValid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, MultiConfigMultiPropInvalid) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_MultiProp_MultiConfigMultiPropInvalid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, MultiConfigOverwriteValid) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_MultiProp_MultiConfigOverwriteValid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, MultiConfigOverwriteInvalid) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_MultiProp_MultiConfigOverwriteInvalid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, MultiConfigCodecProfileValid) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_MultiProp_MultiConfigCodecProfileValid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, MultiConfigCodecProfileInvalid) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_MultiProp_MultiConfigCodecProfileInvalid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, MultiConfigMultiCodecMultiProfileValid) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_MultiProp_MultiConfigMultiCodecMultiProfileValid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, MultiConfigMultiCodecMultiProfileReorderValid) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_MultiProp_MultiConfigMultiCodecMultiProfileReorderValid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, MultiConfigMultiCodecMultiProfileReorder2Valid) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_MultiProp_MultiConfigMultiCodecMultiProfileReorder2Valid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, MultiConfigMultiCodecMultiProfileReorderInvalid) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_MultiProp_MultiConfigMultiCodecMultiProfileReorderInvalid(MFX_IMPL_TYPE_HARDWARE);
}
