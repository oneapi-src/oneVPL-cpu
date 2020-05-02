/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

/// @file
///
/// C version API for the oneAPI Video Processing Library (oneVPL).
///

#ifndef LIBVPL_INCLUDE_VPL_VERSION_H_
#define LIBVPL_INCLUDE_VPL_VERSION_H_

#include <stdint.h>

#include "vpl/export.h"

typedef struct vpl_library_version {
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
} vpl_library_version_t;

#ifdef __cplusplus
extern "C" {
#endif

/// Computes stuff.
VPL_API vpl_library_version_t const* vpl_get_library_version();

#ifdef __cplusplus
}
#endif

#endif // LIBVPL_INCLUDE_VPL_VERSION_H_
