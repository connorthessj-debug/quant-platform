@echo off
echo === Building Quant Platform with MSVC ===

if not exist build_msvc mkdir build_msvc
cd build_msvc

cmake .. -G "Visual Studio 17 2022" -A x64
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
