::------------------------------------------------------------------------------
:: Copyright (C) 2020 Intel Corporation
::
:: SPDX-License-Identifier: MIT
::------------------------------------------------------------------------------
@echo off
SETLOCAL
:: switch cd to repo root
FOR /D %%i IN ("%~dp0\..") DO (
	set PROJ_DIR=%%~fi
)
cd %PROJ_DIR%
:: start of commands -----------------------------------------------------------
gitlint || exit /b 1
pre-commit run --all-files
