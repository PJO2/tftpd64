!include "MUI2.nsh"
!include "nsDialogs.nsh"

; ------------------------------------------------------------------
; Metadata
; ------------------------------------------------------------------
!define PRODUCT_NAME "Tftpd32"
!define PRODUCT_PUBLISHER "Ph. Jounin"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!ifndef PRODUCT_VERSION
  !define PRODUCT_VERSION "undefined"
!endif

Outfile "..\Releases\Tftpd32_Installer_v${PRODUCT_VERSION}.exe"
InstallDir "$PROGRAMFILES32\${PRODUCT_NAME}"
RequestExecutionLevel admin

Name "Tftpd32"
BrandingText "Tftpd32 Installer"
InstallDirRegKey HKLM "Software\Tftpd32" "InstallPath"

Var AllowFirewall
Var hChkFirewall
Var AddDesktopIcon
Var hChkDesktop


; ------------------------------------------------------------------
; Page order
; ------------------------------------------------------------------
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
Page custom FirewallPage
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_LANGUAGE "English"

; ------------------------------------------------------------------
; Installation section
; ------------------------------------------------------------------
Section "Install"

  SetOutPath "$INSTDIR"

  File "..\ARTS\bin\dist\${PRODUCT_VERSION}\signed\tftpd32.exe"
  File "..\doc-help\tftpd32.chm"
  File "EUPL-EN.pdf"

  ; Shortcuts
  CreateDirectory "$SMPROGRAMS\Tftpd32"
  CreateShortCut "$SMPROGRAMS\Tftpd32\Tftpd32.lnk" "$INSTDIR\tftpd32.exe"
  CreateShortCut "$SMPROGRAMS\Tftpd32\Help.lnk"     "$INSTDIR\tftpd32.chm"
  ${If} $AddDesktopIcon == ${BST_CHECKED}
     CreateShortCut "$DESKTOP\Tftpd32.lnk" "$INSTDIR\tftpd32.exe"
  ${EndIf}

  ; create ini file in %AppData%tftpd32
  ; Make sure $AppData resolves for the CURRENT user (not all users)
  SetShellVarContext current
  ; Target dir and file
  StrCpy $0 "$AppData\${PRODUCT_NAME}"
  CreateDirectory "$0"
  ; --- MIGRATION (legacy -> AppData) ---
  ; If an old INI sits in $INSTDIR and there isn’t one yet in %AppData%, move it.
  IfFileExists "$INSTDIR\tftpd32.ini" 0 +5
     IfFileExists "$0\tftpd32.ini" +4 0
        DetailPrint "Migrating settings to $0\tftpd32.ini"
        CopyFiles /SILENT "$INSTDIR\tftpd32.ini" "$0\tftpd32.ini"
        Delete "$INSTDIR\tftpd32.ini"
  ; If there’s still no INI in %AppData%, drop the shipped default there.
  IfFileExists "$0\tftpd32.ini" 0 +2
    File "/oname=$0\tftpd32.ini" "tftpd32.ini"
  ; let the app know where the INI is (only if your app can read it)
  WriteRegStr HKLM "Software\${PRODUCT_NAME}" "IniPath" "$0\tftpd32.ini"

  ; Firewall rule if requested
  ${If} $AllowFirewall == ${BST_CHECKED}
    ExecWait 'netsh advfirewall firewall add rule name="Tftpd32" dir=in action=allow program="$INSTDIR\tftpd32.exe" enable=yes profile=any'
  ${EndIf}

  ; Uninstall registry
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_NAME}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\tftpd32.exe"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninstall.exe"

  ; Save install path
  WriteRegStr HKLM "Software\${PRODUCT_NAME}" "InstallPath" "$INSTDIR"

  WriteUninstaller "$INSTDIR\uninstall.exe"

SectionEnd


Section "Uninstall"

  ; Remove shortcuts
  Delete "$DESKTOP\Tftpd32.lnk"
  Delete "$SMPROGRAMS\Tftpd32\Tftpd32.lnk"
  Delete "$SMPROGRAMS\Tftpd32\Help.lnk"
  Delete "$SMPROGRAMS\Tftpd32\Uninstall.lnk"
  RMDir  "$SMPROGRAMS\Tftpd32"

  ; Remove installed files
  Delete "$INSTDIR\tftpd32.exe"
  Delete "$INSTDIR\tftpd32.chm"
  Delete "$INSTDIR\EUPL-EN.pdf"
  Delete "$INSTDIR\uninstall.exe"
  RMDir  "$INSTDIR"

  ; Remove ini
  SetShellVarContext current
  Delete "$AppData\${PRODUCT_NAME}\tftpd32.ini"
  RMDir  "$AppData\${PRODUCT_NAME}"

  ; Clean registry
  DeleteRegKey HKLM "Software\${PRODUCT_NAME}"
  DeleteRegKey HKLM "${PRODUCT_UNINST_KEY}"

  ; Remove firewall rule
  ExecWait 'netsh advfirewall firewall delete rule name="${PRODUCT_NAME}"'

SectionEnd


; ------------------------------------------------------------------
; Custom firewall checkbox page
; ------------------------------------------------------------------
Function FirewallPage
  nsDialogs::Create 1018
  Pop $0
  ${NSD_CreateCheckbox} 0u 0u 100% 12u "Allow Tftpd32 through Windows Firewall"
  Pop $hChkFirewall
  ${NSD_SetState} $hChkFirewall ${BST_CHECKED}
  ${NSD_OnClick} $hChkFirewall FirewallClicked

  ${NSD_CreateCheckbox} 0u 16u 100% 12u "Create Desktop Shortcut"
  Pop $hChkDesktop
  ${NSD_SetState} $hChkDesktop ${BST_CHECKED}
  ${NSD_OnClick} $hChkDesktop DesktopClicked

  nsDialogs::Show
FunctionEnd

Function FirewallClicked
  ${NSD_GetState} $hChkFirewall $AllowFirewall
FunctionEnd

Function DesktopClicked
  ${NSD_GetState} $hChkDesktop $AddDesktopIcon
FunctionEnd

