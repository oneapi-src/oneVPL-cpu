=========================================
Memory Allocation and External Allocators
=========================================

There are two models of memory management in SDK implementations: internal and external.

-----------------------
Surface Pool Allocation
-----------------------

When connecting SDK function **A** to SDK function **B**, the application must
take into account the requirements of both functions to calculate the number of
frame surfaces in the surface pool. Typically, the application can use the formula
**Na+Nb**, where **Na** is the frame surface requirements from SDK function **A**
output, and **Nb** is the frame surface needs from SDK function **B** input.

For performance considerations, the application must submit multiple operations
and delays synchronization as much as possible, which gives the SDK flexibility
to organize internal pipelining. For example, the following operation sequence:

.. graphviz::

   digraph {
      rankdir=LR;
      f1 [shape=record label="ENCODE(F1)" ];
      f2 [shape=record label="ENCODE(F2)" ];
      f3 [shape=record label="SYNC(F1)" ];
      f4 [shape=record label="SYNC(F2)" ];
      f1->f2->f3->f4;
   }

is recommended, compared with:

.. graphviz::

   digraph {
      rankdir=LR;
      f1 [shape=record label="ENCODE(F1)" ];
      f2 [shape=record label="ENCODE(F2)" ];
      f3 [shape=record label="SYNC(F1)" ];
      f4 [shape=record label="SYNC(F2)" ];
      f1->f3->f2->f4;
   }

In this case, the surface pool needs additional surfaces to take into account
multiple asynchronous operations before synchronization. The application can use
the **AsyncDepth** parameter of the mfxVideoParam structure to inform an SDK
function how many asynchronous operations the application plans to perform
before synchronization. The corresponding SDK **QueryIOSurf** function will
reflect such consideration in the NumFrameSuggested value. The following example
shows a way of calculating the surface needs based on NumFrameSuggested values:

.. code-block:: c++

   async_depth=4;
   init_param_v.AsyncDepth=async_depth;
   MFXVideoVPP_QueryIOSurf(session, &init_param_v, response_v);
   init_param_e.AsyncDepth=async_depth;
   MFXVideoENCODE_QueryIOSurf(session, &init_param_e, &response_e);
   num_surfaces=    response_v[1].NumFrameSuggested
            +response_e.NumFrameSuggested
            -async_depth; /* double counted in ENCODE & VPP */

--------------------------
External Memory Management
--------------------------

In the external memory model the application must allocate sufficient memory for
input and output parameters and buffers, and deallocate it when SDK functions
complete their operations. During execution, the SDK functions use callback
functions to the application to manage memory for video frames through the
external allocator interface mfxFrameAllocator.

If an application needs to control the allocation of video frames, it can use
callback functions through the mfxFrameAllocator interface. If an application
does not specify an allocator, an internal allocator is used. However, if an
application uses video memory surfaces for input and output, it must specify the
hardware acceleration device and an external frame allocator using mfxFrameAllocator.

The external frame allocator can allocate different frame types:

- In-system memory.
- In-video memory, as “decoder render targets” or “processor render targets.”
  See the section `Working with hardware acceleration`_ for additional details.

The external frame allocator responds only to frame allocation requests for the
requested memory type and returns MFX_ERR_UNSUPPORTED for all other types. The
allocation request uses flags, part of the memory type field, to indicate which
SDK class initiated the request, so the external frame allocator can respond
accordingly.

Simple external frame allocator:

.. code-block:: c++

   typedef struct {
      mfxU16 width, height;
      mfxU8 *base;
   } mid_struct;

   mfxStatus fa_alloc(mfxHDL pthis, mfxFrameAllocRequest *request, mfxFrameAllocResponse *response) {
      if (!(request->type&MFX_MEMTYPE_SYSTEM_MEMORY))
         return MFX_ERR_UNSUPPORTED;
      if (request->Info->FourCC!=MFX_FOURCC_NV12)
         return MFX_ERR_UNSUPPORTED;
      response->NumFrameActual=request->NumFrameMin;
      for (int i=0;i<request->NumFrameMin;i++) {
         mid_struct *mmid=(mid_struct *)malloc(sizeof(mid_struct));
         mmid->width=ALIGN32(request->Info->Width);
         mmid->height=ALIGN32(request->Info->Height);
         mmid->base=(mfxU8*)malloc(mmid->width*mmid->height*3/2);
         response->mids[i]=mmid;
      }
      return MFX_ERR_NONE;
   }

   mfxStatus fa_lock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr) {
      mid_struct *mmid=(mid_struct *)mid;
      ptr->pitch=mmid->width;
      ptr->Y=mmid->base;
      ptr->U=ptr->Y+mmid->width*mmid->height;
      ptr->V=ptr->U+1;
      return MFX_ERR_NONE;
   }

   mfxStatus fa_unlock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr) {
      if (ptr) ptr->Y=ptr->U=ptr->V=ptr->A=0;
      return MFX_ERR_NONE;
   }

   mfxStatus fa_gethdl(mfxHDL pthis, mfxMemId mid, mfxHDL *handle) {
      return MFX_ERR_UNSUPPORTED;
   }

   mfxStatus fa_free(mfxHDL pthis, mfxFrameAllocResponse *response) {
      for (int i=0;i<response->NumFrameActual;i++) {
         mid_struct *mmid=(mid_struct *)response->mids[i];
         free(mmid->base); free(mid);
      }
      return MFX_ERR_NONE;
   }


For system memory, it is highly recommended to allocate memory for all
planes of the same frame as a single buffer (using one single malloc call).

--------------------------
Internal Memory Management
--------------------------

In the internal memory management model, the SDK provides interface functions for
frames allocation:

:cpp:func:`MFXMemory_GetSurfaceForVPP`

:cpp:func:`MFXMemory_GetSurfaceForEncode`

:cpp:func:`MFXMemory_GetSurfaceForDecode`

These functions are used together with :cpp:struct:`mfxFrameSurfaceInterface`
for surface management. The surface returned by these function is a reference
counted object and the application must call :cpp:member:`mfxFrameSurfaceInterface::Release`
after finishing all operations with the surface. In this model the application
doesn't need to create and set the external allocator to the SDK.
Another method to obtain an internally allocated surface is to call
:cpp:func:`MFXVideoDECODE_DecodeFrameAsync` with a working surface equal to NULL
(see :ref:`Simplified decoding procedure <simplified-decoding-procedure>`). In
this scenario, the Decoder will allocate a new refcountable
:cpp:struct:`mfxFrameSurface1` and return it to the user. All assumed contracts
with the user are similar to functions MFXMemory_GetSurfaceForXXX.

------------------------
mfxFrameSurfaceInterface
------------------------

oneVPL API version 2.0 introduces :cpp:struct:`mfxFrameSurfaceInterface`. This
interface is a set of callback functions to manage the lifetime of allocated
surfaces, get access to pixel data, and obtain native handles and device
abstractions (if suitable). It's recommended to use mfxFrameSurface1::mfxFrameSurfaceInterface
if present, instead of directly accessing :cpp:struct:`mfxFrameSurface1` structure
members or call external allocator callback functions if set.

The following example shows the usage of :cpp:struct:`mfxFrameSurfaceInterface`
for memory sharing:

.. code-block:: c++


    // let decode frame and try to access output optimal way.
    sts = MFXVideoDECODE_DecodeFrameAsync(session, NULL, NULL, &outsurface, &syncp);
    if (MFX_ERR_NONE == sts)
    {
        outsurface->FrameInterface->(*GetDeviceHandle)(outsurface, &device_handle, &device_type);
        // if application or component is familar with mfxHandleType and it's possible to share memory created by device_handle.
        if (isDeviceTypeCompatible(device_type) && isPossibleForMemorySharing(device_handle)) {
            // get native handle and type
            outsurface->FrameInterface->(*GetNativeHandle)(outsurface, &resource, &resource_type);
            if (isResourceTypeCompatible(resource_type)) {
                //use memory directly
                ProcessNativeMemory(resource);
                outsurface->FrameInterface->(*Release)(outsurface);
            }
        }
        // Application or component is not aware about such DeviceHandle or Resource type need to map to system memory.
        outsurface->FrameInterface->(*Map)(outsurface, MFX_MAP_READ);
        ProcessSystemMemory(outsurface);
        outsurface->FrameInterface->(*Unmap)(outsurface);
        outsurface->FrameInterface->(*Release)(outsurface);
    }



----------------
Frame and Fields
----------------

In SDK terminology, a frame (or frame surface, interchangeably) contains either
a progressive frame or a complementary field pair. If the frame is a complementary
field pair, the odd lines of the surface buffer store the top fields and the even
lines of the surface buffer store the bottom fields.

Frame Surface Locking
---------------------

During encoding, decoding, or video processing, cases arise that require reserving
input or output frames for future use. For example, with decoding a frame that is
ready for output must remain as a reference frame until the current sequence
pattern ends. The usual approach is to cache the frames internally. This method
requires a copy operation, which can significantly reduce performance.

SDK functions define a frame-locking mechanism to avoid the need for copy
operations. This mechanism is as follows:

- The application allocates a pool of frame surfaces large enough to include SDK
  function I/O frame surfaces and internal cache needs. Each frame surface
  maintains a Locked counter, part of the mfxFrameData structure. Initially, the
  Locked counter is set to zero.
- The application calls an SDK function with frame surfaces from the pool, whose
  Locked counter is set as appropriate. For decoding or video processing
  operations, where the SDK uses the surfaces to write, the Locked counter
  should be equal to zero. If the SDK  function needs to reserve any frame surface,
  the SDK function increases the Locked counter of the frame surface. A non-zero
  Locked counter indicates that the calling application must treat the frame
  surface as “in use.” When the frame surface is in use, the application can read,
  but cannot alter, move, delete, or free the frame surface.
- In subsequent SDK executions, if the frame surface is no longer in use, the
  SDK decreases the Locked counter. When the Locked counter reaches zero, the
  application is free to do as it wishes with the frame surface.

In general, the application must not increase or decrease the Locked counter,
since the SDK manages this field. If, for some reason, the application needs to
modify the Locked counter, the operation must be atomic to avoid a race condition.

.. attention:: Modifying the Locked counter is not recommended.

Starting with API version 2.0, the mfxFrameSurfaceInterface structure as a set of
callback functions was introduced for mfxFrameSurface1 to work with frames.
This interface defines mfxFrameSurface1 as a reference counted object which can
be allocated by the SDK or application. The application must follow the general
rules of operations with reference counted objects. For example, when surfaces
are allocated by the SDK during MFXVideoDECODE_DecodeFrameAsync or with help of
MFXMemory_GetSurfaceForVPP or MFXMemory_GetSurfaceForEncode, the application must
call the corresponding mfxFrameSurfaceInterface->(\*Release) for the surfaces
that are no longer in use.

.. attention:: Need to distinguish Locked counter which defines read/write access
               polices and reference counter responsible for managing frames'
               lifetime.

.. note:: All mfxFrameSurface1 structures starting from mfxFrameSurface1::mfxStructVersion = {1,1}
          support the mfxFrameSurfaceInterface.
