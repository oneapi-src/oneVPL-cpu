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

@REM Load project environment
SET VARS_SCRIPT=vars.bat
IF "%ARCH_OPT%"=="x86_32" (
  SET VARS_SCRIPT=vars32.bat
)

IF NOT DEFINED "%VPL_ROOT%" (
  IF NOT EXIST "%VPL_INSTALL_DIR%" (
    ECHO "Base must be installed to build implementation"
    EXIT /b 1
  )

  IF NOT EXIST "%VPL_INSTALL_DIR%\env\%VARS_SCRIPT%" (
    @REM Detect case where user points VPL_INSTALL_DIR at the
    @REM Base repo instead of the built output.
    ECHO "Cannot find environment script in %VPL_INSTALL_DIR%\env"
    EXIT /b 1
  )

  CALL "%VPL_INSTALL_DIR%\env\%VARS_SCRIPT%" || EXIT /b 1
)

set /A result_all = 0
SET BUILD_DIR=_build
PUSHD  %BUILD_DIR%
  PUSHD %COFIG_OPT%
    ECHO *** Running Unit Tests ***
    CALL vpl-utest.exe --gtest_output=xml:%PROJ_DIR%\_logs\
    IF %errorlevel%==0 GOTO unit_tests_passed
    ECHO *** Unit Tests FAILED ***
    SET /A result_all = 1
    GOTO test_end

    :unit_tests_passed
    echo *** Unit Tests PASSED ***

    :test_end
  POPD
POPD

ENDLOCAL && EXIT /B %result_all%
