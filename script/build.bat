::------------------------------------------------------------------------------
:: Copyright (C) 2020 Intel Corporation
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

:: Read options -----------------------------------------------------------
SET USE_GPL=no
SET BUILD_MODE=Release

:Loop
IF "%~1"=="" GOTO Continue
  IF "%~1"=="gpl" (
    SET USE_GPL=yes
  )
  IF "%~1"=="debug" (
    SET BUILD_MODE=Debug
  )
SHIFT
GOTO Loop
:Continue

:: start of commands -----------------------------------------------------------
set CMAKE_BINARY_DIR=_build

mkdir %CMAKE_BINARY_DIR%
cd %CMAKE_BINARY_DIR%

if "%USE_GPL%"=="yes" (
  cmake -A x64 -DBUILD_GPL_X264=ON ..
) else (
  cmake -A x64 ..
)

cmake --build . --config %BUILD_MODE% -j %NUMBER_OF_PROCESSORS%
 
