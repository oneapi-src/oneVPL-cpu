@echo off

:: ############################################################################
::  # Copyright (C) 2019 Intel Corporation
::  #
::  # SPDX-License-Identifier: MIT
::  ############################################################################

:: Configure enviroment variables

:: set VPL_TEST_DIR=%~dp0\..
FOR /D %%i IN ("%~dp0\..") DO (
	set VPL_TEST_DIR=%%~fi
)
set "VPL_TEST_PYTHONPATH=%VPL_TEST_DIR%\tools\python"
IF DEFINED PYTHONPATH ( 
	set "PYTHONPATH=%VPL_TEST_PYTHONPATH%;%PYTHONPATH%"
) ELSE (
	set "PYTHONPATH=%VPL_TEST_PYTHONPATH%"
)
