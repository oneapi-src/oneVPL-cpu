/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "windows/mfx_dispatcher_onevpl.h"

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

mfxConfig MFXCreateConfig(mfxLoader loader) {
    if (!loader)
        return nullptr;

    // create config context
    // each loader may have more than one config
    LoaderCtxOneVPL* loaderCtx = (LoaderCtxOneVPL*)loader;

    std::unique_ptr<ConfigCtxOneVPL> configCtx;
    try {
        configCtx.reset(new ConfigCtxOneVPL{});
    }
    catch (...) {
        return nullptr;
    }

    ConfigCtxOneVPL* config = loaderCtx->AddConfigFilter();

    return (mfxConfig)(config);
}

mfxStatus MFXSetConfigFilterProperty(mfxConfig config,
                                     const mfxU8* name,
                                     mfxVariant value) {
    if (!config)
        return MFX_ERR_NULL_PTR;

    ConfigCtxOneVPL* configCtx = (ConfigCtxOneVPL*)config;

    configCtx->SetFilterProperty(name, value);

    return MFX_ERR_NONE;
}

mfxStatus MFXEnumImplementations(mfxLoader loader,
                                 mfxU32 i,
                                 mfxImplCapsDeliveryFormat format,
                                 mfxHDL* idesc) {
    if (!loader)
        return MFX_ERR_NULL_PTR;

    LoaderCtxOneVPL* loaderCtx = (LoaderCtxOneVPL*)loader;

    mfxStatus sts = loaderCtx->QueryImpl(i, format, idesc);

    return sts;
}

mfxStatus MFXCreateSession(mfxLoader loader, mfxU32 i, mfxSession* session) {
    if (!loader)
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

// passthrough functions to implementation
mfxStatus MFXMemory_GetSurfaceForVPP(mfxSession session,
                                     mfxFrameSurface1** surface) {
    mfxStatus mfxRes         = MFX_ERR_INVALID_HANDLE;
    MFX_DISP_HANDLE* pHandle = (MFX_DISP_HANDLE*)session;

    if (pHandle) {
        mfxFunctionPointer pFunc;
        pFunc = pHandle->callVideoTable2[eMFXMemory_GetSurfaceForVPP];
        if (pFunc) {
            session = pHandle->session;
            mfxRes =
                (*(mfxStatus(MFX_CDECL*)(mfxSession, mfxFrameSurface1**))pFunc)(
                    session,
                    surface);
        }
    }

    return mfxRes;
}

mfxStatus MFXMemory_GetSurfaceForEncode(mfxSession session,
                                        mfxFrameSurface1** surface) {
    mfxStatus mfxRes         = MFX_ERR_INVALID_HANDLE;
    MFX_DISP_HANDLE* pHandle = (MFX_DISP_HANDLE*)session;

    if (pHandle) {
        mfxFunctionPointer pFunc;
        pFunc = pHandle->callVideoTable2[eMFXMemory_GetSurfaceForEncode];
        if (pFunc) {
            session = pHandle->session;
            mfxRes =
                (*(mfxStatus(MFX_CDECL*)(mfxSession, mfxFrameSurface1**))pFunc)(
                    session,
                    surface);
        }
    }

    return mfxRes;
}

mfxStatus MFXMemory_GetSurfaceForDecode(mfxSession session,
                                        mfxFrameSurface1** surface) {
    mfxStatus mfxRes         = MFX_ERR_INVALID_HANDLE;
    MFX_DISP_HANDLE* pHandle = (MFX_DISP_HANDLE*)session;

    if (pHandle) {
        mfxFunctionPointer pFunc;
        pFunc = pHandle->callVideoTable2[eMFXMemory_GetSurfaceForDecode];
        if (pFunc) {
            session = pHandle->session;
            mfxRes =
                (*(mfxStatus(MFX_CDECL*)(mfxSession, mfxFrameSurface1**))pFunc)(
                    session,
                    surface);
        }
    }

    return mfxRes;
}
