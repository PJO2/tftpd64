@echo off
setlocal

if "%VERSION%"=="" (
    echo [ERROR] VERSION is not defined. Please set it before running this script.
    exit /b 1
)

REM -- Check thumbprint --
if "%SIGN_CERT_THUMBPRINT%"=="" (
    echo [ERROR] SIGN_CERT_THUMBPRINT is not defined. Please set it before running this script.
    exit /b 1
)

REM -- Set path to signtool --
set SIGNTOOL="C:\Program Files (x86)\Windows Kits\10\App Certification Kit\signtool.exe"

REM -- Move to the build directory --
cd /d ..\ARTS\BIN\release 
if errorlevel 1 (
    echo [ERROR] Could not change to ..\ARTS\BIN\release
    exit /b 1
)
REM -- create destination file
mkdir ..\dist\%VERSION% 2>nul
mkdir ..\dist\%VERSION%\signed 2>nul

REM -- Copy original executables to their target names --
echo Copying files to target names...

copy /y tftpd32.x64.exe     ..\dist\%VERSION%\signed\tftpd64.exe
copy /y tftpd32.x86.exe     ..\dist\%VERSION%\signed\tftpd32.exe
copy /y tftpd32_gui.x64.exe ..\dist\%VERSION%\signed\tftpd64_gui.exe
copy /y tftpd32_gui.x86.exe ..\dist\%VERSION%\signed\tftpd32_gui.exe
copy /y tftpd32_svc.x64.exe ..\dist\%VERSION%\signed\tftpd64_svc.exe
copy /y tftpd32_svc.x86.exe ..\dist\%VERSION%\signed\tftpd32_svc.exe

REM -- Sign all 6 executables --
echo Signing executables...

cd /d ..\dist\%VERSION%\signed
if errorlevel 1 (
    echo [ERROR] Could not change to ..\ARTS\BIN\release
    exit /b 1
)
for %%F in (
    tftpd64.exe
    tftpd32.exe
    tftpd64_gui.exe
    tftpd32_gui.exe
    tftpd64_svc.exe
    tftpd32_svc.exe
) do (
    if exist "%%F" (
        echo Signing %%F...
        %SIGNTOOL% sign /sha1 %SIGN_CERT_THUMBPRINT% /tr http://time.certum.pl /td sha256 /fd sha256 /v "%%F"
    ) else (
        echo [WARNING] File not found: %%F
    )
)

echo Done.
endlocal
