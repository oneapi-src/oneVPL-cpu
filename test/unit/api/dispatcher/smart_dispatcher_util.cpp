/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>

#include "./smart_dispatcher.h"
#include "api/unit_api.h"

// implement templatized helper functions for dispatcher tests

// utility functions to fill mfxVariant
template <typename varDataType>
void FillVariant(mfxVariant *var, varDataType data);

#define CREATE_FILL_VARIANT_FN(x_field, x_type, x_varDataType) \
    template <>                                                \
    void FillVariant(mfxVariant *var, x_type data) {           \
        var->Version.Version = (mfxU16)MFX_VARIANT_VERSION;    \
        var->Type            = x_varDataType;                  \
        var->Data.x_field    = data;                           \
    }

// expand into a unique implementation for each type
CREATE_FILL_VARIANT_FN(U8, mfxU8, MFX_VARIANT_TYPE_U8)
CREATE_FILL_VARIANT_FN(U16, mfxU16, MFX_VARIANT_TYPE_U16)
CREATE_FILL_VARIANT_FN(U32, mfxU32, MFX_VARIANT_TYPE_U32)
CREATE_FILL_VARIANT_FN(U64, mfxU64, MFX_VARIANT_TYPE_U64)
CREATE_FILL_VARIANT_FN(I8, mfxI8, MFX_VARIANT_TYPE_I8)
CREATE_FILL_VARIANT_FN(I16, mfxI16, MFX_VARIANT_TYPE_I16)
CREATE_FILL_VARIANT_FN(I32, mfxI32, MFX_VARIANT_TYPE_I32)
CREATE_FILL_VARIANT_FN(I64, mfxI64, MFX_VARIANT_TYPE_I64)
CREATE_FILL_VARIANT_FN(F32, mfxF32, MFX_VARIANT_TYPE_F32)
CREATE_FILL_VARIANT_FN(F64, mfxF64, MFX_VARIANT_TYPE_F64)
CREATE_FILL_VARIANT_FN(Ptr, mfxHDL, MFX_VARIANT_TYPE_PTR)

template <typename varDataType>
mfxStatus SetConfigFilterProperty(mfxLoader loader, const char *name, varDataType data) {
    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    mfxVariant var;
    FillVariant(&var, data);

    mfxStatus sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)name, var);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    return sts;
}

template mfxStatus SetConfigFilterProperty<mfxU8>(mfxLoader, const char *, mfxU8);
template mfxStatus SetConfigFilterProperty<mfxU16>(mfxLoader, const char *, mfxU16);
template mfxStatus SetConfigFilterProperty<mfxU32>(mfxLoader, const char *, mfxU32);
template mfxStatus SetConfigFilterProperty<mfxU64>(mfxLoader, const char *, mfxU64);
template mfxStatus SetConfigFilterProperty<mfxI8>(mfxLoader, const char *, mfxI8);
template mfxStatus SetConfigFilterProperty<mfxI16>(mfxLoader, const char *, mfxI16);
template mfxStatus SetConfigFilterProperty<mfxI32>(mfxLoader, const char *, mfxI32);
template mfxStatus SetConfigFilterProperty<mfxI64>(mfxLoader, const char *, mfxI64);
template mfxStatus SetConfigFilterProperty<mfxF32>(mfxLoader, const char *, mfxF32);
template mfxStatus SetConfigFilterProperty<mfxF64>(mfxLoader, const char *, mfxF64);
template mfxStatus SetConfigFilterProperty<mfxHDL>(mfxLoader, const char *, mfxHDL);

// in this version we use the mfxConfig obj which is passed in vs. creating a new one
template <typename varDataType>
mfxStatus SetConfigFilterProperty(mfxLoader loader, mfxConfig cfg, const char *name, varDataType data) {
    mfxVariant var;
    FillVariant(&var, data);

    mfxStatus sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)name, var);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    return sts;
}

template mfxStatus SetConfigFilterProperty<mfxU8>(mfxLoader, mfxConfig, const char *, mfxU8);
template mfxStatus SetConfigFilterProperty<mfxU16>(mfxLoader, mfxConfig, const char *, mfxU16);
template mfxStatus SetConfigFilterProperty<mfxU32>(mfxLoader, mfxConfig, const char *, mfxU32);
template mfxStatus SetConfigFilterProperty<mfxU64>(mfxLoader, mfxConfig, const char *, mfxU64);
template mfxStatus SetConfigFilterProperty<mfxI8>(mfxLoader, mfxConfig, const char *, mfxI8);
template mfxStatus SetConfigFilterProperty<mfxI16>(mfxLoader, mfxConfig, const char *, mfxI16);
template mfxStatus SetConfigFilterProperty<mfxI32>(mfxLoader, mfxConfig, const char *, mfxI32);
template mfxStatus SetConfigFilterProperty<mfxI64>(mfxLoader, mfxConfig, const char *, mfxI64);
template mfxStatus SetConfigFilterProperty<mfxF32>(mfxLoader, mfxConfig, const char *, mfxF32);
template mfxStatus SetConfigFilterProperty<mfxF64>(mfxLoader, mfxConfig, const char *, mfxF64);
template mfxStatus SetConfigFilterProperty<mfxHDL>(mfxLoader, mfxConfig, const char *, mfxHDL);

// set any int property - passed as U64 to hold any size
template <>
mfxStatus SetSingleProperty(mfxLoader loader, const char *name, mfxVariantType varType, mfxU64 data) {
    switch (varType) {
        case MFX_VARIANT_TYPE_U8:
            return SetConfigFilterProperty(loader, name, (mfxU8)data);
        case MFX_VARIANT_TYPE_U16:
            return SetConfigFilterProperty(loader, name, (mfxU16)data);
        case MFX_VARIANT_TYPE_U32:
            return SetConfigFilterProperty(loader, name, (mfxU32)data);
        case MFX_VARIANT_TYPE_U64:
            return SetConfigFilterProperty(loader, name, (mfxU64)data);
        case MFX_VARIANT_TYPE_I8:
            return SetConfigFilterProperty(loader, name, (mfxI8)data);
        case MFX_VARIANT_TYPE_I16:
            return SetConfigFilterProperty(loader, name, (mfxI16)data);
        case MFX_VARIANT_TYPE_I32:
            return SetConfigFilterProperty(loader, name, (mfxI32)data);
        case MFX_VARIANT_TYPE_I64:
            return SetConfigFilterProperty(loader, name, (mfxI64)data);

        case MFX_VARIANT_TYPE_F32:
        case MFX_VARIANT_TYPE_F64:
        case MFX_VARIANT_TYPE_PTR:
        case MFX_VARIANT_TYPE_UNSET:
            return MFX_ERR_UNSUPPORTED;
    }

    return MFX_ERR_UNSUPPORTED;
}

// set ptr or float/double properties (should be cast to a pointer)
template <>
mfxStatus SetSingleProperty(mfxLoader loader, const char *name, mfxVariantType varType, mfxHDL data) {
    mfxF32 fData32 = 0;
    mfxF64 fData64 = 0;
    switch (varType) {
        case MFX_VARIANT_TYPE_F32:
            fData32 = *(reinterpret_cast<mfxF32 *>(data));
            return SetConfigFilterProperty(loader, name, fData32);
        case MFX_VARIANT_TYPE_F64:
            fData64 = *(reinterpret_cast<mfxF64 *>(data));
            return SetConfigFilterProperty(loader, name, fData64);
        case MFX_VARIANT_TYPE_PTR:
            return SetConfigFilterProperty(loader, name, (mfxHDL)data);

        case MFX_VARIANT_TYPE_U8:
        case MFX_VARIANT_TYPE_U16:
        case MFX_VARIANT_TYPE_U32:
        case MFX_VARIANT_TYPE_U64:
        case MFX_VARIANT_TYPE_I8:
        case MFX_VARIANT_TYPE_I16:
        case MFX_VARIANT_TYPE_I32:
        case MFX_VARIANT_TYPE_I64:
        case MFX_VARIANT_TYPE_UNSET:
            return MFX_ERR_UNSUPPORTED;
    }

    return MFX_ERR_UNSUPPORTED;
}

// set any int property - passed as U64 to hold any size
template <>
mfxStatus SetSingleProperty(mfxLoader loader, mfxConfig cfg, const char *name, mfxVariantType varType, mfxU64 data) {
    switch (varType) {
        case MFX_VARIANT_TYPE_U8:
            return SetConfigFilterProperty(loader, cfg, name, (mfxU8)data);
        case MFX_VARIANT_TYPE_U16:
            return SetConfigFilterProperty(loader, cfg, name, (mfxU16)data);
        case MFX_VARIANT_TYPE_U32:
            return SetConfigFilterProperty(loader, cfg, name, (mfxU32)data);
        case MFX_VARIANT_TYPE_U64:
            return SetConfigFilterProperty(loader, cfg, name, (mfxU64)data);
        case MFX_VARIANT_TYPE_I8:
            return SetConfigFilterProperty(loader, cfg, name, (mfxI8)data);
        case MFX_VARIANT_TYPE_I16:
            return SetConfigFilterProperty(loader, cfg, name, (mfxI16)data);
        case MFX_VARIANT_TYPE_I32:
            return SetConfigFilterProperty(loader, cfg, name, (mfxI32)data);
        case MFX_VARIANT_TYPE_I64:
            return SetConfigFilterProperty(loader, cfg, name, (mfxI64)data);

        case MFX_VARIANT_TYPE_F32:
        case MFX_VARIANT_TYPE_F64:
        case MFX_VARIANT_TYPE_PTR:
        case MFX_VARIANT_TYPE_UNSET:
            return MFX_ERR_UNSUPPORTED;
    }

    return MFX_ERR_UNSUPPORTED;
}

// set ptr or float/double properties (should be cast to a pointer)
template <>
mfxStatus SetSingleProperty(mfxLoader loader, mfxConfig cfg, const char *name, mfxVariantType varType, mfxHDL data) {
    mfxF32 fData32 = 0;
    mfxF64 fData64 = 0;
    switch (varType) {
        case MFX_VARIANT_TYPE_F32:
            fData32 = *(reinterpret_cast<mfxF32 *>(data));
            return SetConfigFilterProperty(loader, cfg, name, fData32);
        case MFX_VARIANT_TYPE_F64:
            fData64 = *(reinterpret_cast<mfxF64 *>(data));
            return SetConfigFilterProperty(loader, cfg, name, fData64);
        case MFX_VARIANT_TYPE_PTR:
            return SetConfigFilterProperty(loader, cfg, name, (mfxHDL)data);

        case MFX_VARIANT_TYPE_U8:
        case MFX_VARIANT_TYPE_U16:
        case MFX_VARIANT_TYPE_U32:
        case MFX_VARIANT_TYPE_U64:
        case MFX_VARIANT_TYPE_I8:
        case MFX_VARIANT_TYPE_I16:
        case MFX_VARIANT_TYPE_I32:
        case MFX_VARIANT_TYPE_I64:
        case MFX_VARIANT_TYPE_UNSET:
            return MFX_ERR_UNSUPPORTED;
    }

    return MFX_ERR_UNSUPPORTED;
}

template mfxStatus SetSingleProperty<mfxU64>(mfxLoader, const char *, mfxVariantType, mfxU64);
template mfxStatus SetSingleProperty<mfxHDL>(mfxLoader, const char *, mfxVariantType, mfxHDL);

template mfxStatus SetSingleProperty<mfxU64>(mfxLoader, mfxConfig, const char *, mfxVariantType, mfxU64);
template mfxStatus SetSingleProperty<mfxHDL>(mfxLoader, mfxConfig, const char *, mfxVariantType, mfxHDL);
