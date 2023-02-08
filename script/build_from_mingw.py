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
import shutil
import time
import posixpath
import multiprocessing
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


def main():
    """Main steps to build ffmpeg and dependencies"""
    parser = argparse.ArgumentParser(prog="build_openh264")

    parser.add_argument("--config",
                        '-m',
                        "--build_mode",
                        dest='build_mode',
                        choices=['Release', 'Debug', 'Trace'],
                        default='Release',
                        help='Build mode/configuration')

    parser.add_argument('-warning_as_error',
                        "--use_warning_as_error",
                        "--warning_as_error",
                        dest='use_warning_as_error',
                        action="store_true",
                        help='Use warning as error')

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

    parser.add_argument('--oneapi_layout',
                        dest='oneapi_layout',
                        action="store_true",
                        help='Use oneAPI install directory layout')

    args = parser.parse_args()

    build_opts = []
    for opts in sys.argv[1:]:
        build_opts.append(opts)

    launch_builder(build_opts, args.arch)


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


#pylint: disable=too-many-arguments,too-many-branches,too-many-statements
def launch_builder(build_opts, arch):
    """launch build script"""
    #pylint: disable=global-statement
    global GIT_ENV
    mingw_path = make_mingw_path(arch)
    GIT_ENV = {'PATH': make_git_path(mingw_path)}
    set_env('PATH', mingw_path)

    set_env('CMAKE_GENERATOR', "Unix Makefiles")

    if "VPL_INSTALL_DIR" in os.environ:
        vpl_install_dir = environ.get("VPL_INSTALL_DIR")
        set_env('VPL_INSTALL_DIR', vpl_install_dir)
    else:
        raise 'VPL_INSTALL_DIR is not set'

    cmd(to_posix_path(os.path.join(SCRIPT_PATH, 'build')),
        *build_opts,
        shell='bash')


if __name__ == "__main__":
    main()
