###############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
###############################################################################
"""Build oneVPL-cpu ffmpeg dependencies"""
from io import BytesIO
import sys
import os
import posixpath
import argparse
import subprocess
import shutil
import time
import multiprocessing
import urllib.request
import zipfile
from pathlib import Path
from os import environ
from contextlib import contextmanager

# Component Versions
SVT_HEVC_VERSION = '1.5.1'
SVT_AV1_VERSION = 'v0.8.6'  # v0.8.7 is missing AVC support
DAV1D_VERSION = '0.9.0'
X264_VERSION = 'stable'
FFMPEG_VERSION = 'n4.4'

# Folder this script is in
SCRIPT_PATH = os.path.realpath(
    os.path.join(os.getcwd(), os.path.dirname(__file__)))

# Number of CPU cores to try to use in parallel
CPU_COUNT = multiprocessing.cpu_count()

# Flag indicating if verbose (debug) loging  should be output
VERBOSE = 'VERBOSE' in os.environ
if VERBOSE:
    if os.environ['VERBOSE'] not in ['', '-']:
        # pylint: disable=consider-using-with
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

# indicate if we prefer to clone, or to download archives
PREFER_CLONE = False


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
        # Rarely there is a bit of async delay in filesystem changes.
        # If a user script deleted this folder just before running this
        # script we may need to wait a moment to see the folder created.
        if not os.path.exists(target):
            time.sleep(2)


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
    with subprocess.Popen(exec_cmd, shell=True, env=env) as proc:
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
    with subprocess.Popen(exec_cmd,
                          stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE,
                          universal_newlines=True,
                          shell=True,
                          env=env) as proc:
        result = proc.communicate()
        if log_errors and result[1]:
            sys.stderr.write(result[1])
        return (result[0], result[1], proc.returncode)


class ZipFileWithPermissions(zipfile.ZipFile):
    """ZipFile class that handles file permissions."""
    def _extract_member(self, member, targetpath, pwd):
        """Extract the ZipInfo object 'member' to a physical
           file on the path targetpath, preserving permissions.
        """
        if not isinstance(member, zipfile.ZipInfo):
            member = self.getinfo(member)
        targetpath = super()._extract_member(member, targetpath, pwd)
        attr = member.external_attr >> 16
        if attr != 0:
            os.chmod(targetpath, attr)
        return targetpath


def download_archive(url, path):
    """download an archive and unpack it to a folder"""
    if not os.path.exists(path):
        mkdir(path)
    log_comment(f"Downloading {url}")
    with urllib.request.urlopen(url) as webstream:
        with ZipFileWithPermissions(BytesIO(webstream.read())) as archfileobj:
            log_comment(f"Extracting {url} to {path} as zip file")
            archfileobj.extractall(path)


def main():
    """Main steps to build ffmpeg and dependencies"""

    proj_dir = str(Path(os.path.dirname(os.path.realpath(sys.argv[0]))).parent)
    parser = argparse.ArgumentParser(prog="bootstrap")

    parser.add_argument("--config",
                        '-m',
                        "--build_mode",
                        dest='build_mode',
                        choices=['Release', 'Debug'],
                        default='Release',
                        help='Build mode/configuration')

    parser.add_argument('-gpl',
                        "--use_gpl",
                        "--gpl",
                        dest='use_gpl',
                        action="store_true",
                        help='Use GPL codecs (ex: x264)')

    parser.add_argument(
        '-A',
        "--arch",
        dest='arch',
        choices=['x86_64', 'x86_32'] if os.name == 'nt' else ['x86_64'],
        default='x86_64',
        help='Target Architecture')

    parser.add_argument(
        '--clean',
        '-clean',
        dest='clean',
        action="store_true",
        help='Remove previous build/install dirs before starting')

    parser.add_argument('--validation',
                        dest='validation',
                        action="store_true",
                        help='Build validation binaries')

    # Unused argument for compatibility
    parser.add_argument('--bootstrap',
                        dest='bootstrap',
                        action="store_true",
                        help=argparse.SUPPRESS)

    args = parser.parse_args()

    bootstrap(args.clean, args.use_gpl, args.build_mode, proj_dir, args.arch,
              args.validation)


def make_mingw_path(arch):
    """Create PATH setting for MinGW"""
    fallback_msys_root = os.path.join('C:\\', 'tools', 'msys64')
    if 'MSYS_ROOT' in os.environ:
        msys_root = os.environ['MSYS_ROOT']
        print(f'MSYS_ROOT found: {msys_root}', file=sys.stderr)
    elif os.path.exists(fallback_msys_root):
        msys_root = fallback_msys_root
        print(f'MSYS_ROOT not found using msys at: {msys_root}',
              file=sys.stderr)
    else:
        raise 'MSys not found'
    msys_usr_path = os.path.join(msys_root, 'usr')
    msys_usr_bin_path = os.path.join(msys_usr_path, 'bin')
    win_path = os.path.join('C:\\', 'Windows')
    win_sys_path = os.path.join(win_path, 'System32')
    mingw_path = []
    if arch == 'x86_32':
        mingw_path.append(os.path.join(msys_root, 'mingw32', 'bin'))
        mingw_path.append(
            os.path.join(msys_root, 'mingw32', 'i686-w64-mingw32', 'bin'))
    mingw_path.append(os.path.join(msys_root, 'mingw64', 'bin'))
    mingw_path.extend([
        os.path.join(msys_usr_path, 'local', 'bin'),
        msys_usr_bin_path,
        os.path.join(msys_root, 'bin'),
        win_sys_path,
        win_path,
        os.path.join(win_sys_path, 'Wbem'),
        os.path.join(win_sys_path, 'WindowsPowerShell', 'v1.0'),
        os.path.join(msys_usr_bin_path, 'site_perl'),
        os.path.join(msys_usr_bin_path, 'vendor_perl'),
        os.path.join(msys_usr_bin_path, 'core_perl'),
    ])
    return os.pathsep.join(mingw_path)


def make_git_path(mingw_path):
    """Create PATH setting for Git"""
    git_path = os.environ['PATH']
    # MSYS git does not play with other gits, so use users version if present
    git_location = shutil.which('git')
    if git_location is None:
        git_path = mingw_path
    return git_path


#pylint: disable=too-many-arguments
def bootstrap(clean, use_gpl, build_mode, proj_dir, arch, validation):
    """Bootstrap install"""
    if os.name == 'nt':
        #pylint: disable=global-statement
        global GIT_ENV
        mingw_path = make_mingw_path(arch)
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
        # build_aom_av1_decoder(install_dir)
        if arch == 'x86_64':
            if use_gpl:
                build_gpl_x264_encoder(install_dir)
            build_dav1d_decoder(install_dir)
            build_svt_av1_encoder(install_dir, build_mode)
            build_svt_hevc_encoder(install_dir, build_mode)
        #prepare ffmpeg build
        version = FFMPEG_VERSION
        if os.path.exists(f'FFmpeg-{version}'):
            print("using existing ffmpeg dir")
        else:
            if PREFER_CLONE:
                cmd('git',
                    'clone',
                    '--depth=1',
                    '-b',
                    f'{version}',
                    'https://github.com/FFmpeg/FFmpeg',
                    f'FFmpeg-{version}',
                    xenv=GIT_ENV)
            else:
                download_archive(
                    f"https://github.com/FFmpeg/FFmpeg/archive/refs/tags/{version}.zip",
                    ".")
        with pushd(f'FFmpeg-{version}'):
            configure_opts = []
            configure_opts.extend(
                ffmpeg_configure_opts(install_dir, arch, validation))
            if build_mode == "Debug":
                configure_opts.extend(ffmpeg_debug_configure_opts())
            configure_opts.extend(
                ffmpeg_3rdparty_configure_opts(build_dir, use_gpl))
            # run configure
            cmd('./configure', *configure_opts, shell='bash')
            # build ffmpeg
            cmd('make', '-j', CPU_COUNT)
            cmd('make', 'install')


def build_dav1d_decoder(install_dir):
    """build libdav1d from source"""
    version = DAV1D_VERSION
    if os.path.exists(f'dav1d-{version}'):
        print("using existing david decoder dir")
        return
    if PREFER_CLONE:
        cmd('git',
            'clone',
            '--depth=1',
            '-b',
            f'{version}',
            'https://code.videolan.org/videolan/dav1d.git',
            f'dav1d-{version}',
            xenv=GIT_ENV)
    else:
        download_archive(
            f"https://code.videolan.org/videolan/dav1d/-/archive/{version}/dav1d-{version}.zip",
            ".")
    with pushd(f'dav1d-{version}'):
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
    version = SVT_HEVC_VERSION
    if os.path.exists(f'SVT-HEVC-{version}'):
        print("using existing SVT-HEVC encoder dir")
        return
    if PREFER_CLONE:
        cmd('git',
            'clone',
            '--depth=1',
            '-b',
            f'v{version}',
            'https://github.com/OpenVisualCloud/SVT-HEVC.git',
            f'SVT-HEVC-{version}',
            xenv=GIT_ENV)
    else:
        download_archive(
            f"https://github.com/OpenVisualCloud/SVT-HEVC/archive/refs/tags/v{version}.zip",
            ".")
    with pushd(f'SVT-HEVC-{version}'):
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
                '-DCMAKE_INSTALL_LIBDIR=lib', '-DBUILD_SHARED_LIBS=off',
                '-DBUILD_APP=off')
            cmd('make', '-j', CPU_COUNT)
            cmd('make', 'install')


def build_svt_av1_encoder(install_dir, build_mode):
    """build SVT AV1 encoder from source"""
    version = SVT_AV1_VERSION
    if os.path.exists(f'SVT-AV1-{version}'):
        print("using existing SVT-AV1 encoder dir")
        return
    if PREFER_CLONE:
        cmd('git',
            'clone',
            '--depth=1',
            '-b',
            f'{version}',
            'https://gitlab.com/AOMediaCodec/SVT-AV1',
            f'SVT-AV1-{version}',
            xenv=GIT_ENV)
    else:
        download_archive(
            f"https://gitlab.com/AOMediaCodec/SVT-AV1/-/archive/{version}/SVT-AV1-{version}.zip",
            ".")
    with pushd(f'SVT-AV1-{version}'):
        if build_mode == 'Debug':
            replace(
                os.path.join('Source', 'Lib', 'Common', 'Codec', 'EbMalloc.h'),
                '#define DEBUG_MEMORY_USAGE', '#undef DEBUG_MEMORY_USAGE')
        mkdir('release')
        with pushd('release'):
            cmd('cmake', '..', '-GUnix Makefiles',
                f'-DCMAKE_BUILD_TYPE={build_mode}',
                f'-DCMAKE_INSTALL_PREFIX={os.path.join(install_dir, "")}',
                '-DCMAKE_INSTALL_LIBDIR=lib', '-DBUILD_SHARED_LIBS=off',
                '-DBUILD_APPS=off',
                '-DBUILD_DEC=off' if os.name != 'nt' else '',
                '-DCMAKE_C_FLAGS=$(CMAKE_C_FLAGS) -DSVT_LOG_QUIET=1')
            cmd('make', '-j', CPU_COUNT)
            cmd('make', 'install')


def build_gpl_x264_encoder(install_dir):
    """build x264 encoder from source"""
    version = X264_VERSION
    posix_install_dir = to_posix_path(install_dir)
    if os.path.exists(f'x264-{version}'):
        print("using existing x264 encoder dir")
        return
    if PREFER_CLONE:
        cmd('git',
            'clone',
            '--depth=1',
            '-b',
            f'{version}',
            'https://code.videolan.org/videolan/x264.git',
            f'x264-{version}',
            xenv=GIT_ENV)
    else:
        download_archive(
            f"https://code.videolan.org/videolan/x264/-/archive/{version}/x264-{version}.zip",
            ".")
    with pushd(f'x264-{version}'):
        cmd('./configure',
            f'--prefix={posix_install_dir}',
            '--enable-static',
            '--enable-pic',
            shell='bash')
        cmd('make', '-j', CPU_COUNT)
        cmd('make', 'install')


def ffmpeg_configure_opts(install_dir, arch, validation):
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
            '--enable-filter=testsrc2',
        ])
        if arch == 'x86_64':
            result.append('--arch=x86_64')
            result.append('--target-os=mingw64')
        elif arch == 'x86_32':
            result.append('--arch=x86_32')
            result.append('--target-os=mingw32')
        else:
            raise Exception(f'Unknown architecture {arch}')
    else:
        if validation:
            result.extend([
                '--enable-filter=testsrc2', '--disable-vaapi',
                '--disable-cuda-llvm'
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


def ffmpeg_3rdparty_configure_opts(build_dir, use_gpl):
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
    if use_gpl:
        if "x264" in pkg_list:
            print("x264 encoder found")
            result.extend([
                '--enable-gpl', '--enable-libx264', '--enable-encoder=libx264'
            ])
    if "SvtAv1Enc" in pkg_list:
        print("SVT-AV1 encoder found")
        result.extend(['--enable-libsvtav1', '--enable-encoder=libsvtav1'])
    if "SvtHevcEnc" in pkg_list:
        print("SVT-HEVC encoder found")
        result.extend(['--enable-libsvthevc', '--enable-encoder=libsvt_hevc'])
        if os.path.isfile("svt-hevc-patched"):
            print("SVT-HEVC patch already applied")
        else:
            if not PREFER_CLONE:
                # make this folder a git repo so we can use "git am" to apply patches
                cmd('git', 'init', xenv=GIT_ENV)
                cmd('git', 'add', '.', xenv=GIT_ENV)
                cmd('git',
                    '-c',
                    'user.name=bootstrap',
                    '-c',
                    'user.email=bootstrap@localhost',
                    'commit',
                    '-m',
                    'Import',
                    xenv=GIT_ENV)
            patch = 'n4.4-0001-lavc-svt_hevc-add-libsvt-hevc-encoder-wrapper.patch'
            cmd('git',
                '-c',
                'user.name=bootstrap',
                '-c',
                'user.email=bootstrap@localhost',
                'am',
                os.path.join(build_dir, f'SVT-HEVC-{SVT_HEVC_VERSION}',
                             'ffmpeg_plugin', patch),
                xenv=GIT_ENV)
            cmd('touch', 'svt-hevc-patched')
    return result


if __name__ == "__main__":
    main()
