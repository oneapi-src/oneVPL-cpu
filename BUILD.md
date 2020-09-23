# Configuring the build environment

This section describes the steps to configure the build environment. It extends the high-level build information described in the README.

The basic steps are:

1. Configure the environment

2. Build dependencies using script/bootstrap

3. Build the code using the script/build 

After you have configured the environment you can follow the basic build
steps in the README. Note that if dependencies are not modified, bootstrapping
only needs to be done once. The build script will look in the location specified
by the `VPL_BUILD_DEPENDENCIES` environment variable.


## Configure the Windows Environment

Install common developer tools
Note: If behind a proxy, make sure your shell is configured to allow external downloads and git clones.  For more information please see the documentation for your local network environment.

* [Microsoft Visual Studio](https://visualstudio.microsoft.com/) Visual Studio 2017 or newer

* [CMake](https://cmake.org/) 

* [Python3](https://www.python.org/)

* [git](https://git-scm.com/)


### Install MSYS2 based tools

This guide provides a MSYS2 setup based on 
[chocolatey](http://chocolatey.org). Some of the paths to mingw may be different if another method to install MSYS2 is used.

Install MSYS2 from a cmd or powershell with admin permissions using the following command:

```bash
       choco install msys2
```

Start a mingw64 shell from cmd or powershell with regular permissions.  

```bash
       c:\tools\msys64\msys2.exe
```

Note: This path is from choco install.  If another install method is used
the path to the shell may be different.

Get packages required by build

```bash
       pacman -Syu
       pacman --needed -Sy mingw-w64-x86_64-toolchain base-devel yasm nasm mingw-w64-x86_64-cmake git python-pip mingw-w64-x86_64-meson mingw-w64-x86_64-ninja
```
Note: While the msys2 shell is used for this step in initial setup it is
not otherwise supported as a build or run environment.

## Configuring the Ubuntu Environment

From a shell as root, enter the following:

```bash
       apt update
       apt-get update && apt-get install -y --no-install-recommends \
       build-essential git pkg-config yasm nasm cmake python3 \
       python3-setuptools python3-pip
       pip3 install -U wheel --user 
       pip3 install meson ninja
```


## Docker

See docker/README.md for more information.
