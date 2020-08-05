/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>
#include "vpl/mfxvideo.h"

// These optional functions for encode, decode, and VPP are not implemented
// in the CPU reference implementation

TEST(GetEncodeStat, DISABLED_AlwaysReturnsNotImplemented) {
    FAIL() << "Test not implemented";
}

TEST(GetDecodeStat, DISABLED_AlwaysReturnsNotImplemented) {
    FAIL() << "Test not implemented";
}

TEST(GetVPPStat, DISABLED_AlwaysReturnsNotImplemented) {
    FAIL() << "Test not implemented";
}

TEST(DecodeSetSkipMode, DISABLED_AlwaysReturnsNotImplemented) {
    FAIL() << "Test not implemented";
}

TEST(DecodeGetPayload, DISABLED_AlwaysReturnsNotImplemented) {
    FAIL() << "Test not implemented";
}
