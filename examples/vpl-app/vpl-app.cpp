/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

///
/// Example of a minimal oneAPI Video Processing Library (oneVPL) application.
///
/// @file

#include <cstdio>

#include "vpl/mfxvideo.h"

/// Main entry point.
int main(int argc, char* argv[]) {
    mfxStatus sts      = MFX_ERR_NONE;
    mfxIMPL impl       = MFX_IMPL_SOFTWARE;
    mfxVersion version = { { 0, 2 } };
    mfxSession session = { 0 };

    sts = MFXInit(impl, &version, &session);
    if (sts != MFX_ERR_NONE) {
        fprintf(stderr, "MFXInit failed with code %d\n", sts);
        return sts;
    }

    sts = MFXQueryIMPL(session, &impl);
    if (sts != MFX_ERR_NONE) {
        fprintf(stderr, "MFXQueryIMPL failed with code %d\n", sts);
        return sts;
    }

    sts = MFXQueryVersion(session, &version);
    if (sts != MFX_ERR_NONE) {
        fprintf(stderr, "MFXQueryVersion failed with code %d\n", sts);
        return sts;
    }

    printf("%d.%d (%s)\n",
           version.Major,
           version.Minor,
           (impl == MFX_IMPL_SOFTWARE) ? "VPL-CPU" : "HARDWARE");

    MFXClose(session);

    return 0;
}
