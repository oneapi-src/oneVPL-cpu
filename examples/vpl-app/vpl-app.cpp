/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

///
/// Example of a minimal oneAPI Video Processing Library (oneVPL) application.
///
/// @file

#include <iostream>

#include "vpl/mfxvideo++.h"

/// Main entry point.
int main(int argc, char* argv[]) {
    mfxStatus sts = MFX_ERR_NONE;
    //    mfxIMPL impl       = MFX_IMPL_AUTO_ANY;
    mfxIMPL impl       = MFX_IMPL_SOFTWARE_VPL;
    mfxVersion version = { { 0, 1 } };
    MFXVideoSession session;

    sts = session.Init(impl, &version);
    if (sts > MFX_ERR_NONE) {
        std::cerr << "vpl-app: Init() failed with code " << sts << " \n";
        return sts;
    }

    sts = session.QueryIMPL(&impl);
    if (sts > MFX_ERR_NONE) {
        std::cerr << "vpl-app: QueryIMPL() failed with code " << sts << " \n";
        return sts;
    }

    sts = session.QueryVersion(&version);
    if (sts > MFX_ERR_NONE) {
        std::cerr << "vpl-app: QueryVersion() failed with code " << sts
                  << " \n";
        return sts;
    }

    std::cout << "oneVPL " << version.Major << "." << version.Minor << " ("
              << ((impl == MFX_IMPL_SOFTWARE_VPL) ? "VPL-CPU" : "HARDWARE")
              << ")\n";

    session.Close();

    return 0;
}
