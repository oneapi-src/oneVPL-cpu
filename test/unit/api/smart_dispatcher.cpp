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
TEST(QueryImplsDescription, DeliveryFormatInReturnsHdl) {}

TEST(QueryImplsDescription, NullDeliveryFormatInReturnsErrNull) {}

TEST(QueryImplsDescription, NullNumImplsInReturnsErrNull) {}

//ReleaseImplDescription
TEST(ReleaseImplDescription, InitializedHdlReturnsErrNone) {}

TEST(ReleaseImplDescription, UninitializedHdlReturnsErrNull) {}

//MFXLoad
TEST(Load, CallReturnsLoader) {}

//MFXCreateConfig
TEST(CreateConfig, InitializedLoaderReturnsConfig) {}

TEST(CreateConfig, NullLoaderReturnsErrNull) {}

//MFXSetConfigFilterProperty
TEST(SetConfigFilterProperty, PropertyNameInReturnsErrNone) {}

TEST(SetConfigFilterProperty, NullConfigReturnsErrNull) {}

TEST(SetConfigFilterProperty, NullNameReturnsErrNull) {}

TEST(SetConfigFilterProptery, UnknownParamReturnsNotFound) {}

TEST(SetConfigFilterProptery, ValueTypeMismatchReturnsErrUnsupported) {}

//MFXEnumImplementations
TEST(EnumImplementations, ValidInputsReturnErrNone) {}

TEST(EnumImplementations, NullLoaderReturnsErrNull) {}

TEST(EnumImplementations, NullSessionReturnsErrNull) {}

TEST(EnumImplementations, IndexOutOfRangeReturnsNotFound) {}

//MFXCreateSession
TEST(CreateSession, ValidInputReturnsErrNone) {}

TEST(CreateSession, NullLoaderReturnsErrNull) {}

TEST(CreateSession, NullSessionReturnsErrNull) {}

//MFXDispReleaseImplDescription
TEST(DispReleaseImplDescription, ValidInputReturnsErrNone) {}

TEST(DispReleaseImplDescription, NullLoaderReturnsErrNull) {}

TEST(DispReleaseImplDescription, HandleMismatchReturnsInvalidHandle) {}