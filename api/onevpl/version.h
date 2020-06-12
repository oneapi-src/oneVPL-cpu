/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

///
/// C version API for the oneAPI Video Processing Library (oneVPL).
///
/// @file

#ifndef API_ONEVPL_VERSION_H_
#define API_ONEVPL_VERSION_H_

#include <stdint.h>

#include "onevpl/export.h"

/// The library version
typedef struct vpl_library_version {
    uint32_t major; ///< Major version.
    uint32_t minor; ///< Minor version.
    uint32_t patch; ///< Patch version.
} vpl_library_version_t;

#ifdef __cplusplus
extern "C" {
#endif

/// Computes stuff.
VPL_API vpl_library_version_t const* vpl_get_library_version();

#ifdef __cplusplus
}
#endif

#endif // API_ONEVPL_VERSION_H_
