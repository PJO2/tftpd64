
!include "MUI2.nsh"
!include "x64.nsh"
!include "nsDialogs.nsh"

; ------------------------------------------------------------------
; Product Info - params : ARCH and PRODUCT_VERSION 
; ------------------------------------------------------------------
!define PRODUCT_NAME "Tftpd${ARCH}"
!define PRODUCT_PUBLISHER "Ph. Jounin"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!ifndef PRODUCT_VERSION
  !define PRODUCT_VERSION "undefined"
!endif

Outfile "..\Releases\${PRODUCT_NAME}_Installer_v${PRODUCT_VERSION}.exe"
InstallDir "$PROGRAMFILES${ARCH}\${PRODUCT_NAME}"
RequestExecutionLevel admin

Name "${PRODUCT_NAME}"
BrandingText "${PRODUCT_NAME} Installer"
InstallDirRegKey HKLM "Software\${PRODUCT_NAME}" "InstallPath"

Var AllowFirewall
Var AddDesktopIcon
Var hChkFirewall
Var hChkDesktop


; ------------------------------------------------------------------
; Page order
; ------------------------------------------------------------------
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
; add a leave function to the custom page
Page custom FirewallPage FirewallPageLeave
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_LANGUAGE "English"

; ------------------------------------------------------------------
; Installation section
; ------------------------------------------------------------------
Section "Install"

  SetOutPath "$INSTDIR"

  ; Install files
  File "..\ARTS\bin\dist\${PRODUCT_VERSION}\signed\${PRODUCT_NAME}.exe"
  File "..\doc-help\tftpd32.chm"
  File "EUPL-EN.pdf"


  ; Generate uninstaller
  WriteUninstaller "$INSTDIR\uninstall.exe"

  ; Shortcuts in Start Menu
  SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk" "$INSTDIR\${PRODUCT_NAME}.exe"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Help.lnk"            "$INSTDIR\tftpd32.chm"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall.lnk"       "$INSTDIR\uninstall.exe"

  ; Desktop shortcut if selected
  ${If} $AddDesktopIcon == ${BST_CHECKED}
    SetShellVarContext current
    CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "$INSTDIR\${PRODUCT_NAME}.exe"
    SetShellVarContext all ; restore for the rest of the install
  ${EndIf}

  ; Firewall rule if requested
  ${If} $AllowFirewall == ${BST_CHECKED}
    ExecWait 'netsh advfirewall firewall add rule name="${PRODUCT_NAME}" dir=in action=allow program="$INSTDIR\${PRODUCT_NAME}.exe" enable=yes profile=any'
  ${EndIf}

  ; ----------------------------------------------
  ; create ini file in %AppData%${PRODUCT_NAME}
  ; ----------------------------------------------
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
  IfFileExists "$0\tftpd32.ini" +2 0
    File "/oname=$0\tftpd32.ini" "tftpd32.ini"
  ; let the app know where the INI is (only if your app can read it)
  WriteRegStr HKLM "Software\${PRODUCT_NAME}" "IniPath" "$0\tftpd32.ini"
  ; ----------------------------------------------

  ; Add uninstall info
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_NAME}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\${PRODUCT_NAME}.exe"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninstall.exe"

  ; Save install path
  WriteRegStr HKLM "Software\${PRODUCT_NAME}" "InstallPath" "$INSTDIR"

SectionEnd


; ------------------------------------------------------------------
; Uninstall Section
; ------------------------------------------------------------------
Section "Uninstall"

  SetShellVarContext all
  Delete "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk"
  Delete "$SMPROGRAMS\${PRODUCT_NAME}\Help.lnk"
  Delete "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall.lnk"
  RMDir  "$SMPROGRAMS\${PRODUCT_NAME}"

  ; Delete shortcuts
  SetShellVarContext current
  Delete "$DESKTOP\${PRODUCT_NAME}.lnk"
  SetShellVarContext all
  Delete "$DESKTOP\${PRODUCT_NAME}.lnk"

  ; Remove installed files
  Delete "$INSTDIR\${PRODUCT_NAME}.exe"
  Delete "$INSTDIR\tftpd32.chm"
  Delete "$INSTDIR\tftpd32.ini"
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
; Custom Page for Firewall and Desktop Shortcut
; ------------------------------------------------------------------

Function FirewallPage
  nsDialogs::Create 1018
  Pop $0

  ${NSD_CreateCheckbox} 0u 0u 100% 12u "Allow ${PRODUCT_NAME} through Windows Firewall"
  Pop $hChkFirewall
  ${NSD_SetState} $hChkFirewall ${BST_CHECKED}

  ${NSD_CreateCheckbox} 0u 16u 100% 12u "Create Desktop Shortcut"
  Pop $hChkDesktop
  ${NSD_SetState} $hChkDesktop ${BST_CHECKED}

  nsDialogs::Show
FunctionEnd

Function FirewallPageLeave
  ${NSD_GetState} $hChkFirewall $AllowFirewall
  ${NSD_GetState} $hChkDesktop  $AddDesktopIcon
FunctionEnd

