The **ENCODE** class of functions takes raw frames as input and compresses them into a bitstream.

Input frames usually come encoded in a repeated pattern called the Group of Picture (GOP) sequence.
For example, a GOP sequence can start from an I-frame, followed by a few B-frames, a P-frame, and so on.
**ENCODE** uses an MPEG-2 style GOP sequence structure that can specify the length of the sequence and the
distance between two key frames: I- or P-frames. A GOP sequence ensures that the segments of a bitstream do not
completely depend upon each other. It also enables decoding applications to reposition the bitstream.


An **ENCODE** output consists of one frame of a bitstream with the time stamp passed from the input frame.
The time stamp is used for multiplexing subsequent video with other associated data such as audio. The SDK library provides
only pure video stream encoding. The application must provide its own multiplexing.



Encoding procedure

There are two ways of  allocation and handling in SDK for shared memory: external and internal.

Example below shows the pseudo code of the encoding procedure with external memory  (legacy mode).

.. code-block:: c++

   MFXVideoENCODE_QueryIOSurf(session, &init_param, &request);
   allocate_pool_of_frame_surfaces(request.NumFrameSuggested);
   MFXVideoENCODE_Init(session, &init_param);
   sts=MFX_ERR_MORE_DATA;
   for (;;) {
      if (sts==MFX_ERR_MORE_DATA && !end_of_stream()) {
         find_unlocked_surface_from_the_pool(&surface);
         fill_content_for_encoding(surface);
      }
      surface2=end_of_stream()?NULL:surface;
      sts=MFXVideoENCODE_EncodeFrameAsync(session,NULL,surface2,bits,&syncp);
      if (end_of_stream() && sts==MFX_ERR_MORE_DATA) break;
      // Skipped other error handling
      if (sts==MFX_ERR_NONE) {
         MFXVideoCORE_SyncOperation(session, syncp, INFINITE);
         do_something_with_encoded_bits(bits);
      }
   }
   MFXVideoENCODE_Close();
   free_pool_of_frame_surfaces();

The following describes a few key points:

- The application uses the MFXVideoENCODE_QueryIOSurf function to obtain the number of working frame surfaces
  required for reordering input frames.
- The application calls the MFXVideoENCODE_EncodeFrameAsync function for the encoding operation. The input frame
  must be in an unlocked frame surface from the frame surface pool. If the encoding output is not available,
  the function returns the status code MFX_ERR_MORE_DATA to request additional input frames.
- Upon successful encoding, the MFXVideoENCODE_EncodeFrameAsync function returns MFX_ERR_NONE. However,
  the encoded bitstream is not yet available because the MFXVideoENCODE_EncodeFrameAsync function is asynchronous.
  The application must use the MFXVideoCORE_SyncOperation function to synchronize the encoding operation
  before retrieving the encoded bitstream.
- At the end of the stream, the application continuously calls the MFXVideoENCODE_EncodeFrameAsync function
  with NULL surface pointer to drain any remaining bitstreams cached within the SDK encoder, until the function
  returns MFX_ERR_MORE_DATA.

.. note:: It is the application's responsibility to fill pixels outside of crop window when it is smaller than
   frame to be encoded. Especially in cases when crops are not aligned to minimum coding block size (16 for AVC,
   8 for HEVC and VP9).

Another approach is when SDK allocates memory for shared objects internally. 

.. code-block:: c++

   MFXVideoENCODE_Init(session, &init_param);
   sts=MFX_ERR_MORE_DATA;
   for (;;) {
      if (sts==MFX_ERR_MORE_DATA && !end_of_stream()) {
         MFXMemory_GetSurfaceForEncode(&surface);
         fill_content_for_encoding(surface);
      }
      surface2=end_of_stream()?NULL:surface;
      sts=MFXVideoENCODE_EncodeFrameAsync(session,NULL,surface2,bits,&syncp);
      if (surface2) surface->FrameInterface->(*Release)(surface2);
      if (end_of_stream() && sts==MFX_ERR_MORE_DATA) break;
      // Skipped other error handling
      if (sts==MFX_ERR_NONE) {
         MFXVideoCORE_SyncOperation(session, syncp, INFINITE);
         do_something_with_encoded_bits(bits);
      }
   }
   MFXVideoENCODE_Close();

There are several key points which are different from legacy mode:

- The application doesn't need to call MFXVideoENCODE_QueryIOSurf function to obtain the number of working frame surfaces since allocation is done by SDK
- The application calls the MFXMemory_GetSurfaceForEncode function to get free surface for the following encode operation.
- The application needs to call the FrameInterface->(\*Release) function to decrement reference counter of the obtained surface after MFXVideoENCODE_EncodeFrameAsync call.


Configuration Change


The application changes configuration during encoding by calling MFXVideoENCODE_Reset function. Depending on
difference in configuration parameters before and after change, the SDK encoder either continues current sequence
or starts a new one. If the SDK encoder starts a new sequence it completely resets internal state and begins a new
sequence with IDR frame.

The application controls encoder behavior during parameter change by attaching mfxExtEncoderResetOption to
mfxVideoParam structure during reset. By using this structure, the application instructs encoder to start or not
to start a new sequence after reset. In some cases request to continue current sequence cannot be satisfied and
encoder fails during reset. To avoid such cases the application may query reset outcome before actual reset
by calling MFXVideoENCODE_Query function with mfxExtEncoderResetOption attached to mfxVideoParam structure.

The application uses the following procedure to change encoding configurations:

- The application retrieves any cached frames in the SDK encoder by calling the MFXVideoENCODE_EncodeFrameAsync
  function with a NULL input frame pointer until the function returns MFX_ERR_MORE_DATA.

.. note:: The application must set the initial encoding configuration flag EndOfStream of the mfxExtCodingOption
   structure to OFF to avoid inserting an End of Stream (EOS) marker into the bitstream. An EOS marker causes
   the bitstream to terminate before encoding is complete.

- The application calls the MFXVideoENCODE_Reset function with the new configuration:

   - If the function successfully set the configuration, the application can continue encoding as usual.
   - If the new configuration requires a new memory allocation, the function returns MFX_ERR_INCOMPATIBLE_VIDEO_PARAM.
     The application must close the SDK encoder and reinitialize the encoding procedure with the new configuration.

