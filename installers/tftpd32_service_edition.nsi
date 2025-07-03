!include "MUI2.nsh"
!include "x64.nsh"
!include "nsDialogs.nsh"

; ------------------------------------------------------------------
; Product Info
; ------------------------------------------------------------------
!define PRODUCT_NAME "Tftpd32 Service Edition"
!define PRODUCT_PUBLISHER "Ph. Jounin"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tftpd32SVC"
!ifndef PRODUCT_VERSION
  !define PRODUCT_VERSION "undefined"
!endif

Outfile "..\Releases\Tftpd32_Service_Installer_v${PRODUCT_VERSION}.exe"
InstallDir "$PROGRAMFILES32\Tftpd32-SVC"
RequestExecutionLevel admin

Name "Tftpd32 Service Edition"
BrandingText "Tftpd32 Service Installer"
InstallDirRegKey HKLM "Software\Tftpd32SVC" "InstallPath"

; service name defined also in the executable
!define SERVICE_NAME "Tftpd32_svc"

Var AllowFirewall
Var AddDesktopIcon
Var InstallService
Var hChkFirewall
Var hChkDesktop
Var hChkService
Var StartService
Var hChkStartSvc

; ------------------------------------------------------------------
; Pages
; ------------------------------------------------------------------
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
Page custom OptionsPage
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_LANGUAGE "English"

; ------------------------------------------------------------------
; Install Section
; ------------------------------------------------------------------
Section "Install"

  SetOutPath "$INSTDIR"

  ; Install files
  File "..\ARTS\bin\signed\tftpd32_gui.exe"
  File "..\ARTS\bin\signed\tftpd32_svc.exe"
  File "..\doc-help\tftpd32.chm"
  File "tftpd32.ini"
  File "EUPL-EN.pdf"

  ; Generate uninstaller
  WriteUninstaller "$INSTDIR\uninstall.exe"

  ; Start Menu shortcuts
  CreateDirectory "$SMPROGRAMS\Tftpd32-SVC"
  CreateShortCut "$SMPROGRAMS\Tftpd32-SVC\GUI.lnk" "$INSTDIR\tftpd32_gui.exe"
  CreateShortCut "$SMPROGRAMS\Tftpd32-SVC\Help.lnk" "$INSTDIR\tftpd32.chm"
  CreateShortCut "$SMPROGRAMS\Tftpd32-SVC\Uninstall.lnk" "$INSTDIR\uninstall.exe"

  ; Desktop shortcut (optional)
  ${If} $AddDesktopIcon == ${BST_CHECKED}
    CreateShortCut "$DESKTOP\Tftpd32 GUI.lnk" "$INSTDIR\tftpd32_gui.exe"
  ${EndIf}

  ; Firewall rule (optional)
  ${If} $AllowFirewall == ${BST_CHECKED}
    ExecWait 'netsh advfirewall firewall add rule name="Tftpd32 SVC" dir=in action=allow program="$INSTDIR\tftpd32_svc.exe" enable=yes profile=any'
  ${EndIf}

  ; Register the service (optional)
   ${If} $InstallService == ${BST_CHECKED}
    nsExec::Exec '"$INSTDIR\tftpd32_svc.exe" -install'
      ${If} $StartService == ${BST_CHECKED}
         nsExec::ExecToLog 'net start "${SERVICE_NAME}"'
      ${EndIf}
   ${EndIf}

  ; Registry uninstall info
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_NAME}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\tftpd32_gui.exe"

  ; Save install location
  WriteRegStr HKLM "Software\Tftpd32SVC" "InstallPath" "$INSTDIR"

SectionEnd

; ------------------------------------------------------------------
; Uninstall Section
; ------------------------------------------------------------------
Section "Uninstall"

  ; Stop and remove the service
  nsExec::ExecToLog 'sc stop "${SERVICE_NAME}"'
  nsExec::Exec  '"$INSTDIR\tftpd32_svc.exe" -remove'

  ; Delete shortcuts
  Delete "$DESKTOP\Tftpd32 GUI.lnk"
  Delete "$SMPROGRAMS\Tftpd32-SVC\GUI.lnk"
  Delete "$SMPROGRAMS\Tftpd32-SVC\Help.lnk"
  Delete "$SMPROGRAMS\Tftpd32-SVC\Uninstall.lnk"
  RMDir "$SMPROGRAMS\Tftpd32-SVC"

  ; Delete files
  Delete "$INSTDIR\tftpd32_gui.exe"
  Delete "$INSTDIR\tftpd32_svc.exe"
  Delete "$INSTDIR\tftpd32.chm"
  Delete "$INSTDIR\tftpd32.ini"
  Delete "$INSTDIR\EUPL-EN.pdf"
  Delete "$INSTDIR\uninstall.exe"
  RMDir "$INSTDIR"

  ; Remove firewall rule
  ExecWait 'netsh advfirewall firewall delete rule name="Tftpd32 SVC"'

  ; Clean registry
  DeleteRegKey HKLM "Software\Tftpd32SVC"
  DeleteRegKey HKLM "${PRODUCT_UNINST_KEY}"

SectionEnd

; ------------------------------------------------------------------
; Custom Options Page (checkboxes)
; ------------------------------------------------------------------
Function OptionsPage
  nsDialogs::Create 1018
  Pop $0

  ${NSD_CreateCheckbox} 0u 0u 100% 12u "Allow Service through Windows Firewall"
  Pop $hChkFirewall
  ${NSD_SetState} $hChkFirewall ${BST_CHECKED}
  ${NSD_GetState} $hChkFirewall $AllowFirewall 
  ${NSD_OnClick} $hChkFirewall OnFirewallClicked

  ${NSD_CreateCheckbox} 0u 16u 100% 12u "Create Desktop Shortcut for Tftpd32 GUI"
  Pop $hChkDesktop
  ${NSD_SetState} $hChkDesktop ${BST_CHECKED}
  ${NSD_GetState} $hChkDesktop $AddDesktopIcon
  ${NSD_OnClick} $hChkDesktop OnDesktopClicked

  ${NSD_CreateCheckbox} 0u 32u 100% 12u "Install and start Tftpd32 as a Windows Service"
  Pop $hChkService
  ${NSD_SetState} $hChkService ${BST_CHECKED}
  ${NSD_GetState} $hChkService $InstallService
  ${NSD_OnClick} $hChkService OnServiceClicked

  ${NSD_CreateCheckbox} 0u 48u 100% 12u "Start the service after registration"
  Pop $hChkStartSvc
  ${NSD_SetState} $hChkStartSvc ${BST_CHECKED}
  ${NSD_GetState} $hChkStartSvc $StartService
  ${NSD_OnClick} $hChkStartSvc OnStartSvcClicked

  nsDialogs::Show
FunctionEnd

Function OnFirewallClicked
  ${NSD_GetState} $hChkFirewall $AllowFirewall
FunctionEnd

Function OnDesktopClicked
  ${NSD_GetState} $hChkDesktop $AddDesktopIcon
FunctionEnd

Function OnServiceClicked
  ${NSD_GetState} $hChkService $InstallService
FunctionEnd

Function OnStartSvcClicked
  ${NSD_GetState} $hChkStartSvc $StartService
FunctionEnd
