//////////////////////////////////////////////////////
//
// Projet TFTPD32.   April 2007 Ph.jounin
// File service stuff.c:  services procedures
//
// derivative work from sdk_service.cpp 
//                 by Craig Link - Microsoft Developer Support
// 
// source released under artistic license (see license.txt)
//
//////////////////////////////////////////////////////


#define SZSERVICEDISPLAYNAME  "Tftpd32 service edition"
#define SZSERVICENAME		  "Tftpd32_svc"
#define SZDEPENDENCIES        NULL
#define SZSERVDESCRIPTION     "Tftpd32 operates a TFTP server"

#define _tprintf printf
#define _stprintf sprintf

// internal variables
extern BOOL                    bDebug ;

// internal function prototypes
VOID WINAPI service_ctrl(DWORD dwCtrlCode);
VOID WINAPI service_main(DWORD dwArgc, LPTSTR *lpszArgv);
VOID CmdInstallService();
VOID CmdRemoveService();
VOID CmdDebugService();
BOOL WINAPI ControlHandler ( DWORD dwCtrlType );
LPTSTR GetLastErrorText( LPTSTR lpszBuf, DWORD dwSize );
BOOL ReportStatusToSCMgr(DWORD dwCurrentState,
                         DWORD dwWin32ExitCode,
                         DWORD dwWaitHint);
void ServiceStart (void);
void ServiceStop (void);
void AddToMessageLog(LPTSTR lpszMsg); // to be replaced withWriteIntoEventlog
