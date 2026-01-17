@echo off
REM ================================================
REM Build and run Pirate Scrabble (Windows, MSVC + vcpkg)
REM ================================================

REM --- Paths ---
set VCPKG_PATH=%~dp0external\vcpkg
set BUILD_DIR=%~dp0build
set CONFIG=Debug
set GENERATOR="Visual Studio 17 2022"
set ARCH=x64
set TARGET=pirate-scrabble

cd ..

REM --- Make build directory if it doesn't exist ---
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM --- Go to build directory ---
cd /d "%BUILD_DIR%"

REM --- Configure CMake ---
echo Configuring CMake...
cmake .. ^
  -G %GENERATOR% ^
  -A %ARCH% ^
  -DCMAKE_TOOLCHAIN_FILE=%VCPKG_PATH%\scripts\buildsystems\vcpkg.cmake ^
  -DCMAKE_BUILD_TYPE=%CONFIG%

IF ERRORLEVEL 1 (
    echo CMake configuration failed!
    exit /b 1
)

REM --- Build project ---
echo Building project...
cmake --build . --config %CONFIG% --target %TARGET% -- /m

IF ERRORLEVEL 1 (
    echo Build failed!
    exit /b 1
)

REM --- Run executable ---
echo Running %TARGET%...
"%BUILD_DIR%\%CONFIG%\%TARGET%.exe"

pause
