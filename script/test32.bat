::------------------------------------------------------------------------------
:: Copyright (C) Intel Corporation
::
:: SPDX-License-Identifier: MIT
::------------------------------------------------------------------------------
:: start of boilerplate to switch to project root ------------------------------
@echo off
SETLOCAL
FOR /D %%i IN ("%~dp0\..") DO (
	set PROJ_DIR=%%~fi
)
cd %PROJ_DIR%
:: start of commands -----------------------------------------------------------
call "%PROJ_DIR%/test/tools/env/vars.bat"
if defined VPL_BUILD_DEPENDENCIES (
  set ffmpeg_dir=%VPL_BUILD_DEPENDENCIES%\bin
) else (
    echo VPL_BUILD_DEPENDENCIES not defined. Did you run bootstrap?
    exit /b 1
  )
)
set "PATH=%ffmpeg_dir%;%PATH%"

if defined VPL_INSTALL_DIR (
   if not defined VPL_ROOT (
      call "%VPL_INSTALL_DIR%\env\vars32.bat" || exit /b 1
   )
)


cd _build\Release
set /A result_all = 0

:unit_tests
echo *** Running Unit Tests ***
call vpl-utest.exe --gtest_output=xml:%PROJ_DIR%\_logs\
if %errorlevel%==0 goto unit_tests_passed
echo *** Unit Tests FAILED ***
set /A result_all = 1
goto test_end

:unit_tests_passed
echo *** Unit Tests PASSED ***

:test_end
exit /B %result_all%
