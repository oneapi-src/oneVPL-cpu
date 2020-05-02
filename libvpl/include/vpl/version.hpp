/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

/// @file
///
/// C++ version API for the oneAPI Video Processing Library (oneVPL).
///

#ifndef LIBVPL_INCLUDE_VPL_VERSION_HPP_
#define LIBVPL_INCLUDE_VPL_VERSION_HPP_

#include <cstdint>

#include "vpl/version.h"

namespace vpl {
typedef struct vpl_library_version library_version_t;

/// Get the library version
library_version_t const& get_library_version() {
    return *vpl_get_library_version();
}

} // namespace vpl

#endif // LIBVPL_INCLUDE_VPL_VERSION_HPP_
