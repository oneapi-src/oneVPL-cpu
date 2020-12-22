###############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
###############################################################################
"""Build oneVPL-cpu ffmpeg dependencies"""
import sys
import os
import argparse
import subprocess
from pathlib import Path
from os import environ


def main():
    """Main steps to build ffmpeg and dependencies"""

    proj_dir = str(Path(os.path.dirname(os.path.realpath(sys.argv[0]))).parent)
    build_dir = proj_dir + "/_extbuild"
    install_dir = proj_dir + "/_deps"
    build_mode = "Release"
    parser = argparse.ArgumentParser(prog="bootstrap")

    parser.add_argument('-m',
                        "--build_mode",
                        dest='build_mode',
                        choices=['Release', 'Debug'],
                        default='Release',
                        help='Build mode')

    parser.add_argument('-gpl',
                        "--use_gpl",
                        dest='use_gpl',
                        action="store_true",
                        help='Use GPL codecs (x264)')

    parser.add_argument(
        '-clean',
        dest='clean',
        action="store_true",
        help='Remove previous build/install dirs before starting')

    args = parser.parse_args()

    build_mode = args.build_mode

    build_dir = proj_dir + "/_extbuild"
    if "VPL_BUILD_DEPENDENCIES" in os.environ:
        install_dir = environ.get("VPL_BUILD_DEPENDENCIES")
    else:
        os.environ["VPL_BUILD_DEPENDENCIES"] = install_dir

    if args.clean:
        os.system("rm -rf %s %s" % (build_dir, install_dir))

    os.system("mkdir -p %s" % build_dir)
    os.system("mkdir -p %s" % install_dir)

    #build dependencies
    if args.use_gpl:
        build_gpl_x264_encoder(build_dir, install_dir)

    #build_aom_av1_decoder(build_dir, install_dir)
    build_dav1d_decoder(build_dir, install_dir)
    build_svt_av1_encoder(build_dir, install_dir)
    build_svt_hevc_encoder(build_dir, install_dir)

    #prepare ffmpeg build
    clone_ffmpeg(build_dir)
    cmd = configure_ffmpeg(build_dir, install_dir)
    cmd += configure_ffmpeg_debug(build_mode)
    cmd += configure_ffmpeg_3rdparty(build_dir, install_dir)

    #run configure
    os.system(cmd)

    #build ffmpeg
    os.system("cd %s/ffmpeg; make -j $(nproc) && make install" % (build_dir))


def build_aom_av1_decoder(build_dir, install_dir):
    """build libaom from source"""

    if os.path.isdir("%s/aom" % (build_dir)):
        print("using existing aom av1 decoder dir")
        return

    cmd = "cd %s; " % (build_dir)
    cmd += "git clone --depth=1 -bmaster https://aomedia.googlesource.com/aom; "
    cmd += "cd aom; mkdir _build; cd _build;"
    cmd += "cmake .. "
    cmd += "    -DCMAKE_INSTALL_PREFIX=%s " % (install_dir)
    cmd += "-DCONFIG_AV1_ENCODER=0  -DBUILD_SHARED_LIBS=0;"
    cmd += " make -j $nproc; make install"
    os.system(cmd)


def build_dav1d_decoder(build_dir, install_dir):
    """build libdav1d from source"""

    if os.path.isdir("%s/dav1d" % (build_dir)):
        print("using existing david decoder dir")
        return

    cmd = "cd %s; " % (build_dir)
    cmd += "git clone --depth=1 -b0.7.0  https://code.videolan.org/videolan/dav1d.git; "
    cmd += "cd dav1d; "
    cmd += "meson build --prefix %s " % (install_dir)
    cmd += "--libdir %s/lib " % (install_dir)
    cmd += "--buildtype release --default-library=static -Denable_avx512=false;"
    cmd += "ninja -C build; cd build; ninja install"
    os.system(cmd)

    if os.path.isfile("%s/lib/pkgconfig/dav1d_edited"):
        print("dav1d.pc already edited")
    else:
        cmd = "cd %s/lib/pkgconfig; " % (install_dir)
        cmd += "sed -i 's/-ldav1d/-ldav1d -pthread -ldl/' dav1d.pc; "
        cmd += "touch dav1d_edited;"
        os.system(cmd)


def build_svt_hevc_encoder(build_dir, install_dir):
    """build SVT HEVC encoder from source"""

    if os.path.isdir("%s/SVT-HEVC" % (build_dir)):
        print("using existing SVT-HEVC encoder dir")
        return

    cmd = "cd %s; " % (build_dir)
    cmd += "git clone --depth=1 -b v1.5.0 https://github.com/OpenVisualCloud/SVT-HEVC.git;"
    cmd += "cd SVT-HEVC; "
    cmd += "sed -i 's/#define LIB_PRINTF_ENABLE                1"
    cmd += "/#define LIB_PRINTF_ENABLE                0/' "
    cmd += "    ./Source/Lib/Codec/EbDefinitions.h;"
    cmd += "mkdir release && cd release; "
    cmd += "cmake .. "
    cmd += "    -DCMAKE_INSTALL_PREFIX=%s -DBUILD_SHARED_LIBS=off -DBUILD_APP=off;" % (
        install_dir)
    cmd += "make -j $(nproc); make install"
    os.system(cmd)

    if os.path.isdir("%s/lib64" % (install_dir)):
        cmd = "cp %s/lib64/* %s/lib;" % (install_dir, install_dir)
        cmd += "mkdir -p %s/lib/pkgconfig; " % (install_dir)
        cmd += "cp %s/lib64/pkgconfig/* %s/lib/pkgconfig;" % (install_dir,
                                                              install_dir)
        cmd += "sed -i 's/lib64/lib/' %s/lib/pkgconfig/SvtHevcEnc.pc" % (
            install_dir)
        os.system(cmd)


def build_svt_av1_encoder(build_dir, install_dir):
    """build SVT AV1 encoder from source"""

    if os.path.isdir("%s/SVT-AV1" % (build_dir)):
        print("using existing SVT-AV1 encoder dir")
        return

    cmd = "cd %s; " % (build_dir)
    cmd += "git clone --depth=1 -b v0.8.4 https://github.com/AOMediaCodec/SVT-AV1; "
    cmd += "cd SVT-AV1; mkdir release && cd release; "
    cmd += "cmake .. "
    cmd += " -DCMAKE_INSTALL_PREFIX=%s " % (install_dir)
    cmd += " -DBUILD_SHARED_LIBS=off -DBUILD_APPS=off "
    cmd += " -DBUILD_DEC=off -DCMAKE_C_FLAGS=-DSVT_LOG_QUIET=1; "
    cmd += "make -j $(nproc); make install"
    #cmd += "make; make install"
    os.system(cmd)

    if os.path.isdir("%s/lib64" % (install_dir)):
        cmd = "cp %s/lib64/* %s/lib;" % (install_dir, install_dir)
        cmd += "mkdir -p %s/lib/pkgconfig; " % (install_dir)
        cmd += "cp %s/lib64/pkgconfig/* %s/lib/pkgconfig;" % (install_dir,
                                                              install_dir)
        cmd += "sed -i 's/lib64/lib/' %s/lib/pkgconfig/SvtAv1Enc.pc" % (
            install_dir)
        os.system(cmd)


def build_gpl_x264_encoder(build_dir, install_dir):
    """build x264 encoder from source"""
    if os.path.isdir("%s/x264" % (build_dir)):
        print("using existing x264 encoder dir")
        return

    cmd = "cd %s; " % (build_dir)
    cmd += "git clone --depth 1 -b stable https://code.videolan.org/videolan/x264.git; "
    cmd += "cd x264;"
    cmd += "./configure --prefix=%s --enable-static --enable-pic;" % (
        install_dir)
    cmd += "make -j $(nproc); make install"
    os.system(cmd)


def clone_ffmpeg(build_dir):
    """get ffmpeg library source"""
    if os.path.isdir("%s/ffmpeg" % (build_dir)):
        print("using existing ffmpeg dir")
    else:
        cmd = "cd %s;" % (build_dir)
        cmd += "git clone --depth=1 -bn4.3.1 https://github.com/FFmpeg/FFmpeg ffmpeg;"
        os.system(cmd)


def configure_ffmpeg(build_dir, install_dir):
    """run set up configure command line for ffmpeg build"""
    cmd = "cd %s/ffmpeg;" % (build_dir)

    #set PKG_CONFIG_PATH so that configure can find external codecs
    cmd += "export PKG_CONFIG_PATH=%s/lib/pkgconfig:$PKG_CONFIG_PATH;" % (
        install_dir)

    #create configure command line
    cmd += "./configure --disable-vaapi "
    cmd += " --disable-cuda-llvm "
    cmd += " --prefix=%s " % (install_dir)
    cmd += " --enable-static --disable-shared --enable-pic "
    cmd += " --disable-everything --disable-network --disable-doc "
    cmd += " --disable-manpages --disable-hwaccels --disable-appkit "
    cmd += " --disable-alsa  --disable-avfoundation --disable-iconv "
    cmd += " --disable-lzma --disable-securetransport  --disable-xlib "
    cmd += " --disable-zlib --disable-amf --disable-audiotoolbox "
    cmd += " --disable-cuvid "
    cmd += " --disable-d3d11va "
    cmd += " --disable-dxva2 "
    cmd += " --disable-nvdec  "
    cmd += " --disable-nvenc "
    cmd += " --disable-v4l2-m2m "
    cmd += " --disable-videotoolbox  "
    cmd += " --disable-avdevice --disable-swresample "
    cmd += " --disable-sdl2 "
    cmd += " --enable-indev=lavfi "
    cmd += " --enable-protocol=file  "
    cmd += " --enable-bsf=h264_mp4toannexb "
    cmd += " --enable-bsf=hevc_mp4toannexb  "
    cmd += " --enable-bsf=mjpeg2jpeg  "
    cmd += " --enable-bsf=mjpega_dump_header  "
    cmd += " --enable-decoder=rawvideo "
    cmd += " --enable-encoder=rawvideo "
    cmd += " --enable-demuxer=rawvideo  "
    cmd += " --enable-demuxer=mjpeg  "
    cmd += " --enable-muxer=rawvideo  "
    cmd += " --enable-muxer=null "
    cmd += " --enable-decoder=wrapped_avframe "
    cmd += " --enable-encoder=wrapped_avframe "
    cmd += " --enable-muxer=h264 "
    cmd += " --enable-muxer=mpeg2video "
    cmd += " --enable-muxer=mjpeg  --enable-muxer=hevc --enable-muxer=ivf  "
    cmd += " --enable-filter=testsrc  --enable-demuxer=image2 --enable-muxer=image2 "
    cmd += " --enable-filter=yuvtestsrc --enable-filter=rgbtestsrc "
    cmd += " --enable-decoder=h264 --enable-parser=h264 --enable-demuxer=h264 "
    cmd += " --enable-decoder=hevc --enable-demuxer=hevc "
    cmd += " --enable-demuxer=ivf  --enable-parser=hevc  "
    cmd += " --enable-parser=mjpeg  --enable-parser=av1 "
    cmd += " --enable-decoder=mpeg2video --enable-encoder=mpeg2video "
    cmd += " --enable-encoder=mjpeg  --enable-decoder=mjpeg "
    cmd += " --enable-filter=overlay  --enable-filter=crop  --enable-filter=scale "
    cmd += " --enable-filter=drawbox --enable-filter=psnr --enable-filter=split "
    cmd += " --enable-filter=select --enable-filter=concat --enable-filter=ssim "
    return cmd


def configure_ffmpeg_debug(build_mode):
    """add ffmpeg configure debug flags if requested"""
    cmd = ""
    if "Debug" in build_mode:
        cmd += " --disable-optimizations --extra-cflags=-Og "
        cmd += " --extra-cflags=-fno-omit-frame-pointer --enable-debug=3 "
        cmd += " --extra-cflags=-fno-inline "

    return cmd


def configure_ffmpeg_3rdparty(build_dir, install_dir):
    """update ffmpeg configure command line based on packages findable
    by pkg-config"""

    env_vars = os.environ.copy()
    if "PKG_CONFIG_PATH" in env_vars.keys():
        tmpstr = "%s/lib/pkgconfig:" % (
            install_dir) + env_vars["PKG_CONFIG_PATH"]
        env_vars["PKG_CONFIG_PATH"] = tmpstr
    else:
        env_vars["PKG_CONFIG_PATH"] = "%s/lib/pkgconfig:" % (install_dir)

    pkg_list = subprocess.check_output(["pkg-config", "--list-all"],
                                       stderr=subprocess.STDOUT,
                                       universal_newlines=True,
                                       env=env_vars)
    cmd = ""
    if "aom" in pkg_list:
        print("aom decoder found")
        cmd += " --enable-libaom --enable-decoder=libaom_av1 "

    if "dav1d" in pkg_list:
        print("dav1d decoder found")
        cmd += "--enable-libdav1d --enable-decoder=libdav1d "

    if "x264" in pkg_list:
        print("x264 encoder found")
        cmd += "--enable-gpl --enable-libx264 --enable-encoder=libx264 "

    if "SvtAv1Enc" in pkg_list:
        print("SVT-AV1 encoder found")
        cmd += "--enable-libsvtav1 --enable-encoder=libsvt_av1 "

        if os.path.isfile("%s/ffmpeg/svt-av1-patched" % (build_dir)):
            print("SVT-AV1 patch already applied")
        else:
            patchcmd = "cd %s/ffmpeg;" % (build_dir)
            patchcmd += 'git config user.email "bootstrap@localhost"; '
            patchcmd += 'git config user.name "bootstrap"; '
            patchcmd += 'git am %s/SVT-AV1/ffmpeg_plugin/0001-Add-ability' % (
                build_dir)
            patchcmd += '-for-ffmpeg-to-run-svt-av1.patch; '
            patchcmd += 'touch svt-av1-patched'
            os.system(patchcmd)

    if "SvtHevcEnc" in pkg_list:
        print("SVT-HEVC encoder found")
        cmd += "--enable-libsvthevc --enable-encoder=libsvt_hevc "

        if os.path.isfile("%s/ffmpeg/svt-hevc-patched" % (build_dir)):
            print("SVT-HEVC patch already applied")
        else:
            patchcmd = "cd %s/ffmpeg;" % (build_dir)
            patchcmd += 'git config user.email "bootstrap@localhost"; '
            patchcmd += 'git config user.name "bootstrap"; '
            patchcmd += 'git am %s/' % (build_dir)
            patchcmd += 'SVT-HEVC/ffmpeg_plugin/'
            patchcmd += '0001-lavc-svt_hevc-add-libsvt-hevc-encoder-wrapper.patch;'
            patchcmd += 'touch svt-hevc-patched'
            os.system(patchcmd)
    return cmd


if __name__ == "__main__":
    main()
