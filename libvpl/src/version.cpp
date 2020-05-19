/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

///
/// Version API implementation for vpl.
///
/// @file

/// Export symbols
#define VPL_ENABLE_EXPORTS

#include "vpl/version.hpp"
namespace vpl {

static constexpr vpl_library_version_t version = { VPL_VERSION_MAJOR,
                                                   VPL_VERSION_MINOR,
                                                   VPL_VERSION_PATCH };

} // namespace vpl
extern "C" {

VPL_API vpl_library_version_t const* vpl_get_library_version() {
    return &vpl::version;
}
}
