/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef TEST_UNIT_API_SMART_DISPATCHER_H_
#define TEST_UNIT_API_SMART_DISPATCHER_H_

#include "vpl/mfxdispatcher.h"
#include "vpl/mfximplcaps.h"
#include "vpl/mfxvideo.h"

// helper functions for dispatcher tests

// set implementation type
static __inline mfxStatus SetConfigImpl(mfxLoader loader, mfxU32 implType) {
    mfxVariant ImplValue;
    mfxConfig cfg = MFXCreateConfig(loader);
    if (!cfg)
        return MFX_ERR_UNSUPPORTED;

    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = implType;

    return MFXSetConfigFilterProperty(cfg,
                                      reinterpret_cast<const mfxU8 *>("mfxImplDescription.Impl"),
                                      ImplValue);
}

// create mfxConfig object and apply to loader
template <typename varDataType>
mfxStatus SetConfigFilterProperty(mfxLoader loader, const char *name, varDataType data);

// common kernels - set implType for SW, GPU, etc.
void Dispatcher_EnumImplementations_ValidInputsReturnValidDesc(mfxImplType implType);
void Dispatcher_EnumImplementations_NullLoaderReturnsErrNull(mfxImplType implType);
void Dispatcher_EnumImplementations_NullDescReturnsErrNull(mfxImplType implType);
void Dispatcher_EnumImplementations_IndexOutOfRangeReturnsNotFound(mfxImplType implType);

void Dispatcher_CreateSession_SimpleConfigCanCreateSession(mfxImplType implType);
void Dispatcher_CreateSession_UnusedCfgCreatesSession(mfxImplType implType);
void Dispatcher_CreateSession_RequestSWImplCreatesSession(mfxImplType implType);
void Dispatcher_CreateSession_DoubleLoadersCreatesTwoSWSessions(mfxImplType implType);
void Dispatcher_CreateSession_DoubleConfigObjsCreatesTwoSessions(mfxImplType implType);
void Dispatcher_CreateSession_NullLoaderReturnsErrNull(mfxImplType implType);
void Dispatcher_CreateSession_NullSessionReturnsErrNull(mfxImplType implType);
void Dispatcher_CreateSession_InvalidIndexReturnsErrNotFound(mfxImplType implType);
void Dispatcher_CreateSession_RequestSupportedDecoderCreatesSession(mfxImplType implType);
void Dispatcher_CreateSession_RequestSupportedEncoderCreatesSession(mfxImplType implType);
void Dispatcher_CreateSession_RequestSupportedVPPCreatesSession(mfxImplType implType);
void Dispatcher_CreateSession_RequestUnsupportedDecoderReturnsErrNotFound(mfxImplType implType);
void Dispatcher_CreateSession_RequestUnsupportedEncoderReturnsErrNotFound(mfxImplType implType);
void Dispatcher_CreateSession_RequestTwoSupportedDecodersReturnsErrNone(mfxImplType implType);
void Dispatcher_CreateSession_RequestMixedDecodersReturnsErrNotFound(mfxImplType implType);
void Dispatcher_CreateSession_RequestAccelValidCreatesSession(mfxImplType implType);
void Dispatcher_CreateSession_RequestAccelInvalidReturnsNotFound(mfxImplType implType);
void Dispatcher_CreateSession_RequestCurrentAPIVersionCreatesSession(mfxImplType implType);
void Dispatcher_CreateSession_RequestLowerAPIVersionCreatesSession(mfxImplType implType);
void Dispatcher_CreateSession_RequestHigherAPIVersionReturnsNotFound(mfxImplType implType);
void Dispatcher_CreateSession_RequestImplementedFunctionCreatesSession(mfxImplType implType);
void Dispatcher_CreateSession_RequestNotImplementedFunctionReturnsNotFound(mfxImplType implType);
void Dispatcher_CreateSession_RequestCurrentAPIMajorMinorCreatesSession(mfxImplType implType);
void Dispatcher_CreateSession_RequestHigherAPIMajorReturnsNotFound(mfxImplType implType);
void Dispatcher_CreateSession_RequestHigherAPIMinorReturnsNotFound(mfxImplType implType);
void Dispatcher_CreateSession_RequestDeviceIDValidReturnsErrNone(mfxImplType implType);
void Dispatcher_CreateSession_RequestDeviceIDInvalidReturnsErrNotFound(mfxImplType implType);
void Dispatcher_CreateSession_RequestImplNameValidReturnsErrNone(mfxImplType implType);
void Dispatcher_CreateSession_RequestImplNameInvalidReturnsErrNotFound(mfxImplType implType);
void Dispatcher_CreateSession_RequestLicenseValidReturnsErrNone(mfxImplType implType);
void Dispatcher_CreateSession_RequestLicenseInvalidReturnsErrNotFound(mfxImplType implType);
void Dispatcher_CreateSession_RequestLicenseMixedReturnsErrNotFound(mfxImplType implType);
void Dispatcher_CreateSession_RequestKeywordsValidReturnsErrNone(mfxImplType implType);
void Dispatcher_CreateSession_RequestKeywordsMixedReturnsErrNotFound(mfxImplType implType);
void Dispatcher_CreateSession_ConfigHandleReturnsHandle(mfxImplType implType);

void Dispatcher_DispReleaseImplDescription_ValidInputReturnsErrNone(mfxImplType implType);
void Dispatcher_DispReleaseImplDescription_NullLoaderReturnsErrNull(mfxImplType implType);
void Dispatcher_DispReleaseImplDescription_NullDescReturnsErrNull(mfxImplType implType);
void Dispatcher_DispReleaseImplDescription_HandleMismatchReturnsInvalidHandle(mfxImplType implType);
void Dispatcher_DispReleaseImplDescription_ReleaseTwiceReturnsErrNone(mfxImplType implType);

#endif // TEST_UNIT_API_SMART_DISPATCHER_H_
