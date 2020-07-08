/*############################################################################
  # Copyright (C) 2013-2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#ifndef DISABLE_NON_VPL_DISPATCHER
    #include "mfx_dispatcher_defs.h"
    #include "mfx_plugin_hive.h"
    #include "mfxplugin.h"

namespace MFX {
typedef mfxStatus(MFX_CDECL *CreatePluginPtr_t)(mfxPluginUID uid,
                                                mfxPlugin *plugin);

class PluginModule {
    mfxModuleHandle mHmodule;
    CreatePluginPtr_t mCreatePluginPtr;
    wchar_t mPath[MAX_PLUGIN_PATH];

public:
    PluginModule();
    explicit PluginModule(const wchar_t *path);
    PluginModule(const PluginModule &that);
    PluginModule &operator=(const PluginModule &that);
    bool Create(mfxPluginUID guid, mfxPlugin &);
    ~PluginModule(void);

private:
    void Tidy();
};

class MFXPluginFactory {
    struct FactoryRecord {
        mfxPluginParam plgParams;
        PluginModule module;
        mfxPlugin plugin;
        FactoryRecord() : plgParams(), plugin() {}
        FactoryRecord(const mfxPluginParam &plgParams,
                      PluginModule &module,
                      mfxPlugin plugin)
                : plgParams(plgParams),
                  module(module),
                  plugin(plugin) {}
    };
    MFXVector<FactoryRecord> mPlugins;
    mfxU32 nPlugins;
    mfxSession mSession;

public:
    explicit MFXPluginFactory(mfxSession session);
    void Close();
    mfxStatus Create(const PluginDescriptionRecord &);
    bool Destroy(const mfxPluginUID &);

    ~MFXPluginFactory();

protected:
    void DestroyPlugin(FactoryRecord &);
    static bool RunVerification(const mfxPlugin &plg,
                                const PluginDescriptionRecord &dsc,
                                mfxPluginParam &pluginParams);
    static bool VerifyEncoder(const mfxVideoCodecPlugin &videoCodec);
    #ifndef DISABLE_NON_VPL_DISPATCHER
    static bool VerifyAudioEncoder(const mfxAudioCodecPlugin &audioCodec);
    static bool VerifyAudioDecoder(const mfxAudioCodecPlugin &audioCodec);
    #endif
    static bool VerifyEnc(const mfxVideoCodecPlugin &videoEnc);
    static bool VerifyVpp(const mfxVideoCodecPlugin &videoCodec);
    static bool VerifyDecoder(const mfxVideoCodecPlugin &videoCodec);
    static bool VerifyCodecCommon(const mfxVideoCodecPlugin &Video);
};
} // namespace MFX

#endif // DISABLE_NON_VPL_DISPATCHER
