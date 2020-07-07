

This section describes the steps to configure the environment. It extends the
high level build information in the readme.

The basic sequence is

1. Configure the environment

2. Build dependencies using bootstrap

3. Build the code using the build script

After you have configured the environment you can continue with the basic build
steps in the readme. Note that if dependencies are not modified bootstrapping
only needs to be done once. The build script will look in the location specified
by the `VPL_BUILD_DEPENDENCIES` environment variable.

Configuring the Windows environment
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Install common developer tools


        - [Microsoft Visual Studio](https://visualstudio.microsoft.com/)

        - [CMake](https://cmake.org/)

        - [Python3](https://www.python.org/)

        - [git](https://git-scm.com/)

        - [Doxygen](http://www.doxygen.nl/) (to build documentation)

Install MSYS2 based tools


This can be done many ways but this guide is based on 
[chocolatey](http://chocolatey.org). Some of the paths to mingw may be
different if another MSYS install is used.

From a cmd or powershell with admin permissions

.. code-block:: bash

        choco install msys2


Start a mingw64 shell from cmd or powershell with regular permissions.  

.. code-block:: bash

        c:\tools\msys64\msys2.exe


Please note: this path is from choco install.  If another install method is used
the path to the shell may be different.

Get packages required by build

.. code-block:: bash

        pacman -Syu
        pacman --needed -Sy mingw-w64-x86_64-toolchain base-devel yasm nasm mingw-w64-x86_64-cmake git python-pip mingw-w64-x86_64-meson mingw-w64-x86_64-ninja



Configuring the Ubuntu environment
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

From a shell as root:

.. code-block:: bash

        apt update
        apt install -y autoconf automake build-essential git pkg-config python3 python3-pip gdb cmake nasm yasm ninja-build meson



Docker
^^^^^^

The instructions below are based on mounting the oneVPL directory from the host machine to the docker container, and assume starting from the same directory that oneVPL was cloned into.  The oneVPL directory can be copied to the container instead to create a self-contained container/image.

For Ubuntu 18.04:

.. code-block:: bash

        docker build - < oneVPL/docker/Dockerfile-ubuntu-18.04



For Ubuntu 20.04:

.. code-block:: bash

        docker build - < oneVPL/docker/Dockerfile-ubuntu-20.04



To start the container:

.. code-block:: bash

        docker run -it --rm -v `pwd`/oneVPL:/home/oneVPL <image id> /bin/bash


Inside the docker container, oneVPL can be found at /home/oneVPL


Building oneVPL and running the samples
=======================================

Docker sets up all prerequisites needed.  For running outside docker, be sure to bootstrap the necessary dependencies.

Ubuntu bash shell:

.. code-block:: bash

        script/bootstrap


Windows cmd prompt:

.. code-block:: bash

        script\bootstrap


For all environments, cd to the oneVPL directory then 

Ubuntu bash shell:

.. code-block:: bash

        script/build


Windows cmd prompt:

.. code-block:: bash

        script\build



For Linux and msys/mingw shells, set the library load path to the build directory:

.. code-block:: bash

        export LD_LIBRARY_PATH=`pwd`/_build



Run the hello_decode sample:

.. code-block:: bash

        _build/hello_decode h265 examples/content/cars_1280x720.h265 cars_1280x720.i420



Run the hello_encode sample:

.. code-block:: bash

        ./_build/hello_encode h265 cars_1280x720.i420 test.h265 1280 720
