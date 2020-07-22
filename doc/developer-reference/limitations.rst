=================
Known Limitations
=================

Implementation is not fully asynchronous.  Work happens on separate threads
but decode, VPP, and encode operations block until done instead of returning
immediately.  This does not prevent application code written with correct
use of the sync operation from working, though it may allow unportable code.

|

Decode input bitstream buffer size must be large enough to hold several frames.
Buffer sizes which are too small may cause issues. The minimum size needed is
stream dependent but enough for 10 frames is a conservative estimate.

|
  
Only a limited set of parameters are supported -- see supported parameters.


