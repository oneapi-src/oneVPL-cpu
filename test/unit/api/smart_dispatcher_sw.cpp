/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>

#include "api/smart_dispatcher.h"
#include "api/unit_api.h"

//MFXEnumImplementations
TEST(Dispatcher_SW_EnumImplementations, ValidInputsReturnValidDesc) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_EnumImplementations_ValidInputsReturnValidDesc(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_EnumImplementations, NullLoaderReturnsErrNull) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_EnumImplementations_NullLoaderReturnsErrNull(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_EnumImplementations, NullDescReturnsErrNull) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_EnumImplementations_NullDescReturnsErrNull(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_EnumImplementations, IndexOutOfRangeReturnsNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_EnumImplementations_IndexOutOfRangeReturnsNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

//MFXCreateSession
TEST(Dispatcher_SW_CreateSession, SimpleConfigCanCreateSession) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_SimpleConfigCanCreateSession(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, UnusedCfgCreatesSession) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_UnusedCfgCreatesSession(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestSWImplCreatesSession) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestSWImplCreatesSession(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, DoubleLoadersCreatesTwoSWSessions) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_DoubleLoadersCreatesTwoSWSessions(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, DoubleConfigObjsCreatesTwoSessions) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_DoubleConfigObjsCreatesTwoSessions(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, NullLoaderReturnsErrNull) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_NullLoaderReturnsErrNull(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, NullSessionReturnsErrNull) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_NullSessionReturnsErrNull(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, InvalidIndexReturnsErrNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_InvalidIndexReturnsErrNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestSupportedDecoderCreatesSession) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestSupportedDecoderCreatesSession(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestSupportedEncoderCreatesSession) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestSupportedEncoderCreatesSession(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestSupportedVPPCreatesSession) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestSupportedVPPCreatesSession(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestUnsupportedDecoderReturnsErrNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestUnsupportedDecoderReturnsErrNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestUnsupportedEncoderReturnsErrNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestUnsupportedEncoderReturnsErrNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestTwoSupportedDecodersReturnsErrNone) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestTwoSupportedDecodersReturnsErrNone(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestMixedDecodersReturnsErrNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestMixedDecodersReturnsErrNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

// TO DO - changed to "supported accel mode"
TEST(Dispatcher_SW_CreateSession, RequestAccelValidCreatesSession) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestAccelValidCreatesSession(MFX_IMPL_TYPE_SOFTWARE);
}

// TO DO - changed to "unsupported accel mode"
TEST(Dispatcher_SW_CreateSession, RequestAccelInvalidReturnsNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestAccelInvalidReturnsNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestCurrentAPIVersionCreatesSession) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestCurrentAPIVersionCreatesSession(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestLowerAPIVersionCreatesSession) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestLowerAPIVersionCreatesSession(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestHigherAPIVersionReturnsNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestHigherAPIVersionReturnsNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestImplementedFunctionCreatesSession) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestImplementedFunctionCreatesSession(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestNotImplementedFunctionReturnsNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestNotImplementedFunctionReturnsNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestCurrentAPIMajorMinorCreatesSession) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestCurrentAPIMajorMinorCreatesSession(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestHigherAPIMajorReturnsNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestHigherAPIMajorReturnsNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestHigherAPIMinorReturnsNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestHigherAPIMinorReturnsNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestDeviceIDValidReturnsErrNone) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestDeviceIDValidReturnsErrNone(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestDeviceIDInvalidReturnsErrNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestDeviceIDInvalidReturnsErrNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestImplNameValidReturnsErrNone) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestImplNameValidReturnsErrNone(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestImplNameInvalidReturnsErrNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestImplNameInvalidReturnsErrNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestLicenseValidReturnsErrNone) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestLicenseValidReturnsErrNone(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestLicenseInvalidReturnsErrNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestLicenseInvalidReturnsErrNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestLicenseMixedReturnsErrNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestLicenseMixedReturnsErrNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestKeywordsValidReturnsErrNone) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestKeywordsValidReturnsErrNone(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestKeywordsMixedReturnsErrNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestKeywordsMixedReturnsErrNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, ConfigHandleReturnsHandle) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_ConfigHandleReturnsHandle(MFX_IMPL_TYPE_SOFTWARE);
}

//MFXDispReleaseImplDescription
TEST(Dispatcher_SW_DispReleaseImplDescription, ValidInputReturnsErrNone) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_DispReleaseImplDescription_ValidInputReturnsErrNone(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_DispReleaseImplDescription, NullLoaderReturnsErrNull) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_DispReleaseImplDescription_NullLoaderReturnsErrNull(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_DispReleaseImplDescription, NullDescReturnsErrNull) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_DispReleaseImplDescription_NullDescReturnsErrNull(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_DispReleaseImplDescription, HandleMismatchReturnsInvalidHandle) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_DispReleaseImplDescription_HandleMismatchReturnsInvalidHandle(
        MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_DispReleaseImplDescription, ReleaseTwiceReturnsErrNone) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_DispReleaseImplDescription_ReleaseTwiceReturnsErrNone(MFX_IMPL_TYPE_SOFTWARE);
}
