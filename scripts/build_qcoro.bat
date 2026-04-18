@echo off
setlocal

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%..
set QCORO_DIR=%PROJECT_ROOT%\src\vendor\qcoro

echo Building qCoro...
echo Project root: %PROJECT_ROOT%
echo qCoro directory: %QCORO_DIR%

if not exist "%QCORO_DIR%" (
    echo ERROR: qCoro directory not found: %QCORO_DIR%
    exit /b 1
)

cd /d "%QCORO_DIR%"

echo Configuring qCoro with CMake...
cmake -B build -S . -G Ninja ^
    -DQCORO_WITH_QTWEBSOCKETS=OFF ^
    -DBUILD_TESTING=OFF ^
    -DQCORO_BUILD_EXAMPLES=OFF

if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake configuration failed
    exit /b %ERRORLEVEL%
)

echo Building qCoro...
cmake --build build

if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed
    exit /b %ERRORLEVEL%
)

echo Installing qCoro to local install directory...
cmake --install build --prefix install

if %ERRORLEVEL% neq 0 (
    echo ERROR: Installation failed
    exit /b %ERRORLEVEL%
)

echo Copying qCoro install to build directory...
cd /d "%PROJECT_ROOT%"
if not exist "build\vendor\qcoro\" mkdir build\vendor\qcoro
xcopy /E /I /Y "%QCORO_DIR%\install" "build\vendor\qcoro\install\"

echo.
echo ========================================
echo qCoro build completed successfully!
echo Install directory: %QCORO_DIR%\install
echo Build directory: %PROJECT_ROOT%\build\vendor\qcoro\install
echo ========================================

endlocal
