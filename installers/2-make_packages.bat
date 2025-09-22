@echo off
setlocal


REM -- Release, will be inherited by NSIS scripts --
if "%VERSION%"=="" (
    echo [ERROR] VERSION is not defined. Please set it before running this script.
    exit /b 1
)

REM -- Set paths --
set NSIS_COMPILER="C:\Program Files (x86)\NSIS\makensis.exe"
set SIGNED_BIN_DIR=..\ARTS\bin\dist\%VERSION%\signed
set DOC_DIR=..\doc-help
set OUTPUT_DIR=..\releases
set ZIP_APP="C:\Program Files\7-Zip\7z.exe"



REM -- Check NSIS exists --
if not exist %NSIS_COMPILER% (
    echo [ERROR] NSIS compiler not found at %NSIS_COMPILER%
    exit /b 1
)


REM -- Ensure 7-Zip is available --
%ZIP_APP% >nul 2>&1
if errorlevel 1 (
    echo [ERROR] 7-Zip is not in PATH. Install 7-Zip CLI and ensure 7z.exe is accessible.
    exit /b 1
)

REM -- Compile each NSI script --
for %%F in (
    tftpd_xx_installer.nsi
    tftpd_xx_service_edition.nsi
) do (
    echo ---------------------------------------------
    echo Building: %%F
    echo ---------------------------------------------
    %NSIS_COMPILER% /DPRODUCT_VERSION=%VERSION% /DARCH=32 "%%F"
    if errorlevel 1 (
        echo [ERROR] Failed to compile %%F
        exit /b 1
    )
    %NSIS_COMPILER% /DPRODUCT_VERSION=%VERSION% /DARCH=64 "%%F"
    if errorlevel 1 (
        echo [ERROR] Failed to compile %%F
        exit /b 1
    )
)


REM -- Create portable ZIPs --
echo Creating portable packages...

REM Create tftpd32 portable zip
set ZIP_NAME=%OUTPUT_DIR%\tftpd32_portable_v%VERSION%.zip
%ZIP_APP% a -tzip "%ZIP_NAME%" ^
    tftpd32.ini EUPL-EN.pdf ^
    "%DOC_DIR%\tftpd32.chm" ^
    "%SIGNED_BIN_DIR%\tftpd32.exe"
if errorlevel 1 (
    echo [ERROR] Failed to create tftpd32 portable zip
    exit /b 1
)

REM Create tftpd64 portable zip
set ZIP_NAME=%OUTPUT_DIR%\tftpd64_portable_v%VERSION%.zip
%ZIP_APP% a -tzip "%ZIP_NAME%" ^
    tftpd32.ini EUPL-EN.pdf ^
    "%DOC_DIR%\tftpd32.chm" ^
    "%SIGNED_BIN_DIR%\tftpd64.exe"
if errorlevel 1 (
    echo [ERROR] Failed to create tftpd64 portable zip
    exit /b 1
)

echo ---------------------------------------------
echo All installers and portable packages created.

echo All installers built successfully.
endlocal
