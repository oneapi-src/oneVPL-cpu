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
    --desc "Build cpu dependencies." ^
    -- %*
IF DEFINED OPT_ERROR ( EXIT /b 2 )
IF DEFINED HELP_OPT ( EXIT /b 0 )

py -3 %SCRIPT_DIR%\bootstrap.py %FORWARD_OPTS%
SET result=%errorlevel%

@REM Export build dependency environment
@REM Note: this logic mirrors similar logic in bootstrap.py 
IF DEFINED VPL_BUILD_DEPENDENCIES (
  ENDLOCAL & SET "VPL_BUILD_DEPENDENCIES=%VPL_BUILD_DEPENDENCIES%" & EXIT /B %result%
) ELSE (
  ENDLOCAL & EXIT /B %result%
)
