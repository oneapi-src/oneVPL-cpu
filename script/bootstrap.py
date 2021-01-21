###############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
###############################################################################
"""Build oneVPL-cpu ffmpeg dependencies"""
import sys
import os
import posixpath
import argparse
import subprocess
import shutil
import time
import multiprocessing
from pathlib import Path
from os import environ
from contextlib import contextmanager

# Folder this script is in
SCRIPT_PATH = os.path.realpath(
    os.path.join(os.getcwd(), os.path.dirname(__file__)))

# Number of CPU cores to try to use in parallel
CPU_COUNT = multiprocessing.cpu_count()

# Flag indicating if verbose (debug) loging  should be output
VERBOSE = 'VERBOSE' in os.environ
if VERBOSE:
    if os.environ['VERBOSE']:
        VERBOSE_FILE = open(os.environ['VERBOSE'], 'w')
    else:
        VERBOSE_FILE = sys.stdout
    if os.name == 'nt':
        VERBOSE_CMD = '::'
        VERBOSE_CMT = '@REM'
    else:
        VERBOSE_CMD = '# $'
        VERBOSE_CMT = '#'

# Optional dictionary with environment options for Git
# mostly used to set an alternate PATH
GIT_ENV = None


def _escape_cmd_arg(arg):
    """quote/escape and argument for a command line call so that it can
    be safely used even if it has special charaters"""
    arg = str(arg)
    if ' ' in arg or '"' in arg:
        return '"' + arg.replace('"', '""') + '"'
    return arg


def log(message):
    """Log activity"""
    if VERBOSE:
        VERBOSE_FILE.write(f"{VERBOSE_CMD} {message}\n")
        VERBOSE_FILE.flush()


def log_comment(message):
    """Log a comment"""
    if VERBOSE:
        VERBOSE_FILE.write(f"{VERBOSE_CMT} {message}\n")
        VERBOSE_FILE.flush()


def to_posix_path(path):
    """convert path to posix
    On Windows this includes adjusting it based on MinGW drive naming
    """
    if os.name != 'nt':
        return path
    if not path:
        return path
    parts = path.split('\\')
    if len(parts[0]) == 2 and parts[0].endswith(":"):
        parts[0] = "/" + parts[0][:-1].lower()
    return posixpath.join(*parts)


def set_env(name, value):
    """Set environment variable"""
    if os.name == 'nt':
        log(f'set {name}={value}')
    else:
        log(f'export {name}="{value}"')
    os.environ[name] = value


def replace(target, old_str, new_str):
    """replace text in a file"""
    log_comment(f'replace "{old_str}" with "{new_str}" in {target}')
    if os.name == 'nt':
        log(f'powershell -Command "(gc {target}) -replace \'{old_str}\', \'{new_str}\' '
            + f'| Out-File -encoding utf8 {target}"')
    else:
        log(f'sed -i \'s/{old_str}/{new_str}/\' {target}')
    with open(target, "r") as file_obj:
        content = file_obj.read()
    content = content.replace(old_str, new_str)
    with open(target, "w") as file_obj:
        file_obj.write(content)


@contextmanager
def pushd(*dst):
    """change working directory"""
    cur_dir = os.getcwd()
    dest = os.path.join(cur_dir, *dst)
    os.chdir(dest)
    log(f'pushd {dest}')
    try:
        yield
    finally:
        log('popd')
        log_comment(f' -> {cur_dir}')
        os.chdir(cur_dir)


#pylint: disable=invalid-name
def rm(target):
    """delete a file or folder"""
    if os.path.exists(target):
        # Delete sometimes fails if done immediately, timeout
        # is not great, but allows filesystem settings to stabilize.
        timeout = time.time() + 10
        while time.time() < timeout:
            try:
                if os.path.isfile(target):
                    if os.name == 'nt':
                        log(f'del {target}')
                    else:
                        log(f'rm {target}')
                    os.remove(target)
                    break
                if os.path.isdir(target):
                    if os.name == 'nt':
                        log(f'rd /s /q {target}')
                    else:
                        log(f'rm -rf {target}')
                    shutil.rmtree(target)
                    break
            except PermissionError:
                time.sleep(1)


def mkdir(target):
    """make a folder"""
    if target and not os.path.exists(target):
        if os.name == 'nt':
            log(f'md {target}')
        else:
            log(f'mkdir -p {target}')
        os.makedirs(target)


def join_command(command):
    """Join a series or parameters into a command, escaping if needed"""
    return ' '.join([_escape_cmd_arg(argument) for argument in command])


def cmd(*args, shell=None, no_throw=False, env=None, xenv=None):
    """Run a command"""
    if len(args) == 1:
        command = args[0]
    else:
        command = join_command(args)
    if env is not None:
        log_comment('Using custom environment for next command')
    if xenv is not None:
        if env is None:
            env = os.environ.copy()
        env.update(xenv)
        for name in xenv:
            log_comment(f'Using "{name}={xenv[name]}" for next command')
    exec_cmd = command

    if shell is None and os.name != 'nt':
        shell = 'bash'

    if shell == 'bash':
        if os.name == 'nt':
            # In Windows bash is unexpected so we will record using it
            # as part of the verbose log
            command = f"bash -c '{command}'"
            exec_cmd = command
        else:
            # outside Windows we explicitly use bash, but we don't need
            # to worry about letting people know we are using it.
            exec_cmd = f"exec bash -c '{command}'"

    log(f'{command}')
    proc = subprocess.Popen(exec_cmd, shell=True, env=env)
    proc.communicate()
    if not no_throw and proc.returncode != 0:
        raise Exception(f"Error running command: {command}")
    return proc.returncode


def capture_cmd(*args, shell=None, log_errors=True, env=None, xenv=None):
    """Run a command and capture the output"""
    if len(args) == 1:
        command = args[0]
    else:
        command = join_command(args)
    if env is not None:
        log_comment('Using custom environment for next command')
    if xenv is not None:
        if env is None:
            env = os.environ.copy()
        env.update(xenv)
        for name in xenv:
            log_comment(f'Using "{name}={xenv[name]}" for next command')
    exec_cmd = command

    if shell is None and os.name != 'nt':
        shell = 'bash'

    if shell == 'bash':
        if os.name == 'nt':
            # In Windows bash is unexpected so we will record using it
            # as part of the verbose log
            command = f"bash -c '{command}'"
            exec_cmd = command
        else:
            # outside Windows we explicitly use bash, but we don't need
            # to worry about letting people know we are using it.
            exec_cmd = f"exec bash -c '{command}'"

    log(f'{command}')
    proc = subprocess.Popen(exec_cmd,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE,
                            universal_newlines=True,
                            shell=True,
                            env=env)
    result = proc.communicate()
    if log_errors and result[1]:
        sys.stderr.write(result[1])
    return (result[0], result[1], proc.returncode)


def main():
    """Main steps to build ffmpeg and dependencies"""

    proj_dir = str(Path(os.path.dirname(os.path.realpath(sys.argv[0]))).parent)
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

    bootstrap(args.clean, args.use_gpl, args.build_mode, proj_dir)


def make_mingw_path():
    """Create PATH setting for MinGW"""
    if 'MSYS_ROOT' in os.environ:
        msys_root = os.environ['MSYS_ROOT']
        print(f'MSYS_ROOT found: {msys_root}', file=sys.stderr)
    elif os.path.exists('C:\\tools\\msys64'):
        msys_root = 'C:\\tools\\msys64'
        print(f'MSYS_ROOT not found using msys at: {msys_root}',
              file=sys.stderr)
    else:
        raise 'MSys not found'
    msys_usr_path = f'{msys_root}\\usr'
    msys_usr_bin_path = f'{msys_usr_path}\\bin'
    win_path = 'c:\\Windows'
    win_sys_path = f'{win_path}\\System32'
    mingw_path = os.pathsep.join([
        f'{msys_root}\\mingw64\\bin', f'{msys_usr_path}\\local\\bin',
        f'{msys_usr_bin_path}', f'{msys_root}\\bin', f'{win_sys_path}',
        f'{win_path}', f'{win_sys_path}\\Wbem',
        f'{win_sys_path}\\WindowsPowerShell\\v1.0\\',
        f'{msys_usr_bin_path}\\site_perl', f'{msys_usr_bin_path}\\vendor_perl',
        f'{msys_usr_bin_path}\\core_perl'
    ])
    return mingw_path


def make_git_path(mingw_path):
    """Create PATH setting for Git"""
    git_path = os.environ['PATH']
    # MSYS git does not play with other gits, so use users version if present
    git_location = shutil.which('git')
    if git_location is None:
        git_path = mingw_path
    return git_path


def bootstrap(clean, use_gpl, build_mode, proj_dir):
    """Bootstrap install"""
    if os.name == 'nt':
        #pylint: disable=global-statement
        global GIT_ENV
        mingw_path = make_mingw_path()
        GIT_ENV = {'PATH': make_git_path(mingw_path)}
        # Don't update PATH with MinGW until we have figured out Git path
        set_env('PATH', mingw_path)
    build_dir = os.path.join(proj_dir, '_extbuild')
    install_dir = os.path.join(proj_dir, '_deps')
    if "VPL_BUILD_DEPENDENCIES" in os.environ:
        install_dir = environ.get("VPL_BUILD_DEPENDENCIES")
    else:
        set_env('VPL_BUILD_DEPENDENCIES', install_dir)
    pkg_config_path = [os.path.join(install_dir, "lib", "pkgconfig")]
    if 'PKG_CONFIG_PATH' in os.environ:
        pkg_config_path.append(os.environ['PKG_CONFIG_PATH'])
    set_env('PKG_CONFIG_PATH', os.pathsep.join(pkg_config_path))

    if clean:
        rm(build_dir)
        rm(install_dir)
    mkdir(build_dir)
    mkdir(install_dir)
    with pushd(build_dir):
        #build dependencies
        if use_gpl:
            build_gpl_x264_encoder(install_dir)
        # build_aom_av1_decoder(install_dir)
        build_dav1d_decoder(install_dir)
        build_svt_av1_encoder(install_dir, build_mode)
        build_svt_hevc_encoder(install_dir, build_mode)
        #prepare ffmpeg build
        clone_ffmpeg()
        with pushd('ffmpeg'):
            configure_opts = []
            configure_opts.extend(ffmpeg_configure_opts(install_dir))
            if build_mode == "Debug":
                configure_opts.extend(ffmpeg_debug_configure_opts())
            configure_opts.extend(ffmpeg_3rdparty_configure_opts(build_dir))
            # run configure
            cmd('./configure', *configure_opts, shell='bash')
            # build ffmpeg
            cmd('make', '-j', CPU_COUNT)
            cmd('make', 'install')


def build_aom_av1_decoder(install_dir):
    """build libaom from source"""
    if os.path.exists('aom'):
        print("using existing aom av1 decoder dir")
        return
    cmd('git', 'clone'
        '--depth=1',
        '-b',
        'master',
        'https://aomedia.googlesource.com/aom',
        xenv=GIT_ENV)
    with pushd('aom'):
        mkdir('_build')
        with pushd('_build'):
            cmd('cmake', '..', f'-DCMAKE_INSTALL_PREFIX={install_dir}',
                '-DCONFIG_AV1_ENCODER=0', '-DBUILD_SHARED_LIBS=0')
            cmd('make', '-j', CPU_COUNT)
            cmd('make', 'install')


def build_dav1d_decoder(install_dir):
    """build libdav1d from source"""
    if os.path.exists('dav1d'):
        print("using existing david decoder dir")
        return
    cmd('git',
        'clone',
        '--depth=1',
        '-b',
        '0.7.0',
        'https://code.videolan.org/videolan/dav1d.git',
        xenv=GIT_ENV)
    with pushd('dav1d'):
        cmd('meson', 'build', '--prefix', os.path.join(install_dir,
                                                       ''), '--libdir',
            os.path.join(install_dir, 'lib'), '--buildtype', 'release',
            '--default-library=static', '-Denable_avx512=false')
        cmd('ninja', '-C', 'build')
        with pushd('build'):
            cmd('ninja', 'install')
            if os.name != 'nt':
                if os.path.isfile(
                        os.path.join(install_dir, 'lib', 'pkgconfig',
                                     'dav1d_edited')):
                    print("dav1d.pc already edited")
                else:
                    with pushd(install_dir, 'lib', 'pkgconfig'):
                        replace('dav1d.pc', '-ldav1d', '-ldav1d -pthread -ldl')
                        cmd('touch', 'dav1d_edited')


def build_svt_hevc_encoder(install_dir, build_mode):
    """build SVT HEVC encoder from source"""
    if os.path.exists('SVT-HEVC'):
        print("using existing SVT-HEVC encoder dir")
        return
    cmd('git',
        'clone',
        '--depth=1',
        '-b',
        'v1.5.0',
        'https://github.com/OpenVisualCloud/SVT-HEVC.git',
        xenv=GIT_ENV)
    with pushd('SVT-HEVC'):
        if build_mode == 'Debug':
            replace(os.path.join('Source', 'Lib', 'Codec', 'EbMalloc.h'),
                    '#define DEBUG_MEMORY_USAGE', '#undef DEBUG_MEMORY_USAGE')
        replace(os.path.join('Source', 'Lib', 'Codec', 'EbDefinitions.h'),
                '#define LIB_PRINTF_ENABLE                1',
                '#define LIB_PRINTF_ENABLE                0')
        mkdir('release')
        with pushd('release'):
            cmd('cmake', '..', '-GUnix Makefiles',
                f'-DCMAKE_BUILD_TYPE={build_mode}',
                f'-DCMAKE_INSTALL_PREFIX={os.path.join(install_dir, "")}',
                '-DBUILD_SHARED_LIBS=off', '-DBUILD_APP=off')
            cmd('make', '-j', CPU_COUNT)
            cmd('make', 'install')
            if os.name != 'nt':
                if os.path.exists(os.path.join(install_dir, 'lib64')):
                    cmd('cp', f'{install_dir}/lib64/*', f'{install_dir}/lib')
                    mkdir(os.path.join(install_dir, 'lib', 'pkgconfig'))
                    cmd('cp', f'{install_dir}/lib64/pkgconfig/*',
                        f'{install_dir}/lib/pkgconfig')
                    replace(f'{install_dir}/lib/pkgconfig/SvtHevcEnc.pc',
                            'lib64', 'lib')


def build_svt_av1_encoder(install_dir, build_mode):
    """build SVT AV1 encoder from source"""
    if os.path.exists('SVT-AV1'):
        print("using existing SVT-AV1 encoder dir")
        return

    cmd('git',
        'clone',
        '--depth=1',
        '-b',
        'v0.8.4',
        'https://github.com/AOMediaCodec/SVT-AV1',
        xenv=GIT_ENV)
    with pushd('SVT-AV1'):
        if build_mode == 'Debug':
            replace(
                os.path.join('Source', 'Lib', 'Common', 'Codec', 'EbMalloc.h'),
                '#define DEBUG_MEMORY_USAGE', '#undef DEBUG_MEMORY_USAGE')
        mkdir('release')
        with pushd('release'):
            cmd('cmake', '..', '-GUnix Makefiles',
                f'-DCMAKE_BUILD_TYPE={build_mode}',
                f'-DCMAKE_INSTALL_PREFIX={os.path.join(install_dir, "")}',
                '-DBUILD_SHARED_LIBS=off', '-DBUILD_APPS=off',
                '-DBUILD_DEC=off' if os.name != 'nt' else '',
                '-DCMAKE_C_FLAGS=$(CMAKE_C_FLAGS) -DSVT_LOG_QUIET=1')
            cmd('make', '-j', CPU_COUNT)
            cmd('make', 'install')
            if os.name != 'nt':
                if os.path.isdir(f"{install_dir}/lib64"):
                    cmd('cp', f'{install_dir}/lib64/*', f'{install_dir}/lib')
                    mkdir(os.path.join(install_dir, 'lib', 'pkgconfig'))
                    cmd('cp', f'{install_dir}/lib64/pkgconfig/*',
                        f'{install_dir}/lib/pkgconfig')
                    replace(f'{install_dir}/lib/pkgconfig/SvtAv1Enc.pc',
                            'lib64', 'lib')


def build_gpl_x264_encoder(install_dir):
    """build x264 encoder from source"""
    posix_install_dir = to_posix_path(install_dir)
    if os.path.exists('x264'):
        print("using existing x264 encoder dir")
        return
    cmd('git',
        'clone',
        '--depth=1',
        '-b',
        'stable',
        'https://code.videolan.org/videolan/x264.git',
        xenv=GIT_ENV)
    with pushd('x264'):
        cmd('./configure',
            f'--prefix={posix_install_dir}',
            '--enable-static',
            '--enable-pic',
            shell='bash')
        cmd('make', '-j', CPU_COUNT)
        cmd('make', 'install')


def clone_ffmpeg():
    """get ffmpeg library source"""
    if os.path.exists('ffmpeg'):
        print("using existing ffmpeg dir")
    else:
        cmd('git',
            'clone',
            '--depth=1',
            '-b',
            'n4.3.1',
            'https://github.com/FFmpeg/FFmpeg',
            'ffmpeg',
            xenv=GIT_ENV)


def ffmpeg_configure_opts(install_dir):
    """configure options for ffmpeg build"""
    posix_install_dir = to_posix_path(install_dir)
    result = [
        f'--prefix={posix_install_dir}',
        '--enable-static',
        '--disable-shared',
        '--enable-pic',
        '--disable-everything',
        '--disable-network',
        '--disable-doc',
        '--disable-manpages',
        '--disable-hwaccels',
        '--disable-appkit',
        '--disable-alsa',
        '--disable-avfoundation',
        '--disable-iconv',
        '--disable-lzma',
        '--disable-securetransport',
        '--disable-xlib',
        '--disable-zlib',
        '--disable-amf',
        '--disable-audiotoolbox',
        '--disable-cuvid',
        '--disable-d3d11va',
        '--disable-dxva2',
        '--disable-nvdec',
        '--disable-nvenc',
        '--disable-v4l2-m2m',
        '--disable-videotoolbox',
        '--disable-sdl2',
        '--enable-indev=lavfi',
        '--enable-protocol=file',
        '--enable-bsf=h264_mp4toannexb',
        '--enable-bsf=hevc_mp4toannexb',
        '--enable-bsf=mjpeg2jpeg',
        '--enable-bsf=mjpega_dump_header',
        '--enable-decoder=rawvideo',
        '--enable-encoder=rawvideo',
        '--enable-demuxer=rawvideo',
        '--enable-demuxer=mjpeg',
        '--enable-muxer=rawvideo',
        '--enable-muxer=null',
        '--enable-decoder=wrapped_avframe',
        '--enable-encoder=wrapped_avframe',
        '--enable-muxer=h264',
        '--enable-muxer=mpeg2video',
        '--enable-muxer=mjpeg',
        '--enable-muxer=hevc',
        '--enable-muxer=ivf',
        '--enable-filter=testsrc',
        '--enable-demuxer=image2',
        '--enable-muxer=image2',
        '--enable-filter=yuvtestsrc',
        '--enable-filter=rgbtestsrc',
        '--enable-decoder=h264',
        '--enable-parser=h264',
        '--enable-demuxer=h264',
        '--enable-decoder=hevc',
        '--enable-demuxer=hevc',
        '--enable-demuxer=ivf',
        '--enable-parser=hevc',
        '--enable-parser=mjpeg',
        '--enable-parser=av1',
        '--enable-decoder=mpeg2video',
        '--enable-encoder=mpeg2video',
        '--enable-encoder=mjpeg',
        '--enable-decoder=mjpeg',
        '--enable-filter=overlay',
        '--enable-filter=crop',
        '--enable-filter=scale',
        '--enable-filter=drawbox',
        '--enable-filter=psnr',
        '--enable-filter=split',
        '--enable-filter=select',
        '--enable-filter=concat',
        '--enable-filter=ssim',
    ]
    if os.name == 'nt':
        result.extend([
            '--extra-cflags=-fPIC',
            '--extra-ldflags=-fPIC',
            '--arch=x86_64',
            '--target-os=mingw64',
            '--enable-filter=testsrc2',
        ])
    else:
        result.extend([
            '--disable-vaapi', '--disable-cuda-llvm', '--disable-avdevice',
            '--disable-swresample'
        ])
    return result


def ffmpeg_debug_configure_opts():
    """add ffmpeg configure debug flags if requested"""
    return [
        '--disable-optimizations', '--extra-cflags=-Og',
        '--extra-cflags=-fno-omit-frame-pointer', '--enable-debug=3',
        '--extra-cflags=-fno-inline'
    ]


def ffmpeg_3rdparty_configure_opts(build_dir):
    """update ffmpeg configure command line based on packages findable
    by pkg-config"""
    result = []
    pkg_list = capture_cmd("pkg-config", "--list-all")[0]
    if "aom" in pkg_list:
        print("aom decoder found")
        result.extend(['--enable-libaom', '--enable-decoder=libaom_av1'])
    if "dav1d" in pkg_list:
        print("dav1d decoder found")
        result.extend(['--enable-libdav1d', '--enable-decoder=libdav1d'])
    if "x264" in pkg_list:
        print("x264 encoder found")
        result.extend(
            ['--enable-gpl', '--enable-libx264', '--enable-encoder=libx264'])
    if "SvtAv1Enc" in pkg_list:
        print("SVT-AV1 encoder found")
        result.extend(['--enable-libsvtav1', '--enable-encoder=libsvt_av1'])
        if os.path.isfile("svt-av1-patched"):
            print("SVT-AV1 patch already applied")
        else:
            cmd('git',
                'config',
                'user.email',
                'bootstrap@localhost',
                xenv=GIT_ENV)
            cmd('git', 'config', 'user.name', 'bootstrap', xenv=GIT_ENV)
            patch = '0001-Add-ability-for-ffmpeg-to-run-svt-av1.patch'
            cmd('git',
                'am',
                os.path.join(build_dir, 'SVT-AV1', 'ffmpeg_plugin', patch),
                xenv=GIT_ENV)
            cmd('touch', 'svt-av1-patched')
    if "SvtHevcEnc" in pkg_list:
        print("SVT-HEVC encoder found")
        result.extend(['--enable-libsvthevc', '--enable-encoder=libsvt_hevc'])
        if os.path.isfile("svt-hevc-patched"):
            print("SVT-HEVC patch already applied")
        else:
            cmd('git',
                'config',
                'user.email',
                'bootstrap@localhost',
                xenv=GIT_ENV)
            cmd('git', 'config', 'user.name', 'bootstrap', xenv=GIT_ENV)
            patch = '0001-lavc-svt_hevc-add-libsvt-hevc-encoder-wrapper.patch'
            cmd('git',
                'am',
                os.path.join(build_dir, 'SVT-HEVC', 'ffmpeg_plugin', patch),
                xenv=GIT_ENV)
            cmd('touch', 'svt-hevc-patched')
    return result


if __name__ == "__main__":
    main()
