===============================
Asynchronous Pipeline Operation
===============================

The application passes the output of an upstream SDK function to the input of
the downstream SDK function to construct an asynchronous pipeline. Pipeline
construction is done at runtime and can be dynamically changed, as show in the
following example:

.. code-block:: c++

   mfxSyncPoint sp_d, sp_e;
   MFXVideoDECODE_DecodeFrameAsync(session,bs,work,&vin, &sp_d);
   if (going_through_vpp) {
      MFXVideoVPP_RunFrameVPPAsync(session,vin,vout, &sp_d);
      MFXVideoENCODE_EncodeFrameAsync(session,NULL,vout,bits2,&sp_e);
   } else {
      MFXVideoENCODE_EncodeFrameAsync(session,NULL,vin,bits2,&sp_e);
   }
   MFXVideoCORE_SyncOperation(session,sp_e,INFINITE);

The SDK simplifies the requirement for asynchronous pipeline synchronization.
The application only needs to synchronize after the last SDK function. Explicit
synchronization of intermediate results is not required and in fact can slow
performance.

The SDK tracks the dynamic pipeline construction and verifies dependency on
input and output parameters to ensure the execution order of the pipeline function.
In Example 6, the SDK will ensure MFXVideoENCODE_EncodeFrameAsync does not begin
its operation until MFXVideoDECODE_DecodeFrameAsync or MFXVideoVPP_RunFrameVPPAsync
has finished.

During the execution of an asynchronous pipeline, the application must consider
the input data in use and must not change it until the execution has completed.
The application must also consider output data unavailable until the execution
has finished. In addition, for encoders, the application must consider extended
and payload buffers in use while the input surface is locked.

The SDK checks dependencies by comparing the input and output parameters of each
SDK function in the pipeline. Do not modify the contents of input and output
parameters before the previous asynchronous operation finishes. Doing so will
break the dependency check and can result in undefined behavior. An exception
occurs when the input and output parameters are structures, in which case
overwriting fields in the structures is allowed.

.. note:: Note that the dependency check works on the pointers to the structures only.

There are two exceptions with respect to intermediate synchronization:

- The application must synchronize any input before calling the SDK function
  MFXVideoDECODE_DecodeFrameAsync, if the input is from any asynchronous operation.
- When the application calls an asynchronous function to generate an output
  surface and passes that surface to a non-SDK component, it must explicitly
  synchronize the operation before passing the surface to the non-SDK component.

The following pseudo code shows asynchronous **ENC**->**ENCODE** pipeline
construction:

.. code-block:: c++

   mfxENCInput enc_in = ...;
   mfxENCOutput enc_out = ...;
   mfxSyncPoint sp_e, sp_n;
   mfxFrameSurface1* surface = get_frame_to_encode();
   mfxExtBuffer dependency;
   dependency.BufferId = MFX_EXTBUFF_TASK_DEPENDENCY;
   dependency.BufferSz = sizeof(mfxExtBuffer);

   enc_in.InSurface = surface;
   enc_out.ExtParam[enc_out.NumExtParam++] = &dependency;
   MFXVideoENC_ProcessFrameAsync(session, &enc_in, &enc_out, &sp_e);

   surface->Data.ExtParam[surface->Data.NumExtParam++] = &dependency;
   MFXVideoENCODE_EncodeFrameAsync(session, NULL, surface, &bs, &sp_n);

   MFXVideoCORE_SyncOperation(session, sp_n, INFINITE);
   surface->Data.NumExtParam--;

------------------------
Pipeline Error Reporting
------------------------

During asynchronous pipeline construction, each stage SDK function will return a
synchronization point (sync point). These synchronization points are useful in
tracking errors during the asynchronous pipeline operation.

For example, assume the following pipeline:

.. graphviz::

   digraph {
      rankdir=LR;
      A->B->C;
   }

The application synchronizes on sync point **C**. If the error occurs in SDK
function **C**, then the synchronization returns the exact error code. If the
error occurs before SDK function **C**, then the synchronization returns
MFX_ERR_ABORTED. The application can then try to synchronize on sync point **B**.
Similarly, if the error occurs in SDK function **B**, the synchronization returns
the exact error code, or else MFX_ERR_ABORTED. The same logic applies if the
error occurs in SDK function **A**.
