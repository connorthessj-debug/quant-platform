@echo off
echo === Building Quant Platform with MinGW-w64 ===

REM Requires: CMake and MinGW-w64 (g++, mingw32-make) on PATH.

where cmake >nul 2>nul
if errorlevel 1 (
    echo ERROR: cmake not found on PATH.
    pause
    exit /b 1
)

if not exist build_mingw mkdir build_mingw
cd build_mingw

cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
    echo CMake configuration failed.
    pause
    exit /b 1
)

cmake --build .
if errorlevel 1 (
    echo Build failed.
    pause
    exit /b 1
)

echo === Build successful ===
echo Output: build_mingw\QuantPlatform.exe

xcopy /Y /I ..\config config\
xcopy /Y /I /S ..\data data\

echo NOTE: Installer scripts expect build_msvc\Release\QuantPlatform.exe
echo       Copy the exe there if using installer: copy QuantPlatform.exe ..\build_msvc\Release\
echo === Ready to run ===
pause
