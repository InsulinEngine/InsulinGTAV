@echo off
setlocal EnableDelayedExpansion

set "SCRIPTDIR=%~dp0"
if "%SCRIPTDIR:~-1%"=="\" set "SCRIPTDIR=%SCRIPTDIR:~0,-1%"

rem vswhere lives under a path containing "(x86)"; resolve it up front, because
rem expanding that inside a parenthesised if-block breaks cmd's parser.
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

if not defined OO_PS4_TOOLCHAIN goto :wsl

if /i "%~1"=="clean" (
    if exist "%SCRIPTDIR%\build" rmdir /s /q "%SCRIPTDIR%\build"
    echo Cleaned.
    exit /b 0
)

where cmake >nul 2>&1
if errorlevel 1 echo cmake was not found on PATH. & exit /b 1
where clang >nul 2>&1
if errorlevel 1 echo clang was not found on PATH -- install LLVM and add its bin directory. & exit /b 1

rem The Visual Studio generators cannot drive this cross-compile, so a
rem single-config generator is required. Ninja ships inside VS when it is not
rem already on PATH.
set "NINJA="
for /f "delims=" %%i in ('where ninja 2^>nul') do set "NINJA=%%i"
if not defined NINJA if exist "%VSWHERE%" for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -property installationPath 2^>nul`) do (
    if exist "%%i\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe" set "NINJA=%%i\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"
)
if not defined NINJA echo Ninja was not found on PATH or in a Visual Studio install. Try: winget install Ninja-build.Ninja & exit /b 1

cmake -B "%SCRIPTDIR%\build" -S "%SCRIPTDIR%" -G Ninja -DCMAKE_MAKE_PROGRAM="!NINJA!" -DCMAKE_TOOLCHAIN_FILE="%SCRIPTDIR%/cmake/oo-ps4-toolchain.cmake"
if errorlevel 1 exit /b 1
cmake --build "%SCRIPTDIR%\build"
if errorlevel 1 exit /b 1
exit /b 0

:wsl
for /f "usebackq tokens=2 delims==" %%i in (`wsl -- bash -ic "env | grep ^OO_PS4_TOOLCHAIN="`) do set "OO_TOOLCHAIN_WSL=%%i"
if "%OO_TOOLCHAIN_WSL%"=="" (
    echo OO_PS4_TOOLCHAIN is not set in your WSL environment. See the OpenOrbis PS4 Toolchain README.
    exit /b 1
)

for /f "usebackq delims=" %%i in (`wsl wslpath "%SCRIPTDIR%"`) do set "WSLDIR=%%i"

if "%~1"=="clean" (
    wsl -- bash -c "rm -rf '%WSLDIR%/build'"
    echo Cleaned.
    goto :eof
)

wsl -- bash -c "export OO_PS4_TOOLCHAIN=%OO_TOOLCHAIN_WSL%; export PATH=%OO_TOOLCHAIN_WSL%/bin/linux:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin; export DOTNET_SYSTEM_GLOBALIZATION_INVARIANT=1; cd '%WSLDIR%' && cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=cmake/oo-ps4-toolchain.cmake && cmake --build build"