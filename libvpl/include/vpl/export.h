/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

/// @file
///
/// Shared library export macros.
///

#ifndef LIBVPL_INCLUDE_VPL_EXPORT_H_
#define LIBVPL_INCLUDE_VPL_EXPORT_H_

#ifdef VPL_STATIC_DEFINE
    #define VPL_API
    #define VPL_NO_EXPORT
#else
    #if defined(_WIN32)
        #ifdef VPL_ENABLE_EXPORTS
            /* We are building this library */
            #define VPL_API __declspec(dllexport)
        #else
            /* We are using this library */
            #define VPL_API __declspec(dllimport)
        #endif
    #else
        #if __GNUC__ >= 4
            #define VPL_API       __attribute__((visibility("default")))
            #define VPL_NO_EXPORT __attribute__((visibility("hidden")))
        #else
            #define VPL_API
            #define VPL_NO_EXPORT
        #endif
    #endif
#endif

#endif // LIBVPL_INCLUDE_VPL_EXPORT_H_
