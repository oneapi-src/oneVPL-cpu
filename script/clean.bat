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

@REM ------------------------------------------------------------------------------
@REM Globals
IF NOT DEFINED VPL_CPU_DEPS_BUILD_DIR (
    set "VPL_CPU_DEPS_BUILD_DIR=%PROJ_DIR%\_extbuild"
)

IF NOT DEFINED VPL_BUILD_DEPENDENCIES (
    set "VPL_BUILD_DEPENDENCIES=%PROJ_DIR%\_deps"
)

IF NOT DEFINED VPL_CPU_BUILD_DIR (
    set "VPL_CPU_BUILD_DIR=%PROJ_DIR%\_build"
)
@REM ------------------------------------------------------------------------------

IF DEFINED BOOTSTRAP_OPT (
  IF EXIST %VPL_BUILD_DEPENDENCIES% (
    ECHO Cleaning dependencies cache folder...
    RD /s /q %VPL_BUILD_DEPENDENCIES%
  )
  IF EXIST %VPL_CPU_DEPS_BUILD_DIR% (
    ECHO Cleaning dependencies build folder...
    RD /s /q %VPL_CPU_DEPS_BUILD_DIR%
  )
)

IF EXIST %VPL_CPU_BUILD_DIR% (
  ECHO Cleaning cpu build folder...
  RD /s /q %VPL_CPU_BUILD_DIR%
)

ENDLOCAL