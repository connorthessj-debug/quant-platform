@echo off
echo === Building Quant Platform with MSVC ===

REM Requires: CMake on PATH and Visual Studio Build Tools installed.
REM If cmake is not found, run from a Visual Studio Developer Command Prompt
REM or add CMake to your PATH manually.

where cmake >nul 2>nul
if errorlevel 1 (
    echo ERROR: cmake not found on PATH.
    echo Install CMake from https://cmake.org or run from VS Developer Command Prompt.
    pause
    exit /b 1
)

if not exist build_msvc mkdir build_msvc
cd build_msvc

cmake .. -A x64
if errorlevel 1 (
    echo CMake configuration failed.
    pause
    exit /b 1
)

cmake --build . --config Release
if errorlevel 1 (
    echo Build failed.
    pause
    exit /b 1
)

echo === Build successful ===
echo Output: build_msvc\Release\QuantPlatform.exe

xcopy /Y /I ..\config Release\config\
xcopy /Y /I /S ..\data Release\data\

echo === Ready to run ===
pause
