/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

///
/// C++ version API for the oneAPI Video Processing Library (oneVPL).
///
/// @file

#ifndef API_ONEVPL_VERSION_HPP_
#define API_ONEVPL_VERSION_HPP_

#include <cstdint>

#include "onevpl/version.h"

namespace vpl {
/// The library version
typedef struct vpl_library_version library_version_t;

/// Get the library version
library_version_t const& get_library_version() {
    return *vpl_get_library_version();
}

} // namespace vpl

#endif // API_ONEVPL_VERSION_HPP_
