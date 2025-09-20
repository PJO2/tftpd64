@echo off
setlocal

REM -- Check for cert thumbprint --
if "%SIGN_CERT_THUMBPRINT%"=="" (
    echo [ERROR] SIGN_CERT_THUMBPRINT is not defined. Please set it before running this script.
    exit /b 1
)

REM -- Path to signtool --
set SIGNTOOL="C:\Program Files (x86)\Windows Kits\10\App Certification Kit\signtool.exe"
set RELEASE_DIR=..\releases

echo Signing only unsigned .exe files in %RELEASE_DIR% ...

pushd %RELEASE_DIR%

for %%F in (*%VERSION%.exe) do (
    echo Checking %%F ...
    powershell -Command ^
      "if ((Get-AuthenticodeSignature '%%F').Status -eq 'Valid') { exit 0 } else { exit 1 }"
    if not errorlevel 1 (
        echo [SKIP] %%F is already signed.
    ) else (
        echo [SIGN] %%F ...
        %SIGNTOOL% sign /sha1 %SIGN_CERT_THUMBPRINT% /tr http://time.certum.pl /td sha256 /fd sha256 /v "%%F"
        if errorlevel 1 (
            echo [ERROR] Signing failed for %%F
            exit /b 1
        )
    )
)

popd

echo Done.
endlocal
