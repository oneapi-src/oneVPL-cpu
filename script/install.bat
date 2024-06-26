@REM ------------------------------------------------------------------------------
@REM Copyright (C) Intel Corporation
@REM
@REM SPDX-License-Identifier: MIT
@REM ------------------------------------------------------------------------------
@REM Install cpu.

@ECHO off
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION

@REM Read command line options
CALL %~dp0%\_buildopts.bat ^
    --name "%~n0%" ^
    --desc "Install cpu." ^
    -- %*
IF DEFINED HELP_OPT ( EXIT /b 0 )

@REM ------------------------------------------------------------------------------
@REM Globals
IF NOT DEFINED VPL_CPU_BUILD_DIR (
    set "VPL_CPU_BUILD_DIR=%PROJ_DIR%\_build"
)
@REM ------------------------------------------------------------------------------

SET BUILD_DIR=%VPL_CPU_BUILD_DIR%
cmake --build %BUILD_DIR% --config %COFIG_OPT% --target install

ENDLOCAL
