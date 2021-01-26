# oneAPI Video Processing Library CPU Reference Implementation

The oneAPI Video Processing Library (oneVPL) provides a single video processing
API for encode, decode, and video processing that works across a wide range of
accelerators.

This repository contains the CPU reference implementation of the specification,
which includes the following features:

- H.265/HEVC, H.264/AVC, and MJPEG decode (using libavcodec)
- AV1 decode (using libdav1d and libavcodec)
- MJPEG encode (using libavcodec)
- H.265/HEVC and AV1 encode (using SVT-HEVC, SVT-AV1, and libavcodec)
- H.264/AVC encode (using x264 and libavcodec)
- VPP - Crop, Resize, CSC (using libavfilter/ FFmpeg filters)

Note: H.265/HEVC and AV1 are not avalible in 32-bit builds.

---

**NOTE** Use of this implementation requires installation of the loader provided
as part of the [oneVPL base repository](https://github.com/oneapi-src/oneVPL).

---
This project is part of the larger [oneAPI](https://www.oneapi.com/) project.
See the [oneAPI Specification](https://spec.oneapi.com) and
[oneVPL Specification](https://spec.oneapi.com/versions/latest/elements/oneVPL/source/index.html)
for more information.

## Build and Installation

### Prerequisites

- Make sure your shell is configured to allow downloads and git clones from the
  public Internet.

- Obtain and install the oneVPL base package from the
[oneVPL repository](https://github.com/oneapi-src/oneVPL).

### Build and Install on Linux*

1. Install the required build tools:

        sudo apt-get update && sudo apt-get install -y --no-install-recommends \
            build-essential git pkg-config yasm nasm cmake python3 \
            python3-setuptools python3-pip
        sudo pip3 install -U wheel --user
        sudo pip3 install meson ninja

2. Configure, build, and install the CPU reference implementation:

        export VPL_INSTALL_DIR=<vpl-install-location>
        source script/bootstrap
        script/build
        script/install

### Build and Install on Windows*

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


### Optionally Enable H.264 Encode

Add `gpl` to the build commands to enable H.264 encode capability.

For Linux:

```
script/bootstrap gpl
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

## Usage

### Configure the Environment

If you are not using the shell session that you used to build and install, you
need to set up the environment.

For Linux:
```
source <vpl-install-location>/env/vars.sh
```

For Windows:
```
<vpl-install-location>\env\vars.bat
```

### Run the Command Line Tools

The oneVPL build that you installed as a prerequisite includes command line
tools that use installed implementations. You can use these command line tools
to process video from the command line using the CPU reference implementation.

All commands below assume `test/content` is the current directory.


- Report implementation capabilities:

    ```
    vpl-inspect
    ```

- Decode an an H.265 encoded video file:

    ```
    vpl-decode -if h265 -i cars_128x96.h265 -o out.i420
    ```

- Encode a raw video file to H.265:

    ```
    vpl-encode -if i420 -i cars_128x96.i420 -sw 128 -sh 96 -of h265 -o out.h265
    ```

- Resize an I420 raw video file:

    ```
    vpl-vpp -if i420 -i cars_128x96.i420 -sw 128 -sh 96 -of i420 -o out_640x480.i420 -dw 640 -dh 480
    ```


## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for more information.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file
for details.

## Security

See the [Intel Security Center](https://www.intel.com/content/www/us/en/security-center/default.html) for information on how to report a potential
security issue or vulnerability.
