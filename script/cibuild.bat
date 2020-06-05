::------------------------------------------------------------------------------
:: Copyright (C) 2020 Intel Corporation
::
:: SPDX-License-Identifier: MIT
::------------------------------------------------------------------------------
:: start of boilerplate to switch to project root ------------------------------
@echo off
SETLOCAL
:: switch cd to repo root
FOR /D %%i IN ("%~dp0\..") DO (
	set PROJ_DIR=%%~fi
)
cd %PROJ_DIR%
:: start of commands -----------------------------------------------------------
call "script/lint.bat" || exit /b 1
if not defined VPL_BUILD_DEPENDENCIES (
   call "script/bootstrap.bat" || exit /b 1
)
call "script/package.bat" || exit /b 1 
