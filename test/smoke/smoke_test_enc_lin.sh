#!/bin/bash

rm out*.i420 out*.h265
./sample_encode h265 -i /msdk/vpl/test_streams/YUV/cars_1280x720.i420 -o out_vpl.h265 -w 1280 -h 720 -vpl 
./sample_decode h265 -i out_vpl.h265 -o out_vpl_dec_h265.i420 -vpl
/msdk/vpl/reference/bin/ffmpeg/ffmpeg -y -r 30 -s 1280x720 -pix_fmt yuv420p -f rawvideo -i /msdk/vpl/test_streams/YUV/cars_1280x720.i420 -r 30 -s 1280x720 -pix_fmt yuv420p -f rawvideo -i out_vpl_dec_h265.i420 -filter_complex psnr= -f null nullsink

