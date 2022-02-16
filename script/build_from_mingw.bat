@REM ------------------------------------------------------------------------------
@REM Copyright (C) Intel Corporation
@REM 
@REM SPDX-License-Identifier: MIT
@REM ------------------------------------------------------------------------------
@REM Build cpu dependencies.

@ECHO off
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION 

@REM Read command line options
CALL %~dp0%\_buildopts.bat ^
    --name "%~n0%" ^
    --desc "Build cpu." ^
    -- %*
IF DEFINED HELP_OPT ( EXIT /b 0 )

ECHO [build_from_mingw.bat] Calling build_from_mingw.py (%FORWARD_OPTS%)...
py -3 %SCRIPT_DIR%\build_from_mingw.py %FORWARD_OPTS% || exit /b 1

@REM Signal to CI system
IF DEFINED TEAMCITY_VERSION (
    ECHO ##teamcity[publishArtifacts '%VPL_CPU_BUILD_DIR%/*-all.zip=^>']
)
ENDLOCAL
