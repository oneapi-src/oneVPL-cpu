::------------------------------------------------------------------------------
:: Copyright (C) 2020 Intel Corporation
::
:: SPDX-License-Identifier: MIT
::------------------------------------------------------------------------------
:: start of boilerplate to switch to project root ------------------------------
@echo off
SETLOCAL
FOR /D %%i IN ("%~dp0\..") DO (
	set PROJ_DIR=%%~fi
)
cd %PROJ_DIR%
:: start of commands -----------------------------------------------------------
call "%PROJ_DIR%/test/tools/env/vars.bat"
if defined VPL_BUILD_DEPENDENCIES (
  set ffmpeg_dir=%VPL_BUILD_DEPENDENCIES%\bin
) else (
    echo VPL_BUILD_DEPENDENCIES not defined. Did you run bootstrap?
    exit /b 1
  )
)
set "PATH=%ffmpeg_dir%;%PATH%"

cd _build\Release
set /A result_all = 0

:unit_tests
echo *** Running Unit Tests ***
call vpl-utest.exe --gtest_output=xml:%PROJ_DIR%\_logs\
if %errorlevel%==0 goto unit_tests_passed
echo *** Unit Tests FAILED ***
set /A result_all = 1
goto test_decode

:unit_tests_passed
echo *** Unit Tests PASSED ***

:test_decode
echo *** Running Decode Smoke Test***
call sample_decode.exe h265 -i %PROJ_DIR%\test\content\cars_128x96.h265 ^
     -o out_vpl_h265.i420 -vpl
call %VPL_BUILD_DEPENDENCIES%\bin\ffmpeg.exe -y ^
     -i %PROJ_DIR%\test\content\cars_128x96.h265 ^
     -f rawvideo -pixel_format yuv420p out_ref_h265.i420
call %VPL_BUILD_DEPENDENCIES%\bin\ffmpeg.exe -y ^
     -r 30 -video_size 128x96 -pixel_format yuv420p -f rawvideo -i out_vpl_h265.i420 ^
     -r 30 -video_size 128x96 -pixel_format yuv420p -f rawvideo -i out_ref_h265.i420 ^
     -filter_complex psnr= -f null nullsink
call py -3 %PYTHONPATH%\check_content\check_smoke_output.py ^
     out_ref_h265.i420 out_vpl_h265.i420 I420 128x96@30

echo.
if %errorlevel%==0 goto test_decode_passed
echo *** Decode Smoke Test FAILED ***
set /A result_all = 1
goto test_encode

:test_decode_passed
echo *** Decode Smoke Test PASSED ***

:test_encode
echo *** Running Encode Smoke Test***
call sample_encode.exe h265 -i out_ref_h265.i420 ^
     -o out_vpl.h265 -w 128 -h 96 -vpl
call sample_decode.exe h265 -i out_vpl.h265 -o out_vpl_dec_h265.i420 -vpl
call py -3 %PYTHONPATH%\check_content\check_smoke_output.py ^
     out_ref_h265.i420 out_vpl_dec_h265.i420 I420 128x96@30

echo.
if %errorlevel%==0 goto test_encode_passed
echo *** Encode Smoke Test FAILED ***
set /A result_all = 1
goto test_vpp

:test_encode_passed
echo *** Encode Smoke Test PASSED ***

:test_vpp
echo *** Running VPP Smoke Test***
call sample_vpp.exe -sw 128 -sh 96 -scrX 0 -scrY 0 -scrW 96 -scrH 48 -scc i420 ^
     -dw 352 -dh 288 -dcrX 50 -dcrY 50 -dcrW 100 -dcrH 100 -dcc i010 ^
     -i out_ref_h265.i420 ^
     -o out_vpl_vpp.i010 ^
     -lib vpl

set VPP_FILTER=split=2[bg][main];^
               [bg]scale=352:288,drawbox=x=0:y=0:w=352:h=288:t=fill[bg2];^
               [main]crop=96:48:0:0,scale=100:100[ovr];^
               [bg2][ovr]overlay=50:50,format=pix_fmts=yuv420p10le

call %VPL_BUILD_DEPENDENCIES%\bin\ffmpeg.exe -y ^
     -f rawvideo -pixel_format yuv420p -video_size 128x96 ^
     -i out_ref_h265.i420 ^
     -filter_complex "%VPP_FILTER%" ^
     -f rawvideo -pixel_format yuv420p10le -video_size 352x288 ^
     out_ref_vpp.i010
call py -3 %PYTHONPATH%\check_content\check_smoke_output.py ^
     out_vpl_vpp.i010 out_ref_vpp.i010 I010 352x288@30

echo.
if %errorlevel%==0 goto test_vpp_passed
echo *** VPP Smoke Test FAILED ***
set /A result_all = 1
goto test_pipeline_vpp_enc

:test_vpp_passed
echo *** VPP Smoke Test PASSED ***

:test_pipeline_vpp_enc
echo *** Running Pipeline (VPP + Encode) Smoke Test***
call vpl-vppenc.exe out_ref_h265.i420 128 96

call %VPL_BUILD_DEPENDENCIES%\bin\ffmpeg.exe -y ^
     -f rawvideo -pixel_format yuv420p -video_size 128x96 ^
     -i out_ref_h265.i420 ^
     -filter_complex "scale=640:480" ^
     -c:v libsvt_hevc ^
     -g 30 -rc 1 -preset 5 -b:v 4000*1000 ^
     out_ref.h265
call py -3 %PYTHONPATH%\check_content\check_smoke_output.py ^
     out.h265 out_ref.h265 H265 640x480@30

echo.
if %errorlevel%==0 goto test_pipeline_vpp_enc_passed
echo *** Pipeline (VPP + Encode) Smoke Test FAILED ***
set /A result_all = 1
goto test_end

:test_pipeline_vpp_enc_passed
echo *** Pipeline (VPP + Encode) Smoke Test PASSED ***

:test_end
exit /B %result_all%
