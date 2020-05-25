// Copyright (c) 2017 Intel Corporation
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
#ifndef __MFXSESSION_H__
#define __MFXSESSION_H__
#include "mfxcommon.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Global Functions */

/*! SDK session handle */
typedef struct _mfxSession *mfxSession;

/*!
   @fn mfxStatus    MFXInit(mfxIMPL impl, mfxVersion *ver, mfxSession *session)
   @brief
      This function creates and initializes an SDK session. Call this function before calling
      any other SDK functions. If the desired implementation specified by impl is MFX_IMPL_AUTO,
      the function will search for the platform-specific SDK implementation.
      If the function cannot find it, it will use the software implementation.

      The argument ver indicates the desired version of the library implementation.
      The loaded SDK will have an API version compatible to the specified version (equal in
      the major version number, and no less in the minor version number.) If the desired version
      is not specified, the default is to use the API version from the SDK release, with
      which an application is built.

      We recommend that production applications always specify the minimum API version that meets their
      functional requirements. For example, if an application uses only H.264 decoding as described
      in API v1.0, have the application initialize the library with API v1.0. This ensures
      backward compatibility.

   @param[in] impl     mfxIMPL enumerator that indicates the desired SDK implementation.
   @param[in] ver      Pointer to the minimum library version or zero, if not specified.
   @param[out] session Pointer to the SDK session handle.

   @return
      MFX_ERR_NONE        The function completed successfully. The output parameter contains the handle of the session.\n 
      MFX_ERR_UNSUPPORTED The function cannot find the desired SDK implementation or version.
*/
mfxStatus MFX_CDECL MFXInit(mfxIMPL impl, mfxVersion *ver, mfxSession *session);

/*!
   @fn mfxStatus    MFXInitEx(mfxInitParam par, mfxSession *session)
   @brief
      This function creates and initializes an SDK session. Call this function before calling any other SDK functions.
      If the desired implementation specified by par. Implementation is MFX_IMPL_AUTO, the function will search for
      the platform-specific SDK implementation. If the function cannot find it, it will use the software implementation.

      The argument par.Version indicates the desired version of the library implementation. The loaded SDK will have an API
      version compatible to the specified version (equal in the major version number, and no less in the minor version number.)
      If the desired version is not specified, the default is to use the API version from the SDK release, with
      which an application is built.

      We recommend that production applications always specify the minimum API version that meets their functional requirements.
      For example, if an application uses only H.264 decoding as described in API v1.0, have the application initialize
      the library with API v1.0. This ensures backward compatibility.

      The argument par.ExternalThreads specifies threading mode. Value 0 means that SDK should internally create and
      handle work threads (this essentially equivalent of regular MFXInit). If this parameter set to 1 then SDK will expect
      that application should create work threads and pass them to SDK via single-entry function MFXDoWork.

   @param[in]  par     mfxInitParam structure that indicates the desired SDK implementation, minimum library version and desired threading mode.
   @param[out] session Pointer to the SDK session handle.

   @return
      MFX_ERR_NONE        The function completed successfully. The output parameter contains the handle of the session.\n 
      MFX_ERR_UNSUPPORTED The function cannot find the desired SDK implementation or version.
*/
mfxStatus MFX_CDECL MFXInitEx(mfxInitParam par, mfxSession *session);

/*!
   @fn mfxStatus    MFXClose(mfxSession session);
   @brief This function completes and de-initializes an SDK session. Any active tasks in execution or
    in queue are aborted. The application cannot call any SDK function after this function.
    
   All child sessions must be disjoined before closing a parent session.
   @param[in] session SDK session handle.

   @return MFX_ERR_NONE The function completed successfully.
*/
mfxStatus MFX_CDECL MFXClose(mfxSession session);

/*!
   @fn mfxStatus    MFXQueryIMPL(mfxSession session, mfxIMPL *impl);
   @brief This function returns the implementation type of a given session.
    
   @param[in]  session SDK session handle.
   @param[out] impl    Pointer to the implementation type

   @return MFX_ERR_NONE The function completed successfully.
*/
mfxStatus MFX_CDECL MFXQueryIMPL(mfxSession session, mfxIMPL *impl);

/*!
   @fn mfxStatus    MFXQueryVersion(mfxSession session, mfxVersion *version);
   @brief This function returns the SDK implementation version.
    
   @param[in]  session SDK session handle.
   @param[out] version Pointer to the returned implementation version.

   @return MFX_ERR_NONE The function completed successfully.
*/
mfxStatus MFX_CDECL MFXQueryVersion(mfxSession session, mfxVersion *version);

/*!
   @fn mfxStatus    MFXJoinSession(mfxSession session, mfxSession child);
   @brief This function joins the child session to the current session.

   After joining, the two sessions share thread and resource scheduling for asynchronous
   operations. However, each session still maintains its own device manager and buffer/frame
   allocator. Therefore, the application must use a compatible device manager and buffer/frame
   allocator to share data between two joined sessions.

   The application can join multiple sessions by calling this function multiple times. When joining
   the first two sessions, the current session becomes the parent responsible for thread and
   resource scheduling of any later joined sessions.

   Joining of two parent sessions is not supported.
    
   @param[in,out] session    The current session handle.
   @param[in]     child      The child session handle to be joined

   @return MFX_ERR_NONE         The function completed successfully. \n 
           MFX_WRN_IN_EXECUTION Active tasks are executing or in queue in one of the
                                sessions. Call this function again after all tasks are completed. \n
           MFX_ERR_UNSUPPORTED  The child session cannot be joined with the current session.
*/
mfxStatus MFX_CDECL MFXJoinSession(mfxSession session, mfxSession child);

/*!
   @fn mfxStatus    MFXDisjoinSession(mfxSession session);
   @brief This function removes the joined state of the current session. After disjoining, the current
      session becomes independent. The application must ensure there is no active task running
      in the session before calling this function.
    
   @param[in,out] session    The current session handle.

   @return MFX_ERR_NONE                The function completed successfully. \n 
           MFX_WRN_IN_EXECUTION        Active tasks are executing or in queue in one of the
                                       sessions. Call this function again after all tasks are completed. \n
           MFX_ERR_UNDEFINED_BEHAVIOR  The session is independent, or this session is the parent of all joined sessions.
*/
mfxStatus MFX_CDECL MFXDisjoinSession(mfxSession session);
mfxStatus MFX_CDECL MFXCloneSession(mfxSession session, mfxSession *clone);
mfxStatus MFX_CDECL MFXSetPriority(mfxSession session, mfxPriority priority);
mfxStatus MFX_CDECL MFXGetPriority(mfxSession session, mfxPriority *priority);
mfxStatus MFX_CDECL MFXDoWork(mfxSession session);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

