// Copyright (c) 2020 Intel Corporation
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

#ifndef __MFXDISPATCHER_H__
#define __MFXDISPATCHER_H__

#include "mfxdefs.h"

#include "mfxcommon.h"
#include "mfxsession.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! SDK loader handle */
typedef struct _mfxLoader *mfxLoader;

/*! SDK config handle */
typedef struct _mfxConfig *mfxConfig;

/*!
   @brief This function creates the SDK loader.
   @return Loader SDK loader handle or NULL if failed.
*/
mfxLoader MFX_CDECL MFXLoad();

/*!
   @brief This function destroys the SDK dispatcher.
   @param[in] loader SDK loader handle.
*/
void MFX_CDECL MFXUnload(mfxLoader loader);

/*!
   @brief This function creates dispatcher configuration.
   @details This function creates dispatcher internal congfiguration, which is used to filter out avialable implementations.
            Then this config is used to walk through selected implementations to gather more details and select appropriate 
            implementation to load. Loader object remembers all created mfxConfig objects and desrtoyes them during mfxUnload
            function call.

            Multilple configurations per single mfxLoader object is possible.

            Usage example:
            @code
               mfxLoader loader = MFXLoad();
               mfxConfig cfg = MFXCreateConfig(loader);
               MFXCreateSession(loader,0,&session);
            @endcode
   @param[in] loader SDK loader handle.
   @return SDK config handle or NULL pointer is failed.
*/
mfxConfig MFX_CDECL MFXCreateConfig(mfxLoader loader);

/*!
   @brief This function used to add additional filter propery (any fields of mfxImplDescription structure) to the configuration of the SDK loader object.
          One mfxConfig properties can hold only single filter property. 
          @note Each new call with the same parameter "name" will overwrite previously set "value". This may invalidate other properties.
          @note Each new call with another parameter "name" will delete previouse property and create new property based on new "name"'s value. 

          Simple Usage example:
          @code
             mfxLoader loader = MFXLoad();
             mfxConfig cfg = MFXCreateConfig(loader);
             mfxVariant ImplValue;
             ImplValue.Type = MFX_VARIANT_TYPE_U32;
             ImplValue.Data.U32 = MFX_IMPL_SOFTWARE;
             MFXSetConfigFilterProperty(cfg,"mfxImplDescription.Impl",ImplValue);
             MFXCreateSession(loader,0,&session);
          @endcode

          Two sessions usage example (Multiple loaders example):
          @code
             // Create session with software based implementation
             mfxLoader loader1 = MFXLoad();
             mfxConfig cfg1 = MFXCreateConfig(loader1);
             mfxVariant ImplValueSW;
             ImplValueSW.Type = MFX_VARIANT_TYPE_U32;
             ImplValueSW.Data.U32 = MFX_IMPL_SOFTWARE;
             MFXSetConfigFilterProperty(cfg1,"mfxImplDescription.Impl",ImplValueSW);
             MFXCreateSession(loader1,0,&sessionSW);

             // Create session with hardware based implementation
             mfxLoader loader2 = MFXLoad();
             mfxConfig cfg2 = MFXCreateConfig(loader2);
             mfxVariant ImplValueHW;
             ImplValueHW.Type = MFX_VARIANT_TYPE_U32;
             ImplValueHW.Data.U32 = MFX_IMPL_HARDWARE;
             MFXSetConfigFilterProperty(cfg2,"mfxImplDescription.Impl",ImplValueHW);
             MFXCreateSession(loader2,0,&sessionHW);

             // use both sessionSW and sessionHW
             // ...
             // Close everything
             MFXClose(sessionSW);
             MFXClose(sessionHW);
             MFXUnload(loader1); // cfg1 will be destroyed here.
             MFXUnload(loader2); // cfg2 will be destroyed here.
          @endcode

          Two decoders example (Multiple Config objects example):
          @code
             mfxLoader loader = MFXLoad();

             mfxConfig cfg1 = MFXCreateConfig(loader);
             mfxVariant ImplValue;
             val.Type = MFX_VARIANT_TYPE_U32;
             val.Data.U32 = MFX_CODEC_AVC;
             MFXSetConfigFilterProperty(cfg1,"mfxImplDescription.mfxDecoderDescription.decoder.CodecID",ImplValue);

             mfxConfig cfg2 = MFXCreateConfig(loader);
             mfxVariant ImplValue;
             val.Type = MFX_VARIANT_TYPE_U32;
             val.Data.U32 = MFX_CODEC_HEVC;
             MFXSetConfigFilterProperty(cfg2,"mfxImplDescription.mfxDecoderDescription.decoder.CodecID",ImplValue);

             MFXCreateSession(loader,0,&sessionAVC);
             MFXCreateSession(loader,0,&sessionHEVC);
          @endcode
            
   @param[in] config SDK config handle.
   @param[in] name Name of the parameter (see mfxImplDescription structure and example). 
   @param[in] value Value of the parameter.
   @return
      MFX_ERR_NONE The function completed successfully.
      MFX_ERR_NULL_PTR    If config is NULL. \n
      MFX_ERR_NULL_PTR    If name is NULL. \n
      MFX_ERR_NOT_FOUND   If name contains unknown parameter name.
      MFX_ERR_UNSUPPORTED If value data type doesn't equal to the paramer with provided name.
*/
mfxStatus MFX_CDECL MFXSetConfigFilterProperty(mfxConfig config, const mfxU8* name, mfxVariant value);

/*!
   @brief This function used to iterate over filtered out implementations to gather their details. This function allocates memory to store
          mfxImplDescription structure instance. Use MFXReleaseImplDescription function to free memory allocated to the mfxImplDescription structure.
   @param[in] loader SDK loader handle.
   @param[in] i Index of the implementation.
   @param[out] idesc Poiner to the mfxImplDescription structure.
   @return
      MFX_ERR_NONE        The function completed successfully. The idesc contains valid information.\n 
      MFX_ERR_NULL_PTR    If loader is NULL. \n
      MFX_ERR_NULL_PTR    If idesc is NULL. \n
      MFX_ERR_NOT_FOUND   Provided index is out of possible range.
*/
mfxStatus MFX_CDECL MFXEnumImplementations(mfxLoader loader, mfxU32 i, mfxImplDescription* idesc);


/*!
   @brief This function used to load and initialize the implementation.
   @code 
      mfxLoader loader = MFXLoad();
      int i=0;
      while(1) {
         mfxImplDescription *idesc;
         MFXEnumImplementations(loader, i, idesc);
         if(is_good(idesc)) {
             MFXCreateSession(loader, i,&session);
             // ...
             MFXReleaseImplDescription(idesc);
         }
         else
         {
             MFXReleaseImplDescription(idesc);
             break;
         }
      }
   @endcode
   @param[in] loader SDK loader handle.
   @param[in] i Index of the implementation.
   @param[out] session pointer to the SDK session handle.
   @return
      MFX_ERR_NONE        The function completed successfully. The session contains pointer to the SDK session handle.\n 
      MFX_ERR_NULL_PTR    If loader is NULL. \n
      MFX_ERR_NULL_PTR    If session is NULL. \n
      MFX_ERR_NOT_FOUND   Provided index is out of possible range.
*/
mfxStatus MFX_CDECL MFXCreateSession(mfxLoader loader, mfxU32 i, mfxSession* session);

#ifdef __cplusplus
}
#endif

#endif

