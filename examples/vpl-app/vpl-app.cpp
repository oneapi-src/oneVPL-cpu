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

#include "vpl/vpl.hpp"

/// Main entry point.
int main(int argc, char* argv[]) {
    try {
        auto version = vpl::get_library_version();
        std::cout << "oneVPL " << version.major << "." << version.minor << "."
                  << version.patch << "\n";
    }
    catch (const std::exception& e) {
        std::cerr << "vpl-app: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
