@echo off
REM Release packaging script for raycasting-engine (Windows)
REM Creates a distributable package with the game and assets

echo === Raycasting Engine - Release Packaging ===

REM Configuration
set PROJECT_NAME=raycasting-engine
set BUILD_DIR=out\Release
set PACKAGE_DIR=dist
set ASSETS_DIR=ressources

REM Clean previous package
echo Cleaning previous packages...
if exist "%PACKAGE_DIR%" rmdir /s /q "%PACKAGE_DIR%"
mkdir "%PACKAGE_DIR%\%PROJECT_NAME%"

REM Check if release build exists
if not exist "%BUILD_DIR%" (
    echo Error: Release build not found!
    echo Please run .\scripts\build-release.bat first
    exit /b 1
)

REM Copy executable
echo Copying executable...
if exist "%BUILD_DIR%\%PROJECT_NAME%.exe" (
    copy "%BUILD_DIR%\%PROJECT_NAME%.exe" "%PACKAGE_DIR%\%PROJECT_NAME%\"
) else if exist "%BUILD_DIR%\Release\%PROJECT_NAME%.exe" (
    copy "%BUILD_DIR%\Release\%PROJECT_NAME%.exe" "%PACKAGE_DIR%\%PROJECT_NAME%\"
) else (
    echo Error: Executable not found in %BUILD_DIR%
    exit /b 1
)

REM Copy assets
echo Bundling assets...
if exist "%ASSETS_DIR%" (
    xcopy /e /i /y "%ASSETS_DIR%" "%PACKAGE_DIR%\%PROJECT_NAME%\%ASSETS_DIR%"
) else (
    echo Warning: Assets directory not found, creating empty directory
    mkdir "%PACKAGE_DIR%\%PROJECT_NAME%\%ASSETS_DIR%"
)

REM Copy README and LICENSE
echo Copying documentation...
if exist "README.md" copy "README.md" "%PACKAGE_DIR%\%PROJECT_NAME%\"
if exist "LICENSE" copy "LICENSE" "%PACKAGE_DIR%\%PROJECT_NAME%\"

REM Create archive
echo Creating release archive...
cd "%PACKAGE_DIR%"
powershell -Command "Compress-Archive -Path '%PROJECT_NAME%' -DestinationPath '%PROJECT_NAME%-release.zip' -Force"
if %errorlevel% equ 0 (
    echo Created: %PACKAGE_DIR%\%PROJECT_NAME%-release.zip
) else (
    echo Warning: Failed to create zip archive
)
cd ..

echo.
echo === Packaging Complete ===
echo Release package is in: %PACKAGE_DIR%\
pause
