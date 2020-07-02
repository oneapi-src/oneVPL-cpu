#!/bin/bash

export LD_LIBRARY_PATH=$PWD
rm out*.i420 out*.h265
./sample_encode h265 -i /msdk/vpl/test_streams/YUV/cars_128x96.i420 -o out_vpl.h265 -w 128 -h 96 -vpl 
./sample_decode h265 -i out_vpl.h265 -o out_vpl_dec_h265.i420 -vpl
$VPL_BUILD_DEPENDENCIES/bin/ffmpeg -y -r 30 -s 128x96 -pix_fmt yuv420p -f rawvideo -i /msdk/vpl/test_streams/YUV/cars_128x96.i420 -r 30 -s 128x96 -pix_fmt yuv420p -f rawvideo -i out_vpl_dec_h265.i420 -filter_complex psnr= -f null nullsink

