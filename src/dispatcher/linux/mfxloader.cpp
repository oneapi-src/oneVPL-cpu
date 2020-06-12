// Copyright (c) 2017-2018 Intel Corporation
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <assert.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>
#include <list>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include "onevpl/mfxvideo.h"

#include "linux/mfxloader.h"

namespace MFX {

#if defined(__x86_64__)
    #define LIBMFXSW    "libmfxsw64.so.1"
    #define LIBMFXHW    "libmfxhw64.so.1"
    #define LIBMFXSWVPL "libmfxvplsw64.so"
#else
    #error Unsupported architecture
#endif

#undef FUNCTION
#define FUNCTION(return_value,      \
                 func_name,         \
                 formal_param_list, \
                 actual_param_list) \
    e##func_name,

enum Function {
    eMFXInit,
    eMFXInitEx,
    eMFXClose,
    eMFXJoinSession,
#include "linux/mfxvideo_functions.h"
    eFunctionsNum,
    eNoMoreFunctions = eFunctionsNum
};

struct FunctionsTable {
    Function id;
    const char* name;
    mfxVersion version;
};

#define VERSION(major, minor) \
    {                         \
        { minor, major }      \
    }

#undef FUNCTION
#define FUNCTION(return_value,      \
                 func_name,         \
                 formal_param_list, \
                 actual_param_list) \
    { e##func_name, #func_name, API_VERSION },

static const FunctionsTable g_mfxFuncTable[] = {
    { eMFXInit, "MFXInit", VERSION(1, 0) },
    { eMFXInitEx, "MFXInitEx", VERSION(1, 14) },
    { eMFXClose, "MFXClose", VERSION(1, 0) },
    { eMFXJoinSession, "MFXJoinSession", VERSION(1, 1) },
#include "linux/mfxvideo_functions.h" // NOLINT
    { eNoMoreFunctions }
};

class LoaderCtx {
public:
    mfxStatus Init(mfxInitParam& par);
    mfxStatus Close();

    inline void* getFunction(Function func) const {
        return m_table[func];
    }

    inline mfxSession getSession() const {
        return m_session;
    }

    inline mfxIMPL getImpl() const {
        return m_implementation;
    }

    inline mfxVersion getVersion() const {
        return m_version;
    }

private:
    std::shared_ptr<void> m_dlh;
    mfxVersion m_version{};
    mfxIMPL m_implementation{};
    mfxSession m_session = nullptr;
    void* m_table[eFunctionsNum]{};

    std::mutex m_guard;
};

struct GlobalCtx {
    std::mutex m_mutex;
};

static GlobalCtx g_GlobalCtx;

std::shared_ptr<void> make_dlopen(const char* filename, int flags) {
    return std::shared_ptr<void>(dlopen(filename, flags), [](void* handle) {
        if (handle)
            dlclose(handle);
    });
}

mfxStatus LoaderCtx::Init(mfxInitParam& par) {
    if (par.Implementation & MFX_IMPL_AUDIO) {
        return MFX_ERR_UNSUPPORTED;
    }

    std::vector<std::string> libs;

    if (MFX_IMPL_BASETYPE(par.Implementation) == MFX_IMPL_AUTO ||
        MFX_IMPL_BASETYPE(par.Implementation) == MFX_IMPL_AUTO_ANY) {
        libs.emplace_back(LIBMFXHW);
        libs.emplace_back(LIBMFXSW);
    }
    else if (par.Implementation & MFX_IMPL_HARDWARE ||
             par.Implementation & MFX_IMPL_HARDWARE_ANY) {
        libs.emplace_back(LIBMFXHW);
    }
    else if (par.Implementation & MFX_IMPL_SOFTWARE) {
        libs.emplace_back(LIBMFXSW);
    }
    else if (par.Implementation & MFX_IMPL_SOFTWARE_VPL) {
        libs.emplace_back(LIBMFXSWVPL);
    }
    else {
        return MFX_ERR_UNSUPPORTED;
    }

    mfxStatus mfx_res = MFX_ERR_UNSUPPORTED;

    for (auto& lib : libs) {
        std::shared_ptr<void> hdl =
            make_dlopen(lib.c_str(), RTLD_LOCAL | RTLD_NOW);
        if (hdl) {
            do {
                /* Loading functions table */
                bool wrong_version = false;
                for (int i = 0; i < eFunctionsNum; ++i) {
                    assert(i == g_mfxFuncTable[i].id);
                    m_table[i] = dlsym(hdl.get(), g_mfxFuncTable[i].name);
                    if (!m_table[i] &&
                        ((par.Version <= g_mfxFuncTable[i].version) ||
                         (g_mfxFuncTable[i].version <=
                          mfxVersion(VERSION(1, 14))))) {
                        // this version of dispatcher requires MFXInitEx which appeared
                        // in Media SDK API 1.14
                        wrong_version = true;
                        break;
                    }
                }
                if (wrong_version) {
                    mfx_res = MFX_ERR_UNSUPPORTED;
                    break;
                }

                /* Initializing loaded library */
                mfx_res =
                    ((decltype(MFXInitEx)*)m_table[eMFXInitEx])(par,
                                                                &m_session);
                if (MFX_ERR_NONE != mfx_res) {
                    break;
                }

                // Below we just get some data and double check that we got what we have expected
                // to get. Some of these checks are done inside mediasdk init function
                mfx_res =
                    ((decltype(MFXQueryVersion)*)m_table[eMFXQueryVersion])(
                        m_session,
                        &m_version);
                if (MFX_ERR_NONE != mfx_res) {
                    break;
                }

                if (m_version < par.Version) {
                    mfx_res = MFX_ERR_UNSUPPORTED;
                    break;
                }

                mfx_res = ((decltype(MFXQueryIMPL)*)m_table[eMFXQueryIMPL])(
                    m_session,
                    &m_implementation);
                if (MFX_ERR_NONE != mfx_res) {
                    mfx_res = MFX_ERR_UNSUPPORTED;
                    break;
                }
            } while (false);

            if (MFX_ERR_NONE == mfx_res) {
                m_dlh = std::move(hdl);
                break;
            }
            else {
                Close();
            }
        }
    }

    return mfx_res;
}

mfxStatus LoaderCtx::Close() {
    auto proc         = (decltype(MFXClose)*)m_table[eMFXClose];
    mfxStatus mfx_res = (proc) ? (*proc)(m_session) : MFX_ERR_NONE;

    m_implementation = {};
    m_version        = {};
    m_session        = nullptr;
    std::fill(std::begin(m_table), std::end(m_table), nullptr);
    return mfx_res;
}

} // namespace MFX

#ifdef __cplusplus
extern "C" {
#endif

mfxStatus MFXInit(mfxIMPL impl, mfxVersion* ver, mfxSession* session) {
    mfxInitParam par{};

    par.Implementation = impl;
    if (ver) {
        par.Version = *ver;
    }
    else {
        par.Version = VERSION(MFX_VERSION_MAJOR, MFX_VERSION_MINOR);
    }

    return MFXInitEx(par, session);
}

mfxStatus MFXInitEx(mfxInitParam par, mfxSession* session) {
    if (!session)
        return MFX_ERR_NULL_PTR;

    try {
        std::unique_ptr<MFX::LoaderCtx> loader;

        loader.reset(new MFX::LoaderCtx{});

        mfxStatus mfx_res = loader->Init(par);
        if (MFX_ERR_NONE == mfx_res) {
            *session = (mfxSession)loader.release();
        }
        else {
            *session = nullptr;
        }

        return mfx_res;
    }
    catch (...) {
        return MFX_ERR_MEMORY_ALLOC;
    }
}

mfxStatus MFXClose(mfxSession session) {
    if (!session)
        return MFX_ERR_INVALID_HANDLE;

    try {
        std::unique_ptr<MFX::LoaderCtx> loader((MFX::LoaderCtx*)session);
        mfxStatus mfx_res = loader->Close();

        if (mfx_res == MFX_ERR_UNDEFINED_BEHAVIOR) {
            // It is possible, that there is an active child session.
            // Can't unload library in this case.
            loader.release();
        }
        return mfx_res;
    }
    catch (...) {
        return MFX_ERR_MEMORY_ALLOC;
    }
}

mfxStatus MFXJoinSession(mfxSession session, mfxSession child_session) {
    if (!session || !child_session) {
        return MFX_ERR_INVALID_HANDLE;
    }

    MFX::LoaderCtx* loader       = (MFX::LoaderCtx*)session;
    MFX::LoaderCtx* child_loader = (MFX::LoaderCtx*)child_session;

    if (loader->getVersion().Version != child_loader->getVersion().Version) {
        return MFX_ERR_INVALID_HANDLE;
    }

    auto proc =
        (decltype(MFXJoinSession)*)loader->getFunction(MFX::eMFXJoinSession);
    if (!proc) {
        return MFX_ERR_INVALID_HANDLE;
    }

    return (*proc)(loader->getSession(), child_loader->getSession());
}

mfxStatus MFXCloneSession(mfxSession session, mfxSession* clone) {
    if (!session)
        return MFX_ERR_INVALID_HANDLE;

    MFX::LoaderCtx* loader = (MFX::LoaderCtx*)session;
    // initialize the clone session
    mfxVersion version = loader->getVersion();
    mfxStatus mfx_res  = MFXInit(loader->getImpl(), &version, clone);
    if (MFX_ERR_NONE != mfx_res) {
        return mfx_res;
    }

    // join the sessions
    mfx_res = MFXJoinSession(session, *clone);
    if (MFX_ERR_NONE != mfx_res) {
        MFXClose(*clone);
        *clone = nullptr;
        return mfx_res;
    }

    return MFX_ERR_NONE;
}

#undef FUNCTION
#define FUNCTION(return_value,                                            \
                 func_name,                                               \
                 formal_param_list,                                       \
                 actual_param_list)                                       \
    return_value MFX_CDECL func_name formal_param_list {                  \
        /* get the function's address and make a call */                  \
        if (!session)                                                     \
            return MFX_ERR_INVALID_HANDLE;                                \
                                                                          \
        MFX::LoaderCtx* loader = (MFX::LoaderCtx*)session;                \
                                                                          \
        auto proc =                                                       \
            (decltype(func_name)*)loader->getFunction(MFX::e##func_name); \
        if (!proc)                                                        \
            return MFX_ERR_INVALID_HANDLE;                                \
                                                                          \
        /* get the real session pointer */                                \
        session = loader->getSession();                                   \
        /* pass down the call */                                          \
        return (*proc)actual_param_list;                                  \
    }

#include "linux/mfxvideo_functions.h" // NOLINT

#undef FUNCTION

#ifdef __cplusplus
}
#endif
