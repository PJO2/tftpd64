

!include "MUI2.nsh"
!include "x64.nsh"
!include "nsDialogs.nsh"

; ------------------------------------------------------------------
; Product Info
; ------------------------------------------------------------------
!define PRODUCT_NAME "Tftpd64"
!define PRODUCT_PUBLISHER "Ph. Jounin"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!ifndef PRODUCT_VERSION
  !define PRODUCT_VERSION "undefined"
!endif



Outfile "..\Releases\Tftpd64_Installer_v${PRODUCT_VERSION}.exe"
InstallDir "$PROGRAMFILES64\Tftpd64"
RequestExecutionLevel admin

Name "Tftpd64"
BrandingText "Tftpd64 Installer"
InstallDirRegKey HKLM "Software\Tftpd64" "InstallPath"


Var AllowFirewall
Var AddDesktopIcon
Var hChkFirewall
Var hChkDesktop



; ------------------------------------------------------------------
; Pages
; ------------------------------------------------------------------
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
Page custom FirewallPage
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_LANGUAGE "English"

; ------------------------------------------------------------------
; Install Section
; ------------------------------------------------------------------
Section "Install"

  SetOutPath "$INSTDIR"

  ; Install files
  File "..\ARTS\bin\dist\${PRODUCT_VERSION}\signed\tftpd64.exe"
  File "..\doc-help\tftpd32.chm"
  File "tftpd32.ini"
  File "EUPL-EN.pdf"

  ; Generate uninstaller
  WriteUninstaller "$INSTDIR\uninstall.exe"

  ; Shortcuts in Start Menu
  CreateDirectory "$SMPROGRAMS\Tftpd64"
  CreateShortCut "$SMPROGRAMS\Tftpd64\Tftpd64.lnk" "$INSTDIR\tftpd64.exe"
  CreateShortCut "$SMPROGRAMS\Tftpd64\Help.lnk"     "$INSTDIR\tftpd32.chm"
  CreateShortCut "$SMPROGRAMS\Tftpd64\Uninstall.lnk" "$INSTDIR\uninstall.exe"

  ; Desktop shortcut if selected
  ${If} $AddDesktopIcon == ${BST_CHECKED}
    CreateShortCut "$DESKTOP\Tftpd64.lnk" "$INSTDIR\tftpd64.exe"
  ${EndIf}

  ; Firewall rule if selected
  ${If} $AllowFirewall == ${BST_CHECKED}
    ExecWait 'netsh advfirewall firewall add rule name="Tftpd64" dir=in action=allow program="$INSTDIR\tftpd64.exe" enable=yes profile=any'
  ${EndIf}

  ; Add uninstall info
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_NAME}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\tftpd64.exe"

  ; Remember install location
  WriteRegStr HKLM "Software\Tftpd64" "InstallPath" "$INSTDIR"

SectionEnd

; ------------------------------------------------------------------
; Uninstall Section
; ------------------------------------------------------------------
Section "Uninstall"

  ; Delete shortcuts
  Delete "$DESKTOP\Tftpd64.lnk"
  Delete "$SMPROGRAMS\Tftpd64\Tftpd64.lnk"
  Delete "$SMPROGRAMS\Tftpd64\Help.lnk"
  Delete "$SMPROGRAMS\Tftpd64\Uninstall.lnk"
  RMDir  "$SMPROGRAMS\Tftpd64"

  ; Delete files
  Delete "$INSTDIR\tftpd64.exe"
  Delete "$INSTDIR\tftpd32.chm"
  Delete "$INSTDIR\tftpd32.ini"
  Delete "$INSTDIR\EUPL-EN.pdf"
  Delete "$INSTDIR\uninstall.exe"
  RMDir  "$INSTDIR"

  ; Delete registry keys
  DeleteRegKey HKLM "Software\Tftpd64"
  DeleteRegKey HKLM "${PRODUCT_UNINST_KEY}"

  ; Remove firewall rule
  ExecWait 'netsh advfirewall firewall delete rule name="Tftpd64"'

SectionEnd

; ------------------------------------------------------------------
; Custom Page for Firewall and Desktop Shortcut
; ------------------------------------------------------------------
Function FirewallPage
  nsDialogs::Create 1018
  Pop $0

  ${NSD_CreateCheckbox} 0u 0u 100% 12u "Allow Tftpd64 through Windows Firewall"
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
