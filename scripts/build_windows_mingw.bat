@echo off
echo === Building Quant Platform with MinGW-w64 ===

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

echo === Ready to run ===
pause
