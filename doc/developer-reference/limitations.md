## Known Limitations

 * Implementation is blocking
 * Decode input bitstream buffer size must be large enough to hold several 
frames.  Buffer sizes which are too small may cause issues. The minimum size 
needed is stream dependent but enough for 10 frames is a conservative estimate. 
 * Only a limited set of parameters are supported -- see supported parameters 
