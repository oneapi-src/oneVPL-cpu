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

PUSHD  %PROJ_DIR%
  SET BUILD_DIR=_build 
  cmake --build %BUILD_DIR% --config %COFIG_OPT% --target install
POPD

ENDLOCAL
