@echo off
setlocal
cd /d "%~dp0"

where cmake >nul 2>nul || (echo [!] CMake missing. Install it and add to PATH. & pause & exit /b)

powershell -ExecutionPolicy Bypass -File "%~dp0setup.ps1" || (pause & exit /b)

cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release || (pause & exit /b)
cmake --build build --config Release --parallel || (pause & exit /b)

if exist "build\Release\FiveMExternal.exe" (
    copy /y "build\Release\FiveMExternal.exe" "FiveMExternal.exe" >nul
    echo.
    echo [+] Built: %cd%\FiveMExternal.exe
)
pause