::------------------------------------------------------------------------------
:: Copyright (C) Intel Corporation
::
:: SPDX-License-Identifier: MIT
::------------------------------------------------------------------------------
:: start of boilerplate to switch to project root ------------------------------
@echo on
SETLOCAL
FOR /D %%i IN ("%~dp0\..") DO (
	set PROJ_DIR=%%~fi
)
cd %PROJ_DIR%
:: Read options ----------------------------------------------------------------
SET USE_GPL=no
SET BUILD_MODE=Release
SET BUILD_ARCH=x86_64

:Loop
IF "%~1"=="" GOTO Continue
  IF "%~1"=="gpl" (
    SET USE_GPL=yes
  )
  IF "%~1"=="debug" (
    SET BUILD_MODE=Debug
  )
  IF "%~1"=="-A" (
    SET BUILD_ARCH=%~2
    SHIFT
  )
SHIFT
GOTO Loop
:Continue

:: start of commands -----------------------------------------------------------
set CMAKE_BINARY_DIR=_build
if defined VPL_INSTALL_DIR (
  call "%VPL_INSTALL_DIR%\env\vars.bat" || exit /b 1
  set INSTALL_OPTS=-DCMAKE_INSTALL_PREFIX=%VPL_INSTALL_DIR%
)

if "%USE_GPL%"=="yes" (
  set GPL_OPTS=-DBUILD_GPL_X264=ON
)

mkdir %CMAKE_BINARY_DIR%
cd %CMAKE_BINARY_DIR%

:: work around parallel build bug
mkdir %BUILD_MODE%

IF "%BUILD_ARCH%"=="x86_64" (
  SET ARCH=x64
)
IF "%BUILD_ARCH%"=="x86_32" (
  SET ARCH=Win32
)

cmake -A %ARCH% %INSTALL_OPTS% %GPL_OPTS% -DCMAKE_BUILD_TYPE=%BUILD_MODE% .. ^
      || exit /b 1

cmake --build . --config %BUILD_MODE% -j %NUMBER_OF_PROCESSORS% || exit /b 1

cmake --build . --config %BUILD_MODE% --target package || exit /b 1

if defined TEAMCITY_VERSION (
   echo ##teamcity[publishArtifacts 'oneVPL-cpu/%CMAKE_BINARY_DIR%/*-all.zip=^>']
)
