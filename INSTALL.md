# Build/install Instructions

**NOTE** Make sure your shell is configured to allow downloads and git clones from the
  public Internet.
  
  
## Build oneVPL And oneVPL-cpu Repositories from Sources
**NOTE**: You may skip building oneVPL repo if you choose to:
- Obtain and install the oneVPL base package from the
[oneVPL repository](https://github.com/oneapi-src/oneVPL).
- Extract the package to the directory pointed to by $VPL_INSTALL_DIR or %VPL_INSTALL_DIR% environment variable
- Set environment variable VPL_INSTALL_DIR so that it points to the _vplinstall directory
- Skip to the step "6. Configure, build, and install oneVPL-cpu implementation"

### Build and Install on Linux*

#### 1. Install required packages

```
sudo apt-get update && sudo apt-get install -y --no-install-recommends \
    build-essential git pkg-config yasm nasm cmake libva-dev libva-drm2 python3 \
    python3-setuptools python3-pip
sudo pip3 install -U wheel
sudo pip3 install meson ninja
```
#### 2. Update path and specify VPL Install Directory
```
export VPL_INSTALL_DIR=$HOME/_vplinstall
```
#### 3. Create a VPL install directory and run commands from home directory
```
cd $HOME
mkdir _vplinstall
```

#### 4. Clone VPL open source repos
```
git clone https://github.com/oneapi-src/oneVPL.git
git clone https://github.com/oneapi-src/oneVPL-cpu.git
```
**NOTE** Run these commands from home directory. These urls are for open source repos. They create oneVPL and oneVPL-cpu directories in home directory.

#### 5. Configure, build, and install oneVPL base
```
cd oneVPL
mkdir _build
cd _build
cmake .. -DCMAKE_INSTALL_PREFIX=$VPL_INSTALL_DIR
cmake --build . --config Release --target install
```

#### 6. Configure, build, and install oneVPL-cpu implementation
```		
cd $HOME/oneVPL-cpu
source script/bootstrap
script/build
script/install
```

#### 7. Verify build and installation was successful
```		
cd $VPL_INSTALL_DIR
source ./share/oneVPL/env/vars.sh
cd bin
./vpl-inspect
```
**NOTE** Expect to see the enumerations of adapter information. 

### Build and Install on Windows*

#### 1. Install the following common developer tools

- [Microsoft Visual Studio*](https://visualstudio.microsoft.com/), 2019 or newer
- [CMake*](https://cmake.org/)
- [Python* 3](https://www.python.org/)
- [Git*](https://git-scm.com/)
- [Chocolatey*](http://chocolatey.org)

#### 2. Install MSYS2 based tools.

2.1 Install MSYS2 from a Windows cmd or powershell with admin permissions:
```
choco install msys2
```

2.2 Start a mingw64 shell from Windows cmd or powershell with regular permissions:
```
C:\tools\msys64\msys2.exe
```
---

**NOTE** This path is for default choco install. If another install method is
used the path to the shell may be different.

---
2.3 Get packages required by build using the mingw64 shell:
```
pacman -Syu
pacman --needed -Sy mingw-w64-x86_64-toolchain base-devel yasm nasm
pacman --needed -Sy mingw-w64-x86_64-cmake git python-pip
pacman --needed -Sy mingw-w64-x86_64-meson mingw-w64-x86_64-ninja
```

**NOTE** For 32-bit builds you will also need a 32-bit toolchain
```
pacman --needed -Sy mingw-w64-i686-toolchain
```

#### 3. Specify VPL Install directory

**NOTE** All commands starting from here should be run in the same Windows command prompt 
in the user home directory e.g. c:\users\\[username] 

```
md _vplinstall 
set VPL_INSTALL_DIR=c:\users\[username]\_vplinstall
```

#### 4. Clone VPL open source repos
```
git clone https://github.com/oneapi-src/oneVPL.git
git clone https://github.com/oneapi-src/oneVPL-cpu.git
```
**NOTE** Run commands from user home directory. oneVPL and oneVPL-cpu directories will be created there.
Those urls are for VPL open source repos.
	
#### 5. Configure, build, and install oneVPL base
```
cd oneVPL
md _build
cd _build
cmake .. -DCMAKE_INSTALL_PREFIX=%VPL_INSTALL_DIR%
cmake --build . --config Release --target install
```
**NOTE** The intermediary build output from oneVPL repo will be installed to %VPL_INSTALL_DIR%

---
#### 6. Configure, build, and install oneVPL-cpu implementation
```
cd c:\users\[username]\oneVPL-cpu
script\bootstrap
script\build
script\install
```		
**NOTE** The build output from oneVPL-cpu repo will also be in %VPL_INSTALL_DIR%

#### 7. Verify build and installation was successful
```		
cd %VPL_INSTALL_DIR%
share\oneVPL\env\vars.bat
cd bin
vpl-inspect
```
**NOTE** Expect to see the enumerations of adapter information. 

## Enable higher performance h264 encode by passing gpl flag

oneVPL CPU implementation supports `OpenH264` as the default H.264 encoder.

Add `gpl` to enable `x264` instead.

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

## Explore other build options
The bootstrap and build scripts accept additional options. See the -h flag for more details.


