@REM ------------------------------------------------------------------------------
@REM Copyright (C) Intel Corporation
@REM
@REM SPDX-License-Identifier: MIT
@REM ------------------------------------------------------------------------------
@REM Run basic tests on base.

@ECHO off
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION

@REM Read command line options
CALL %~dp0%\_buildopts.bat ^
    --name "%~n0%" ^
    --desc "Build project code." ^
    -- %*
IF DEFINED HELP_OPT ( EXIT /b 0 )

@REM ------------------------------------------------------------------------------
@REM Globals
IF NOT DEFINED VPL_CPU_BUILD_DIR (
    set "VPL_CPU_BUILD_DIR=%PROJ_DIR%\_build"
)
@REM ------------------------------------------------------------------------------

set /A result_all=0

SET BUILD_DIR=%VPL_CPU_BUILD_DIR%
PUSHD %BUILD_DIR%
  ctest --output-on-failure -C %COFIG_OPT% -T test
  SET result_all=%errorlevel%
POPD

ENDLOCAL && EXIT /B %result_all%
