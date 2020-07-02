@del /Q out*.i420
call sample_decode.exe h265 -i %~dp0\..\..\test\content\cars_128x96.h265 -o out_vpl_h265.i420 -vpl 
call %VPL_BUILD_DEPENDENCIES%\bin\ffmpeg.exe -y -i ..\..\test\content\cars_128x96.h265 -f rawvideo -pix_fmt yuv420p out_ref_h265.i420
call %VPL_BUILD_DEPENDENCIES%\bin\ffmpeg.exe -y -r 30 -s 128x96 -pix_fmt yuv420p -f rawvideo -i out_vpl_h265.i420 -r 30 -s 128x96 -pix_fmt yuv420p -f rawvideo -i out_ref_h265.i420 -filter_complex psnr= -f null nullsink
md5sum out*.i420
