/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>

#include "api/unit_api.h"

// implement templatized helper functions for dispatcher tests

// utility functions to fill mfxVariant
template <typename varDataType>
void FillVariant(mfxVariant *var, varDataType data);

#define CREATE_FILL_VARIANT_FN(x_field, x_type, x_varDataType) \
    template <>                                                \
    void FillVariant(mfxVariant *var, x_type data) {           \
        var->Type         = x_varDataType;                     \
        var->Data.x_field = data;                              \
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
