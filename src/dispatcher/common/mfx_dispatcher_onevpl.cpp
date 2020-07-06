/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "common/mfx_dispatcher_onevpl.h"

// exported functions for API >= 2.0

mfxLoader MFXLoad() {
    LoaderCtxOneVPL* loaderCtx;

    // create unique loader context
    try {
        std::unique_ptr<LoaderCtxOneVPL> pLoaderCtx;
        pLoaderCtx.reset(new LoaderCtxOneVPL{});
        loaderCtx = (LoaderCtxOneVPL*)pLoaderCtx.release();
    }
    catch (...) {
        return nullptr;
    }

    // search directories for candidate oneVPL implementations
    //   based on search order in spec
    mfxStatus sts = loaderCtx->BuildListOfCandidateLibs();
    if (MFX_ERR_NONE != sts) {
        return nullptr;
    }

    // prune libraries which are not oneVPL implementations
    // fill function ptr table for each library which is
    mfxU32 numLibs = loaderCtx->CheckValidLibraries();
    if (numLibs == 0) {
        return nullptr;
    }

    return (mfxLoader)loaderCtx;
}

void MFXUnload(mfxLoader loader) {
    // destroy all created mfxConfig objects, other memory
    if (loader) {
        LoaderCtxOneVPL* loaderCtx = (LoaderCtxOneVPL*)loader;

        loaderCtx->UnloadAllLibraries();

        loaderCtx->FreeConfigFilters();

        delete loaderCtx;
    }

    return;
}

// create config context
// each loader may have more than one config
mfxConfig MFXCreateConfig(mfxLoader loader) {
    if (!loader)
        return nullptr;

    LoaderCtxOneVPL* loaderCtx = (LoaderCtxOneVPL*)loader;
    ConfigCtxOneVPL* configCtx;

    try {
        std::unique_ptr<ConfigCtxOneVPL> pConfigCtx;
        pConfigCtx.reset(new ConfigCtxOneVPL{});
        configCtx = loaderCtx->AddConfigFilter();
    }
    catch (...) {
        return nullptr;
    }

    return (mfxConfig)(configCtx);
}

mfxStatus MFXSetConfigFilterProperty(mfxConfig config,
                                     const mfxU8* name,
                                     mfxVariant value) {
    if (!config)
        return MFX_ERR_NULL_PTR;

    ConfigCtxOneVPL* configCtx = (ConfigCtxOneVPL*)config;

    mfxStatus sts = configCtx->SetFilterProperty(name, value);

    return sts;
}

mfxStatus MFXEnumImplementations(mfxLoader loader,
                                 mfxU32 i,
                                 mfxImplCapsDeliveryFormat format,
                                 mfxHDL* idesc) {
    if (!loader || !idesc)
        return MFX_ERR_NULL_PTR;

    LoaderCtxOneVPL* loaderCtx = (LoaderCtxOneVPL*)loader;

    mfxStatus sts = loaderCtx->QueryImpl(i, format, idesc);

    return sts;
}

mfxStatus MFXCreateSession(mfxLoader loader, mfxU32 i, mfxSession* session) {
    if (!loader || !session)
        return MFX_ERR_NULL_PTR;

    LoaderCtxOneVPL* loaderCtx = (LoaderCtxOneVPL*)loader;

    mfxStatus sts = loaderCtx->CreateSession(i, session);

    return sts;
}

mfxStatus MFXDispReleaseImplDescription(mfxLoader loader, mfxHDL hdl) {
    if (!loader)
        return MFX_ERR_NULL_PTR;

    LoaderCtxOneVPL* loaderCtx = (LoaderCtxOneVPL*)loader;

    mfxStatus sts = loaderCtx->ReleaseImpl(hdl);

    return sts;
}
