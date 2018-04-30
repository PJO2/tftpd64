/////////////////////////////////////////////////////////
// 
// Tftpd32 March 2007
// 
// Gui definitions and functions
// 
/////////////////////////////////////////////////////////


//////////////////////////
// Commandes Application
//////////////////////////

#define  APPLICATION            "Tftpd32"
#define  VERSION                0x0321
#define  TFTPD_BCKG_CLASS       "Ttftpd32BackGround"
#define  TFTP_CLIENT_CLASS      "Ttftpd32ClientBackGround"
#define  TFTPD32_ADDIP_CLASS    "TftpAddIpBackGround"
#define  TFTPD32_BDCAST_STRING   "Tftpd32BroadcastString_Ark+"

#define TASKTRAY_ID (* (DWORD *) "Ark+")


/////////////////////////////////////////////////////////
// user defined message main window
enum {
       WM_RESIZE_MAIN_WINDOW = (WM_APP+100),
       WM_INIT_DISPLAY,
       WM_TIMER_HIDE_MAIN_WND,
	   WM_TIMER_REPOS_MAIN_WND,
       WM_FREEMEM,
       WM_RECV_FROM_THREAD,
	   WM_SOCKET_CLOSED,
       WM_SOCKET_ERROR,
       WM_TFTP_TRANFSER_TO_KILL,
       WM_DISPLAY_LISTEN,
       WM_NOTIFYTASKTRAY,
       WM_TFTP_CHG_WORKING_DIR,
       WM_SAVE_SETTINGS,
       WM_SAVE_DHCP_SETTINGS,
       WM_DELETE_ASSIGNATION,
	   WM_DESTROY_SETTINGS,
	   WM_START_SERVICES,

	   WM_CLOSE_2ND,
	   WM_CLOSE_3RD,
	   
	   WM_ESC_EDITBOX,
	   WM_TAB_EDITBOX,
	   WM_ENTER_EDITBOX,
	   
       WM_ANYBODY_HERE =(WM_APP + 591),
    };  // main window

// messages reçus par la fenêtre Serveur TFTP
enum {
        WM_TFTP_CREATE  = (WM_APP+200),
        WM_TFTPD_INIT,
        WM_TFTPD_START_THREADS,
        WM_TFTPD_THREADS_STARTED,
        WM_TFTPD_READY,
        WM_TFTPD_REPORTING,
        WM_ENDTHREAD,
        WM_CANFREEMEMORY,
        WM_NEWCONNECTION,

    };  // TFTP hidden window

// messages reçus par la fenêtre Serveur BOOTP
enum {
        WM_DHCP_INIT = (WM_APP+300),
        WM_DHCP_READCONFIG,
        WM_DHCP_MSG,
     }; // DHCP hidden window

// messages reçus par la fenêtre Serveur SYSLOG
enum {
        WM_SYSLOG_INIT = (WM_APP+400),
        WM_SYSLOG_MSG,
        WM_SYSLOG_INVALIDATE,
     }; // SYSLOG hidden window

// messages reçus par la fenêtre Serveur SNTP
enum {
        WM_SNTP_INIT = (WM_APP+500),
        WM_SNTP_MSG,
        WM_SNTP_INVALIDATE,
     }; // SNTP hidden window


// user defined message browse window
enum {
       WM_NEWCHOICE = (WM_USER+400),
};

// Message reçus par la fenêtre Gauge
enum {
       WM_NEWPOS = (WM_USER+500),
       WM_FILLWND,
};
// Message reçus par la fenêtre du Client TFTP
enum {
       WM_INITCLIENT = (WM_USER+600),
       WM_CLIENT_DATA ,
       WM_CLIENT_ACK,
};

// messages reçus par la fenêtre adresse IP
enum {
        WM_TFTP_GETIP = (WM_USER+700),
        WM_IPADDRESS,
};

// messages reçus par la fenêtre adresse CMsgBox
enum {
        WM_INIT_MSGBOX = (WM_USER+800),
        NEW_MSGBOX,
};

// Messages received by the AsyncSaveKey function
enum {
        WM_INIT_SAVEKEY = (WM_APP+450),
        WM_ASYNC_SAVEKEY,
     };

// Messages received by the AsyncSaveKey function
enum {
        WM_INIT_LOG = (WM_APP+500),
        WM_ASYNC_LOG,
     };
     
/////////////////////////////////////////////////////////
// Windows and Dialogs
// long CALLBACK TftpAddIPProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT  CALLBACK AboutProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT  CALLBACK TftpClientProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT  CALLBACK SettingsProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT  CALLBACK ShDirProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT  CALLBACK BrowseProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT  CALLBACK MsgBoxCbk (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT  CALLBACK AsyncSaveKeyProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT  CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


LRESULT  TR_ChangeTabControl (HWND hDlgWnd);	
int  TR_ChgTabControl (HWND hDlgWnd, int nService, int nStatus); // open one sub window
int TR_OpenAllTabs (HWND hDlgWnd, int nService);	// open all sub windows
DWORD TR_GetCurrentTab (HWND hDlgWnd);		// get service of current tab

int  FillCBLocalIP (HWND hCBWnd, BOOL bShowPassive, const char *szAddress);
BOOL Tftpd32RetrieveWindowPos (HWND hMainWnd);
BOOL Tftpd32SaveWindowPos (HWND hMainWnd);


void SecFileName (char *s);
BOOL SecAllowSecurity (const char *szFile, int op_code);

int ProcessCmdLine (LPSTR lpszCmdLine);
void SetHourglass (BOOL);
BOOL getHourglass();

int TR_MinMaxInfo (HWND hDlgWnd, LPMINMAXINFO lpInfo);
int TR_ResizeWindow (HWND hDlgWnd, BOOL bInit);

BOOL MyBrowseWindow (HWND hWnd, LPSTR szBrowsePath, BOOL bOpenCurDir);


int TftpDir_AddEntry (HWND hCBWnd, const char *szPath);
int TftpDir_Synchronize (HWND hCBWnd);
int TftpDir_SelectEntry (HWND hCBWnd);
char *GetActiveDirectory (char *szActiveDirectory, int nSize);
int StartExplorer (void);

// gui_main 
int ChangeIPAddress (HWND hWnd, struct S_IPAddressList *pIf);


// gui_bootpd
void DhcpRefresh_ListView ( int nbLeases, struct S_Lease tLeases[] );


// gui_syslogd
void AddSyslogItem (HWND hListV, const char *szIP, const char *szTxt);

// gui_dns
void AddDNSItem (HWND hListV, char *szName, char *szIPv4, char *szIPv6);

// exported by gui_gauges.c
int CALLBACK Gui_TftpGaugeProc (HWND hWnd, UINT message, WPARAM wParam, LONG lParam);
HWND Gui_CreateGaugeWindow (HWND hParentWnd, const struct S_TftpGui *pTftpGui);
void Gui_UpdateGaugeWindow (const struct S_TftpGui *pTftpGui, time_t dNow);

// exported by gui_recvmsg.c
const struct S_TftpGui *Gui_GetFirstGuiItem (void);
int Gui_GetMessage (HWND hWnd, SOCKET sService, int bBlocking, int nMsgType);
int Gui_AbortTftpTransfer (SOCKET sService, DWORD dwTransferId);


// from gui_log.c
void LB_LOG (HWND hListBox, const char *szTxt);

// from gui_tftpd.c
long CALLBACK TftpProc (HWND hWnd, UINT message, WPARAM wParam, LONG lParam);
int Gui_TftpReporting (HWND hListV, const struct S_TftpGui *pTftpGuiFirst);
BOOL GuiIsActiveTFTPTransfer (HWND hMainWnd);

// from gui_dialog
int IsGuiConnectedToRemoteService (void);

int Gui_AbortTftpTransfer (SOCKET sService, DWORD dwTransferId);
int Gui_StopTftpService (SOCKET sService);
int Gui_StopDnsService (SOCKET sService);
int Gui_StopSyslogService (SOCKET sService);
int Gui_StopSntpService (SOCKET sService);

int Gui_StopDhcpService (SOCKET sService);
int Gui_StopAllServices (SOCKET sService);
int Gui_SuspendServices (SOCKET sService);
int Gui_StartAllServices (SOCKET sService);
int Gui_AskDHCPSettings (SOCKET sService);
int Gui_AskTFTPSettings (SOCKET sService);
int Gui_ChangeWorkingDirectory (SOCKET sService, const char *szNewDir);
int Gui_SaveSettings (SOCKET sService, struct S_Tftpd32Settings *pset);
int Gui_SaveDhcpSettings (SOCKET sService, struct S_DHCP_Param *pset);
int Gui_RequestWorkingDir (SOCKET sService);
int Gui_RequestRunningServices (SOCKET sService);
int Gui_SuppressDHCPAllocation (SOCKET sService, unsigned ip);
int Gui_DestroySettings (SOCKET sService);
int Gui_RequestIPInterfaces (SOCKET sService);
int Gui_RequestListDirectory (SOCKET sService);
int Gui_RequestFullReport (SOCKET sService);
int Gui_GetMessage (HWND hWnd, SOCKET sService, int bBlocking, int nMsgType);

// gui_bootpd_settings
int Gui_LoadDHCPConfig (HWND hWnd);
int Gui_DHCPSaveConfig (HWND hWnd);

int DhcpServerListPopup (HWND hListV);
int TftpServerListPopup (HWND hListV);
int InitReportingListView (HWND hListV);
void AddSyslogItem (HWND hListV, const char *szIP, const char *szTxt);
