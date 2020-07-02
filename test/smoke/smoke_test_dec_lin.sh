#!/bin/bash

export LD_LIBRARY_PATH=$PWD
rm out*.i420
./sample_decode h265 -i ../test/content/cars_128x96.h265 -o out_vpl_h265.i420 -vpl 
$VPL_BUILD_DEPENDENCIES/bin/ffmpeg -y -i ../test/content/cars_128x96.h265 -f rawvideo -pix_fmt yuv420p out_ref_h265.i420
$VPL_BUILD_DEPENDENCIES/bin/ffmpeg -y -r 30 -s 128x96 -pix_fmt yuv420p -f rawvideo -i out_vpl_h265.i420 -r 30 -s 128x96 -pix_fmt yuv420p -f rawvideo -i out_ref_h265.i420 -filter_complex psnr= -f null nullsink
md5sum out*.i420
