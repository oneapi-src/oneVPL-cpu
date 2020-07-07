The mfxFrameInfo structure is used by both the :cpp:struct:`mfxVideoParam` structure during SDK class initialization and the
:cpp:struct:`mfxFrameSurface1` structure during the actual SDK class function. The following constraints apply:

Constraints common for **DECODE**, **ENCODE** and **VPP**:

+--------------+---------------------------+---------------------------------------------------------------------------------------------+
| Parameters   | During SDK initialization | During SDK operation                                                                        |
+==============+===========================+=============================================================================================+
| FourCC       | Any valid value           | The value must be the same as the initialization value.                                     |
|              |                           |                                                                                             |
|              |                           | The only exception is **VPP** in composition mode, where in some cases it is allowed to     |
|              |                           |                                                                                             |
|              |                           | mix RGB and NV12 surfaces. See :cpp:struct:`mfxExtVPPComposite` for more details.           |
+--------------+---------------------------+---------------------------------------------------------------------------------------------+
| ChromaFormat | Any valid value           | The value must be the same as the initialization value.                                     |
+--------------+---------------------------+---------------------------------------------------------------------------------------------+

Constraints for **DECODE**:

+-------------------+-------------------------------------------------------------+------------------------------------------------------------+
| Parameters        | During SDK initialization                                   | During SDK operation                                       |
+===================+=============================================================+============================================================+
| Width             | Aligned frame size                                          | The values must be the equal to or larger than the         |
|                   |                                                             |                                                            |
| Height            |                                                             | initialization values.                                     |
+-------------------+-------------------------------------------------------------+------------------------------------------------------------+
| CropX, CropY      | Ignored                                                     | **DECODE** output. The cropping values are per-frame based.|
|                   |                                                             |                                                            |
| CropW, CropH      |                                                             |                                                            |
+-------------------+-------------------------------------------------------------+------------------------------------------------------------+
| AspectRatioW      | Any valid values or unspecified (zero); if unspecified,     | **DECODE** output.                                         |
|                   |                                                             |                                                            |
| AspectRatioH      | values from the input bitstream will be used;               |                                                            |
|                   |                                                             |                                                            |
|                   | see note below the table.                                   |                                                            |
+-------------------+-------------------------------------------------------------+------------------------------------------------------------+
| FrameRateExtN     | If unspecified, values from the input bitstream will be     | **DECODE** output.                                         |
|                   |                                                             |                                                            |
| FrameRateExtD     | used; see note below the table.                             |                                                            |
+-------------------+-------------------------------------------------------------+------------------------------------------------------------+
| PicStruct         | Ignored                                                     | **DECODE** output.                                         |
+-------------------+-------------------------------------------------------------+------------------------------------------------------------+

.. note:: Note about priority of initialization parameters.

.. note:: If application explicitly sets FrameRateExtN/FrameRateExtD or AspectRatioW/AspectRatioH during initialization then decoder uses these
          values during decoding regardless of values from bitstream and does not update them on new SPS. If application sets them to 0, then decoder
          uses values from stream and update them on each SPS.



Constraints for **ENCODE**:

+-------------------+----------------------------------------------+---------------------------------------------------------------------------+
| Parameters        | During SDK initialization                    | During SDK operation                                                      |
+===================+==============================================+===========================================================================+
| Width, Height     | Encoded frame aligned size                   | Must be the <=  the initialization values                                 |
+-------------------+----------------------------------------------+---------------------------------------------------------------------------+
| CropX, CropY      | must be zero                                 | Ignored                                                                   |
+-------------------+----------------------------------------------+---------------------------------------------------------------------------+
|                   |                                              |                                                                           |
| CropW, CropH      | specify the real width and height            |                                                                           |
|                   | (maybe unaligned) of the coded frames        |                                                                           |
|                   |                                              |                                                                           |
+-------------------+----------------------------------------------+---------------------------------------------------------------------------+
| AspectRatioW      | Any valid values                             | Ignored                                                                   |
|                   |                                              |                                                                           |
| AspectRatioH      |                                              |                                                                           |
+-------------------+----------------------------------------------+---------------------------------------------------------------------------+
| FrameRateExtN     | Any valid values                             | Ignored                                                                   |
|                   |                                              |                                                                           |
| FrameRateExtD     |                                              |                                                                           |
+-------------------+----------------------------------------------+---------------------------------------------------------------------------+


The following table summarizes how to specify the configuration parameters during initialization and during encoding, decoding and video processing:

=========================== =============== =================== =============== =================== ============ ==================
Structure (param)           **ENCODE** Init **ENCODE** Encoding **DECODE** Init **DECODE** Decoding **VPP** Init **VPP** Processing
=========================== =============== =================== =============== =================== ============ ==================
:cpp:struct:`mfxVideoParam`	   					
   Protected                R               -                   R               -                   R            -
   IOPattern                M               -                   M               -                   M            -
   ExtParam                 O               -                   O               -                   O            -
   NumExtParam              O               -                   O               -                   O            -
:cpp:struct:`mfxInfoMFX`						
   CodecId                  M               -                   M               -                   -            -
   CodecProfile             O               -                   O/M*            -                   -            -
   CodecLevel               O               -                   O               -                   -            -
   NumThread                O               -                   O               -                   -            -
   TargetUsage              O               -                   -               -                   -            -
   GopPicSize               O               -                   -               -                   -            -
   GopRefDist               O               -                   -               -                   -            -
   GopOptFlag               O               -                   -               -                   -            -
   IdrInterval              O               -                   -               -                   -            -
   RateControlMethod        O               -                   -               -                   -            -
   InitialDelayInKB         O               -                   -               -                   -            -
   BufferSizeInKB           O               -                   -               -                   -            -
   TargetKbps               M               -                   -               -                   -            -
   MaxKbps                  O               -                   -               -                   -            -
   NumSlice                 O               -                   -               -                   -            -
   NumRefFrame              O               -                   -               -                   -            -
   EncodedOrder             M               -                   -               -                   -            -
:cpp:struct:`mfxFrameInfo`						
   FourCC                   M               M                   M               M                   M            M
   Width                    M               M                   M               M                   M            M
   Height                   M               M                   M               M                   M            M
   CropX                    M               Ign                 Ign             U                   Ign          M
   CropY                    M               Ign                 Ign             U                   Ign          M
   CropW                    M               Ign                 Ign             U                   Ign          M
   CropH                    M               Ign                 Ign             U                   Ign          M
   FrameRateExtN            M               Ign                 O               U                   M            U
   FrameRateExtD            M               Ign                 O               U                   M            U
   AspectRatioW             O               Ign                 O               U                   Ign          PT
   AspectRatioH             O               Ign                 O               U                   Ign          PT
   PicStruct                O               M                   Ign             U                   M            M/U
   ChromaFormat             M               M                   M               M                   Ign          Ign
=========================== =============== =================== =============== =================== ============ ==================

Table Legend:

======= =================
Remarks
======= =================
Ign     Ignored
PT      Pass Through
-       Does Not Apply
M       Mandated
R       Reserved
O       Optional
U       Updated at output
======= =================
