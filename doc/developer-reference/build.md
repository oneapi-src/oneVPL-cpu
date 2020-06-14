## Configuring the build environment

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

- ### Configuring the Windows environment

    - #### Install common developer tools

        - [Microsoft Visual Studio](https://visualstudio.microsoft.com/)

        - [CMake](https://cmake.org/)

        - [Python3](https://www.python.org/)

        - [git](https://git-scm.com/)

        - [Doxygen](http://www.doxygen.nl/) (to build documentation)

    - #### Install MSYS2 based tools

        This can be done many ways but this guide recommends
[chocolatey](http://chocolatey.org). Some of the paths to mingw may be
different if another MSYS install is used.

        From a cmd or powershell with admin permissions
        ```
        choco install msys2
        ```

        Start a mingw64 shell from cmd or powershell with regular permissions
        ```
        c:\tools\msys64\msys2.exe
        ```

        Get packages required by build
        ```
        pacman -Syu
        pacman --needed -Sy mingw-w64-x86_64-toolchain base-devel yasm nasm mingw-w64-x86_64-cmake git python-pip mingw-w64-x86_64-meson mingw-w64-x86_64-ninja
        ```

- ### Configuring the Ubuntu environment

    From a shell as root:
    ```
    apt update
    apt install -y autoconf automake build-essential git pkg-config python3 python3-pip gdb cmake nasm yasm ninja-build meson

    ```
