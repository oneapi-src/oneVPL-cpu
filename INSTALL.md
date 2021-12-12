# Build/install oneAPI Video Processing Library CPU Implementation from Sources

## Prerequisites

- Make sure your shell is configured to allow downloads and git clones from the
  public Internet.

- Obtain and install the oneVPL base package from the
[oneVPL repository](https://github.com/oneapi-src/oneVPL).

## Build and Install on Linux*

1. Install the required build tools:

        sudo apt-get update && sudo apt-get install -y --no-install-recommends \
            build-essential git pkg-config yasm nasm cmake python3 \
            python3-setuptools python3-pip
        pip3 install -U wheel --user
        pip3 install meson ninja

2. Configure, build, and install the CPU reference implementation:

        export VPL_INSTALL_DIR=<vpl-install-location>
        source script/bootstrap
        script/build
        script/install

## Build and Install on Windows*

1. Install the following common developer tools:

    - [Microsoft Visual Studio*](https://visualstudio.microsoft.com/), 2017 or newer
    - [CMake*](https://cmake.org/)
    - [Python* 3](https://www.python.org/)
    - [Git*](https://git-scm.com/)
    - [Chocolatey*](http://chocolatey.org)

2. Install MSYS2 based tools.

    1. Install MSYS2 from a cmd or powershell with admin permissions:

        ```
        choco install msys2
        ```

    2. Start a mingw64 shell from cmd or powershell with regular permissions:

        ```
        C:\tools\msys64\msys2.exe
        ```

        ---

        **NOTE** This path is for default choco install. If another install method is
        used the path to the shell may be different.

        ---

    3. Get packages required by build using the mingw64 shell:

            pacman -Syu
            pacman --needed -Sy mingw-w64-x86_64-toolchain base-devel yasm nasm
            pacman --needed -Sy mingw-w64-x86_64-cmake git python-pip
            pacman --needed -Sy mingw-w64-x86_64-meson mingw-w64-x86_64-ninja

            # For 32-bit builds you will also need a 32-bit toolchain
            pacman --needed -Sy mingw-w64-i686-toolchain
        ---

        **NOTE** While the msys2 shell is used in initial setup, it is not otherwise
        supported as a build or run environment.

        ---

3. Configure, build, and install CPU reference implementation:

        set VPL_INSTALL_DIR=<vpl-install-location>
        script\bootstrap
        script\build
        script\install


## Optionally Enable H.264 Encode

Add `gpl` to the build commands to enable H.264 encode capability.

For Linux:

```
source script/bootstrap gpl
script/build gpl
script/install
```

For Windows:

```
script\bootstrap gpl
script\build gpl
script\install
```

---

**Note** The GPL license will apply to a build generated with `gpl`.

---

## Additional build options

The bootstrap and build scripts accept additional options.  See the -h flag for more details.

To build in debug mode in Linux:
```
 source script/bootstrap --config Debug
 script/build --config Debug
 script/install
```

For 32 bit builds on Windows:
```
 script/bootstrap -A x86-32
 script/build -A x86-32
 script/install
```

Flags can be used together

