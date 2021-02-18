@REM ------------------------------------------------------------------------------
@REM Copyright (C) Intel Corporation
@REM 
@REM SPDX-License-Identifier: MIT
@REM ------------------------------------------------------------------------------
@REM Build cpu.

@ECHO off
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION 

@REM Read command line options
CALL %~dp0%\_buildopts.bat ^
    --name "%~n0%" ^
    --desc "Build cpu." ^
    -- %*
IF DEFINED HELP_OPT ( EXIT /b 0 )

@REM Load project environment
SET VARS_SCRIPT=vars.bat
IF "%ARCH_OPT%"=="x86_32" (
  SET VARS_SCRIPT=vars32.bat
)
ECHO %VARS_SCRIPT%
IF NOT DEFINED VPL_ROOT (

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

IF DEFINED BOOTSTRAP_OPT (
    ECHO Building dependencies...
    %SCRIPT_DIR%/bootstrap.bat %FORWARD_OPTS%
  )
)

IF "%ARCH_OPT%"=="x86_64" (
  SET ARCH_CM_OPT=-A x64
  IF DEFINED GPL_OPT (
    SET GPL_OPTS=-DBUILD_GPL_X264=ON
  )
) ELSE IF "%ARCH_OPT%"=="x86_32" (
  SET ARCH_CM_OPT=-A Win32
  SET VPL_DIR=%VPL_ROOT%\lib32\cmake
)

IF DEFINED VPL_INSTALL_DIR (
  MD %VPL_INSTALL_DIR%
  SET INSTALL_PREFIX_CM_OPT=-DCMAKE_INSTALL_PREFIX=%VPL_INSTALL_DIR%
)

SET COFIG_CM_OPT=-DCMAKE_BUILD_TYPE=%COFIG_OPT%

PUSHD %PROJ_DIR%
  SET BUILD_DIR=_build
  MKDIR %BUILD_DIR%
  PUSHD  %BUILD_DIR%
    @REM work around parallel build bug
    MKDIR %COFIG_OPT%
    cmake %ARCH_CM_OPT% %INSTALL_PREFIX_CM_OPT% %COFIG_CM_OPT% %GPL_OPTS% .. ^
        || EXIT /b 1
    IF DEFINED NUMBER_OF_PROCESSORS (
      PARALLEL_OPT=-j NUMBER_OF_PROCESSORS
    )
    cmake --build . --config %COFIG_OPT% %PARALLEL_OPT% || EXIT /b 1
    cmake --build . --config %COFIG_OPT% --target package || EXIT /b 1

    @REM Signal to CI system 
    IF DEFINED TEAMCITY_VERSION (
      ECHO ##teamcity[publishArtifacts 'oneVPL-cpu/%BUILD_DIR%/*-all.zip=^>']
    )
  POPD
POPD
ENDLOCAL