/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

/// @file
///
/// Version API implementation for vpl.
///

#define VPL_ENABLE_EXPORTS

#include "vpl/version.hpp"
#define V1 VPL_VERSION_MAJOR
#define V2 VPL_VERSION_MINOR
#define V3 VPL_VERSION_PATCH
namespace vpl {

static constexpr vpl_library_version_t version = { V1, V2, V3 };

} // namespace vpl
extern "C" {

VPL_API vpl_library_version_t const* vpl_get_library_version() {
    return &vpl::version;
}
}
