::------------------------------------------------------------------------------
:: Copyright (C) Intel Corporation
::
:: SPDX-License-Identifier: MIT
::------------------------------------------------------------------------------
:: start of boilerplate to switch to project root ------------------------------
@echo off
IF defined VERBOSE (
  echo on
)

SETLOCAL EnableDelayedExpansion
FOR /D %%i IN ("%~dp0\..") DO (
	set PROJ_DIR=%%~fi
)
cd %PROJ_DIR%
:: Read options -----------------------------------------------------------
SET GPL_FLAG=
SET BUILD_MODE=Release
SET BUILD_ARCH=x86_64

:Loop
IF "%~1"=="" GOTO Continue
  IF "%~1"=="gpl" (
    SET GPL_FLAG=-gpl
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
set DEFAULT_VPL_BUILD_DEPENDENCIES=%PROJ_DIR%\_deps

py -3 script\\bootstrap.py %GPL_FLAG% -m %BUILD_MODE% -A %BUILD_ARCH% || exit /b 1

:: export build dependency environment
(
endlocal
  if not defined VPL_BUILD_DEPENDENCIES (
     set VPL_BUILD_DEPENDENCIES=%DEFAULT_VPL_BUILD_DEPENDENCIES%
  )
)
