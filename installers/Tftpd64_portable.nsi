Outfile "Create_Portable_64_Zip.exe"
RequestExecutionLevel admin

Name "Tftpd64 Portable ZIP Builder"

!define PRODUCT_VERSION "4.70"
!define ZIP_NAME "Tftpd64_Portable_v${PRODUCT_VERSION}.zip"
!define ZIP_PATH "$EXEDIR\..\Releases\${ZIP_NAME}"
!define ZIP7 "C:\Program Files\7-Zip\7z.exe"

Section "Create ZIP"

  ; Copy and rename the binary from BIN folder
  CopyFiles "$EXEDIR\..\BIN\tftpd32.x64.exe" "$EXEDIR\tftpd64.exe"

  ; Clean up existing ZIP if it exists
  Delete "${ZIP_PATH}"

  ; Create the ZIP with renamed binary
  ExecWait '${ZIP7} a -tzip "${ZIP_PATH}" "$EXEDIR\tftpd64.exe" "$EXEDIR\tftpd32.chm" "$EXEDIR\tftpd32.ini"  "$EXEDIR\EUPL-EN.pdf" "$EXEDIR\..\doc-help\tftpd32.chm"'

  ; Delete renamed binary after zipping (optional)
  Delete "$EXEDIR\tftpd64.exe"

  ; Notify
  MessageBox MB_OK "Portable archive created: ${ZIP_NAME}"

SectionEnd