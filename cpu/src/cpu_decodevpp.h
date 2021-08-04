/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef CPU_SRC_CPU_DECODEVPP_H_
#define CPU_SRC_CPU_DECODEVPP_H_

#include <memory>
#include <vector>
#include "src/cpu_common.h"
#include "src/cpu_decode.h"
#include "src/cpu_frame_pool.h"
#include "src/cpu_vpp.h"

class CpuWorkstream;

class mfxRefCountableBase {
public:
    mfxRefCountableBase() : m_ref_count(0) {}

    virtual ~mfxRefCountableBase() {
        if (m_ref_count.load(std::memory_order_relaxed) != 0) {
            std::ignore = MFX_ERR_UNKNOWN;
        }
    }

    mfxU32 GetRefCounter() const {
        return m_ref_count.load(std::memory_order_relaxed);
    }

    void AddRef() {
        std::ignore = m_ref_count.fetch_add(1, std::memory_order_relaxed);
    }

    mfxStatus Release() {
        RET_IF_FALSE(m_ref_count.load(std::memory_order_relaxed), MFX_ERR_UNDEFINED_BEHAVIOR);

        // fetch_sub return value immediately preceding
        if (m_ref_count.fetch_sub(1, std::memory_order_relaxed) - 1 == 0) {
            Close();
        }

        return MFX_ERR_NONE;
    }

protected:
    virtual void Close() {
        return;
    };

private:
    std::atomic<uint32_t> m_ref_count;
};

class mfxSurfaceArrayImpl : public mfxSurfaceArray, public mfxRefCountableBase {
public:
    virtual ~mfxSurfaceArrayImpl() {}

    static mfxSurfaceArrayImpl *CreateSurfaceArray() {
        mfxSurfaceArrayImpl *surfArr = new mfxSurfaceArrayImpl();
        ((mfxSurfaceArray *)surfArr)->AddRef(surfArr);
        return surfArr;
    }

    static mfxStatus AddRef_impl(mfxSurfaceArray *surfArray) {
        RET_IF_FALSE(surfArray, MFX_ERR_NULL_PTR);
        RET_IF_FALSE(surfArray->Context, MFX_ERR_INVALID_HANDLE);

        reinterpret_cast<mfxRefCountableBase *>(surfArray->Context)->AddRef();

        return MFX_ERR_NONE;
    }

    static mfxStatus Release_impl(mfxSurfaceArray *surfArray) {
        RET_IF_FALSE(surfArray, MFX_ERR_NULL_PTR);
        RET_IF_FALSE(surfArray->Context, MFX_ERR_INVALID_HANDLE);

        return reinterpret_cast<mfxRefCountableBase *>(surfArray->Context)->Release();
    }

    static mfxStatus GetRefCounter_impl(mfxSurfaceArray *surfArray, mfxU32 *counter) {
        RET_IF_FALSE(surfArray, MFX_ERR_NULL_PTR);
        RET_IF_FALSE(counter, MFX_ERR_NULL_PTR);
        RET_IF_FALSE(surfArray->Context, MFX_ERR_INVALID_HANDLE);

        *counter = reinterpret_cast<mfxRefCountableBase *>(surfArray->Context)->GetRefCounter();

        return MFX_ERR_NONE;
    }

    void AddSurface(mfxFrameSurface1 *surface) {
        std::lock_guard<std::mutex> guard(m_mutex);

        m_surfaces.push_back(surface);

        Surfaces    = m_surfaces.data();
        NumSurfaces = (mfxU32)m_surfaces.size();
    }

protected:
    void Close() override {
        delete this;
    }

private:
    mfxSurfaceArrayImpl() : mfxSurfaceArray(), mfxRefCountableBase() {
        Context         = static_cast<mfxRefCountableBase *>(this);
        Version.Version = MFX_SURFACEARRAY_VERSION;

        mfxSurfaceArray::AddRef        = &mfxSurfaceArrayImpl::AddRef_impl;
        mfxSurfaceArray::Release       = &mfxSurfaceArrayImpl::Release_impl;
        mfxSurfaceArray::GetRefCounter = &mfxSurfaceArrayImpl::GetRefCounter_impl;
    }

    std::vector<mfxFrameSurface1 *> m_surfaces;
    std::mutex m_mutex;
};

class RAIISurfaceArray {
public:
    RAIISurfaceArray() {
        SurfArray = mfxSurfaceArrayImpl::CreateSurfaceArray();
    };

    RAIISurfaceArray(const RAIISurfaceArray &) = delete;
    RAIISurfaceArray &operator=(const RAIISurfaceArray &) = delete;
    RAIISurfaceArray(RAIISurfaceArray &&)                 = default;
    RAIISurfaceArray &operator=(RAIISurfaceArray &&) = default;

    ~RAIISurfaceArray() {
        if (SurfArray) {
            for (mfxU32 i = 0; i < SurfArray->NumSurfaces; i++) {
                mfxFrameSurface1 *s = SurfArray->Surfaces[i];
                std::ignore         = s->FrameInterface->Release(s);
            }
            std::ignore = SurfArray->mfxRefCountableBase::Release();
        }
    };

    mfxSurfaceArray *ReleaseContent() {
        mfxSurfaceArray *ret = SurfArray;
        SurfArray            = nullptr;
        return ret;
    };

    mfxSurfaceArrayImpl *operator->() {
        return SurfArray;
    };

protected:
    mfxSurfaceArrayImpl *SurfArray;
};

class CpuDecodeVPP {
public:
    explicit CpuDecodeVPP(CpuWorkstream *session);
    ~CpuDecodeVPP();

    mfxStatus InitDecodeVPP(mfxVideoParam *par,
                            mfxVideoChannelParam **vpp_par_array,
                            mfxU32 num_vpp_par);
    mfxStatus Reset(mfxVideoParam *par, mfxVideoChannelParam **vpp_par_array, mfxU32 num_vpp_par);
    mfxStatus DecodeVPPFrame(mfxBitstream *bs,
                             mfxU32 *skip_channels,
                             mfxU32 num_skip_channels,
                             mfxSurfaceArray **surf_array_out);

    mfxU32 GetVPPChannelCount(void);
    mfxStatus GetChannelParam(mfxVideoChannelParam *par, mfxU32 channel_id);

    mfxStatus CheckVideoParamDecodeVPP(mfxVideoParam *in);
    mfxStatus IsSameVideoParam(mfxVideoParam *newPar, mfxVideoParam *oldPar);

    mfxStatus CheckVideoChannelParamDecodeVPP(mfxVideoChannelParam **inChPar, mfxU32 num_ch);
    mfxStatus IsSameVideoChannelParam(mfxVideoChannelParam *newChPar,
                                      mfxVideoChannelParam *oldChPar);
    mfxStatus Close();

private:
    CpuVPP *m_cpuVPP;
    mfxVideoChannelParam **m_vppChParams;
    mfxFrameSurface1 **m_surfOut;
    mfxU32 m_numVPPCh;
    mfxU32 m_numSurfs;
    CpuWorkstream *m_session;
    mfxSession m_mfxsession;

    mfxStatus InitVPP(mfxVideoParam *par, mfxVideoChannelParam **vpp_par_array, mfxU32 num_vpp_par);

    /* copy not allowed */
    CpuDecodeVPP(const CpuDecodeVPP &);
    CpuDecodeVPP &operator=(const CpuDecodeVPP &);
};

#endif // CPU_SRC_CPU_DECODEVPP_H_
