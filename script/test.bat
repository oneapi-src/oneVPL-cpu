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
set VPL_INTEL_ARCH=intel64
if "%ARCH_OPT%"=="x86_32" (
  set VPL_INTEL_ARCH=ia32
)

:: do custom environment configuration if var is set.
if defined VPL_INSTALL_DIR (
   echo VPL_INSTALL_DIR set, assuming environment script needs to be run.
   if exist "%VPL_INSTALL_DIR%\share\oneVPL\env" (
      echo Using custom environment configuration from %VPL_INSTALL_DIR%\share\oneVPL\env
      call "%VPL_INSTALL_DIR%\share\oneVPL\env\vars.bat" %VPL_INTEL_ARCH% || exit /b 1
   ) else (
      echo Using custom environment configuration from %VPL_INSTALL_DIR%\env
      call "%VPL_INSTALL_DIR%\env\vars.bat" %VPL_INTEL_ARCH% || exit /b 1
   )
)

set /A result_all = 0
PUSHD %PROJ_DIR%
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
POPD

ENDLOCAL && EXIT /B %result_all%
