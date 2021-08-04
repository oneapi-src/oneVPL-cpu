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

IF DEFINED WARNING_AS_ERROR_OPT (
  SET WARN_CM_OPTS=-DENABLE_WARNING_AS_ERROR=ON
)

PUSHD %PROJ_DIR%
  SET BUILD_DIR=_build
  MKDIR %BUILD_DIR%
  PUSHD  %BUILD_DIR%
    @REM work around parallel build bug
    MKDIR %COFIG_OPT%
    cmake %ARCH_CM_OPT% %INSTALL_PREFIX_CM_OPT% %COFIG_CM_OPT% %GPL_OPTS% %WARN_CM_OPTS% .. ^
        || EXIT /b 1
    IF DEFINED NUMBER_OF_PROCESSORS (
      SET PARALLEL_OPT=-j %NUMBER_OF_PROCESSORS%
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
