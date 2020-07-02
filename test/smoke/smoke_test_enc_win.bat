@del /Q out*.i420 out*.h265
call sample_encode.exe h265 -i c:\msdk_validation\vpl\test_streams\YUV\cars_128x96.i420 -o out_vpl.h265 -w 128 -h 96 -vpl 
call sample_decode.exe h265 -i out_vpl.h265 -o out_vpl_dec_h265.i420 -vpl
call c:\bin\ffmpeg\ffmpeg.exe -y -r 30 -s 128x96 -pix_fmt yuv420p -f rawvideo -i c:\msdk_validation\vpl\test_streams\YUV\cars_128x96.i420 -r 30 -s 128x96 -pix_fmt yuv420p -f rawvideo -i out_vpl_dec_h265.i420 -filter_complex psnr= -f null nullsink
