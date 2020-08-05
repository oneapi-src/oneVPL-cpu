/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>
#include "vpl/mfxdispatcher.h"
#include "vpl/mfximplcaps.h"
#include "vpl/mfxvideo.h"

// smart dispatcher operations
//QueryImplsDescription
TEST(QueryImplsDescription, DISABLED_DeliveryFormatInReturnsHdl) {
    FAIL() << "Test not implemented";
}

TEST(QueryImplsDescription, DISABLED_NullDeliveryFormatInReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(QueryImplsDescription, DISABLED_NullNumImplsInReturnsErrNull) {
    FAIL() << "Test not implemented";
}

//ReleaseImplDescription
TEST(ReleaseImplDescription, DISABLED_InitializedHdlReturnsErrNone) {
    FAIL() << "Test not implemented";
}

TEST(ReleaseImplDescription, DISABLED_UninitializedHdlReturnsErrNull) {
    FAIL() << "Test not implemented";
}

//MFXLoad
TEST(Load, DISABLED_CallReturnsLoader) {
    FAIL() << "Test not implemented";
}

//MFXCreateConfig
TEST(CreateConfig, DISABLED_InitializedLoaderReturnsConfig) {
    FAIL() << "Test not implemented";
}

TEST(CreateConfig, DISABLED_NullLoaderReturnsErrNull) {
    FAIL() << "Test not implemented";
}

//MFXSetConfigFilterProperty
TEST(SetConfigFilterProperty, DISABLED_PropertyNameInReturnsErrNone) {
    FAIL() << "Test not implemented";
}

TEST(SetConfigFilterProperty, DISABLED_NullConfigReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(SetConfigFilterProperty, DISABLED_NullNameReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(SetConfigFilterProptery, DISABLED_UnknownParamReturnsNotFound) {
    FAIL() << "Test not implemented";
}

TEST(SetConfigFilterProptery, DISABLED_ValueTypeMismatchReturnsErrUnsupported) {
    FAIL() << "Test not implemented";
}

//MFXEnumImplementations
TEST(EnumImplementations, DISABLED_ValidInputsReturnErrNone) {
    FAIL() << "Test not implemented";
}

TEST(EnumImplementations, DISABLED_NullLoaderReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(EnumImplementations, DISABLED_NullSessionReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(EnumImplementations, DISABLED_IndexOutOfRangeReturnsNotFound) {
    FAIL() << "Test not implemented";
}

//MFXCreateSession
TEST(CreateSession, DISABLED_ValidInputReturnsErrNone) {
    FAIL() << "Test not implemented";
}

TEST(CreateSession, DISABLED_NullLoaderReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(CreateSession, DISABLED_NullSessionReturnsErrNull) {
    FAIL() << "Test not implemented";
}

//MFXDispReleaseImplDescription
TEST(DispReleaseImplDescription, DISABLED_ValidInputReturnsErrNone) {
    FAIL() << "Test not implemented";
}

TEST(DispReleaseImplDescription, DISABLED_NullLoaderReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(DispReleaseImplDescription, DISABLED_HandleMismatchReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}
