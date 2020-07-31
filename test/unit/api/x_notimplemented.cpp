/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>
#include "vpl/mfxvideo.h"

// These optional functions for encode, decode, and VPP are not implemented
// in the CPU reference implementation

TEST(GetEncodeStat, AlwaysReturnsNotImplemented) {}

TEST(GetDecodeStat, AlwaysReturnsNotImplemented) {}

TEST(GetVPPStat, AlwaysReturnsNotImplemented) {}

TEST(DecodeSetSkipMode, AlwaysReturnsNotImplemented) {}

TEST(DecodeGetPayload, AlwaysReturnsNotImplemented) {}