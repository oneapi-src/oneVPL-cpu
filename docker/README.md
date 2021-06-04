# Docker file support   

The following files enable building the oneAPI Video Processing Library (oneVPL)
as a Docker container.

# Building

The following command will build all of the supported docker container images:
```bash
$ ./build_docker_images.sh
```

# Running

The following command will illustrate use of one of the oneVPL samples executing from 
a container:
```bash
$ run.sh
```


**LEGAL NOTICE:  By downloading and using any scripts (the “Software Package”) and the included software or software made available for download, you agree to the terms and conditions of the software license agreements for the Software Package, which may also include notices, disclaimers, or license terms for third party software (together, the “Agreements”) included in this README file.
If the Software Package is installed through a silent install, your download and use of the Software Package indicates your acceptance of the Agreements.**

Docker files will also pull in the following: 
* [git](https://git-scm.com)
* [build-essential](https://packages.debian.org/sid/build-essential)
* [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/)
* [yasm](https://github.com/Catstyle/yasm)
* [nasm](https://github.com/MineRobber9000/nasm)
* [cmake](https://cmake.org/)
* [python3](https://www.python.org/download/releases/3.0/)
* [python3-setuptools](https://www.python.org/download/releases/3.0/)
* [python3-pip](https://www.python.org/download/releases/3.0/)
* [wheel](https://github.com/pypa/wheel)
* [meson](https://mesonbuild.com/)
* [ninja](https://ninja-build.org/)
* [autoconf](https://github.com/rhayes777/PyAutoConf)
* [automake](https://github.com/samin36/AutomakeProject)
* [bzip2](https://github.com/mxmlnkn/indexed_bzip2)
* [bzip-devl](https://github.com/mxmlnkn/indexed_bzip2)
* [cmake](https://cmake.org/)
* [freetype-devel](https://www.freetype.org/developer.html)
* [zlib-devel](https://zlib.net/)
* [openssl-devel](https://www.openssl.org/)
* [ca-certificates](https://packages.ubuntu.com/source/hirsute/ca-certificates)
* [dh-autoeconf](https://salsa.debian.org/debian/dh-autoreconf)
* [libdrm-dev](https://cgit.freedesktop.org/mesa/drm/)
* [libtool](https://www.gnu.org/software/libtool/)
* [make](https://www.gnu.org/software/make/)
* [pkgconfig](https://github.com/matze/pkgconfig)
* [devtoolset-7](https://www.softwarecollections.org/en/scls/rhscl/devtoolset-7/)
* [gcc](https://gcc.gnu.org/)
* [gcc-c++](https://gcc.gnu.org/)
* [libarchive](https://www.libarchive.org/)
* [CentOS 7](https://hub.docker.com/layers/centos/library/centos/7/images/sha256-b79613a1c63084399b6c21c97ab4e2816ef5e9c513842c1bd4dca46bdd921b31?context=explore)
* [CentOS 8](https://hub.docker.com/layers/centos/library/centos/centos8/images/sha256-7723d6b5d15b1c64d0a82ee6298c66cf8c27179e1c8a458e719041ffd08cd091?context=explore)
* [Ubuntu 18.04](https://hub.docker.com/layers/ubuntu/library/ubuntu/18.04/images/sha256-dce82ba9ee1bc3a515212bb17fa21c134102bffafb5234a25ac10747df25816b?context=explore)
* [Ubuntu 20.04](https://hub.docker.com/layers/ubuntu/library/ubuntu/20.04/images/sha256-42d5c74d24685935e6167271ebb74c5898c5adf273dae80a82f9e39e8ae0dab4?context=explore)