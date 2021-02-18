@REM ------------------------------------------------------------------------------
@REM Copyright (C) Intel Corporation
@REM 
@REM SPDX-License-Identifier: MIT
@REM ------------------------------------------------------------------------------
@REM Clean cpu.

@ECHO off
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION 

@REM Read command line options
CALL %~dp0%\_buildopts.bat ^
    --name "%~n0%" ^
    --desc "Clean cpu." ^
    -- %*
IF DEFINED HELP_OPT ( EXIT /b 0 )

IF DEFINED BOOTSTRAP_OPT (
  IF EXIST %VPL_BUILD_DEPENDENCIES% (
    ECHO Cleaning dependencies cache folder...
    RD /s /q %VPL_BUILD_DEPENDENCIES%
  )
)

PUSHD %PROJ_DIR%
  IF EXIST _deps (
    ECHO Cleaning cpu dependencies folder...
    RD /s /q _deps
  )

  IF EXIST _extbuild (
    ECHO Cleaning cpu dependencies build folder...
    RD /s /q _extbuild
  )

  IF EXIST _build (
    ECHO Cleaning cpu build folder...
    RD /s /q _build
  )
POPD

ENDLOCAL