::------------------------------------------------------------------------------
:: Copyright (C) 2020 Intel Corporation
::
:: SPDX-License-Identifier: MIT
::------------------------------------------------------------------------------
:: start of boilerplate to switch to project root ------------------------------
@echo off
SETLOCAL EnableDelayedExpansion
FOR /D %%i IN ("%~dp0\..") DO (
	set PROJ_DIR=%%~fi
)
cd %PROJ_DIR%
:: start of commands -----------------------------------------------------------
set build_dir=%PROJ_DIR%\_extbuild
set DEFAULT_MSYS_ROOT=C:\tools\msys64
set DEFAULT_VPL_BUILD_DEPENDENCIES=%PROJ_DIR%\_deps

if not defined VPL_BUILD_DEPENDENCIES (
  set VPL_BUILD_DEPENDENCIES=%DEFAULT_VPL_BUILD_DEPENDENCIES%
)
if defined MSYS_ROOT (
     echo MSYS_ROOT found: %MSYS_ROOT%
) else (
     set MSYS_ROOT=%DEFAULT_MSYS_ROOT%
     call echo MSYS_ROOT not found, assuming !MSYS_ROOT!
)
set GITPATH=%PATH%
set MINGWPATH=%MSYS_ROOT%\mingw64\bin;%MSYS_ROOT%\usr\local\bin;^
%MSYS_ROOT%\usr\bin;%MSYS_ROOT%\bin;c:\Windows\System32;c:\Windows;^
c:\Windows\System32\Wbem;c:\Windows\System32\WindowsPowerShell\v1.0\;^
%MSYS_ROOT%\usr\bin\site_perl;%MSYS_ROOT%\usr\bin\vendor_perl;^
%MSYS_ROOT%\usr\bin\core_perl

:: MSYS git does not play with other gits, so use users version if present
where git >NUL
if ERRORLEVEL 1 set GITPATH=%MINGWPATH%

set install_dir=%VPL_BUILD_DEPENDENCIES%

rd /s /q %build_dir% 2>NUL
rd /s /q %VPL_BUILD_DEPENDENCIES% 2>NUL

:: build FFmpeg with SVT-HEVC
md %build_dir% 2>NUL
cd %build_dir%

git clone https://github.com/OpenVisualCloud/SVT-HEVC.git && cd SVT-HEVC
git config advice.detachedHead false
git checkout v1.4.3
mkdir release && cd release
set PATH=%MINGWPATH%
cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ^
-DCMAKE_INSTALL_PREFIX=%install_dir%\ -DBUILD_SHARED_LIBS=off
if ERRORLEVEL 1 exit /b 1
cmake --build . --target install

cd %build_dir%
set PATH=%GITPATH%
git clone https://github.com/FFmpeg/FFmpeg ffmpeg && cd ffmpeg
git checkout release/4.2

git config user.email "bootstrap@localhost"
git config user.name "bootstrap"
set patch=0001-lavc-svt_hevc-add-libsvt-hevc-encoder-wrapper.patch
git am %build_dir%\SVT-HEVC\ffmpeg_plugin\%patch%
if ERRORLEVEL 1 exit /b 1

set PATH=%MINGWPATH%
set PKG_CONFIG_PATH=%install_dir%\lib\pkgconfig;%PKG_CONFIG_PATH%
set install_dir=%install_dir:\=/%
bash -c './configure --extra-cflags="-fPIC" --extra-ldflags="-fPIC" --enable-pic --prefix=${install_dir} --arch=x86_64 --target-os=mingw64 --disable-shared --enable-static --disable-network --disable-everything --disable-doc --disable-manpages --disable-hwaccels --disable-appkit --disable-alsa --disable-avfoundation --disable-iconv --disable-lzma --disable-sdl2 --disable-securetransport --disable-xlib --disable-zlib --disable-amf --disable-audiotoolbox --disable-cuvid --disable-d3d11va --disable-dxva2 --disable-nvdec --disable-nvenc --disable-v4l2-m2m --disable-videotoolbox --enable-indev=lavfi --enable-protocol=file --enable-bsf=h264_mp4toannexb --enable-bsf=hevc_mp4toannexb --enable-decoder=rawvideo --enable-muxer=rawvideo --enable-muxer=h264 --enable-muxer=mpeg2video --enable-muxer=mjpeg --enable-muxer=hevc --enable-filter=testsrc --enable-filter=testsrc2 --enable-filter=rgbtestsrc --enable-filter=yuvtestsrc --enable-demuxer=h264 --enable-parser=h264 --enable-decoder=h264 --enable-demuxer=hevc --enable-decoder=hevc --enable-parser=hevc --enable-decoder=mpeg2video --enable-encoder=mpeg2video --enable-decoder=mjpeg --enable-encoder=mjpeg --enable-filter=scale --enable-filter=psnr --enable-filter=ssim --enable-libsvthevc --enable-encoder=libsvt_hevc'

make -j %NUMBER_OF_PROCESSORS% && make install

:: export build dependency environment
(
endlocal
if not defined VPL_BUILD_DEPENDENCIES (
   set VPL_BUILD_DEPENDENCIES=%DEFAULT_VPL_BUILD_DEPENDENCIES%
)
)
