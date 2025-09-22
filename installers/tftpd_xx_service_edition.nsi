!include "MUI2.nsh"
!include "x64.nsh"
!include "nsDialogs.nsh"

; ------------------------------------------------------------------
; Product Info - params : ARCH and PRODUCT_VERSION 
; ------------------------------------------------------------------
!define PRODUCT_NAME "Tftpd${ARCH}"
!define RELEASE_NAME "${PRODUCT_NAME} Service Edition"
!define PRODUCT_PUBLISHER "Ph. Jounin"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}SVC"
!ifndef PRODUCT_VERSION
  !define PRODUCT_VERSION "undefined"
!endif

Outfile "..\Releases\${PRODUCT_NAME}_Service_Installer_v${PRODUCT_VERSION}.exe"
InstallDir "$PROGRAMFILES${ARCH}\${PRODUCT_NAME}-SVC"
RequestExecutionLevel admin

Name "${RELEASE_NAME}"
BrandingText "${PRODUCT_NAME} Service Installer"
InstallDirRegKey HKLM "Software\${PRODUCT_NAME}SVC" "InstallPath"

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
  File "..\ARTS\bin\dist\${PRODUCT_VERSION}\signed\${PRODUCT_NAME}_gui.exe"
  File "..\ARTS\bin\dist\${PRODUCT_VERSION}\signed\${PRODUCT_NAME}_svc.exe"
  File "..\doc-help\tftpd32.chm"
  File "tftpd32.ini"
  File "EUPL-EN.pdf"

  ; Generate uninstaller
  WriteUninstaller "$INSTDIR\uninstall.exe"

  ; Start Menu shortcuts
  CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}-SVC"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}-SVC\GUI.lnk" "$INSTDIR\${PRODUCT_NAME}_gui.exe"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}-SVC\Help.lnk" "$INSTDIR\tftpd32.chm"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}-SVC\Uninstall.lnk" "$INSTDIR\uninstall.exe"

  ; Desktop shortcut (optional)
  ${If} $AddDesktopIcon == ${BST_CHECKED}
    CreateShortCut "$DESKTOP\${PRODUCT_NAME} GUI.lnk" "$INSTDIR\${PRODUCT_NAME}_gui.exe"
  ${EndIf}

  ; Firewall rule (optional)
  ${If} $AllowFirewall == ${BST_CHECKED}
    ExecWait 'netsh advfirewall firewall add rule name="${PRODUCT_NAME} SVC" dir=in action=allow program="$INSTDIR\${PRODUCT_NAME}_svc.exe" enable=yes profile=any'
  ${EndIf}

  ; Register the service (optional)
   ${If} $InstallService == ${BST_CHECKED}
    nsExec::Exec '"$INSTDIR\${PRODUCT_NAME}_svc.exe" -install'
      ${If} $StartService == ${BST_CHECKED}
         nsExec::ExecToLog 'net start "${SERVICE_NAME}"'
      ${EndIf}
   ${EndIf}

  ; Registry uninstall info
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayName" "${RELEASE_NAME}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\${PRODUCT_NAME}_gui.exe"

  ; Save install location
  WriteRegStr HKLM "Software\${PRODUCT_NAME}SVC" "InstallPath" "$INSTDIR"

SectionEnd

; ------------------------------------------------------------------
; Uninstall Section
; ------------------------------------------------------------------
Section "Uninstall"

  ; Stop and remove the service
  nsExec::ExecToLog 'sc stop "${SERVICE_NAME}"'
  nsExec::Exec  '"$INSTDIR\${PRODUCT_NAME}_svc.exe" -remove'

  ; Delete shortcuts
  Delete "$DESKTOP\${PRODUCT_NAME} GUI.lnk"
  Delete "$SMPROGRAMS\${PRODUCT_NAME}-SVC\GUI.lnk"
  Delete "$SMPROGRAMS\${PRODUCT_NAME}-SVC\Help.lnk"
  Delete "$SMPROGRAMS\${PRODUCT_NAME}-SVC\Uninstall.lnk"
  RMDir "$SMPROGRAMS\${PRODUCT_NAME}-SVC"

  ; Delete files
  Delete "$INSTDIR\${PRODUCT_NAME}_gui.exe"
  Delete "$INSTDIR\${PRODUCT_NAME}_svc.exe"
  Delete "$INSTDIR\tftpd32.chm"
  Delete "$INSTDIR\tftpd32.ini"
  Delete "$INSTDIR\EUPL-EN.pdf"
  Delete "$INSTDIR\uninstall.exe"
  RMDir "$INSTDIR"

  ; Remove firewall rule
  ExecWait 'netsh advfirewall firewall delete rule name="${PRODUCT_NAME} SVC"'

  ; Clean registry
  DeleteRegKey HKLM "Software\${PRODUCT_NAME}SVC"
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

  ${NSD_CreateCheckbox} 0u 16u 100% 12u "Create Desktop Shortcut for ${PRODUCT_NAME} GUI"
  Pop $hChkDesktop
  ${NSD_SetState} $hChkDesktop ${BST_CHECKED}
  ${NSD_GetState} $hChkDesktop $AddDesktopIcon
  ${NSD_OnClick} $hChkDesktop OnDesktopClicked

  ${NSD_CreateCheckbox} 0u 32u 100% 12u "Install and start ${PRODUCT_NAME} as a Windows Service"
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
