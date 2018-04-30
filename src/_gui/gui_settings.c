//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Mai 98 Ph.jounin
// File tftp_sec.c:   Settings
//
//////////////////////////////////////////////////////

// registry key :
//       HKEY_LOCAL_MACHINE\SOFTWARE\TFTPD32



// some shortcurts
#define ISDLG_CHECKED(hWnd,Ctrl) \
 (SendDlgItemMessage (hWnd, Ctrl, BM_GETCHECK, 0, 0) == BST_CHECKED)
#define CHECK_DLG_IF(hWnd, Ctrl, Flag) \
     SendDlgItemMessage (hWnd, Ctrl, BM_SETCHECK, (Flag) ? BST_CHECKED : BST_UNCHECKED, 0);
#define UNCHECK_DLG(hWnd, Ctrl) \
     SendDlgItemMessage (hWnd, Ctrl, BM_SETCHECK, BST_UNCHECKED, 0);


#include <stdio.h>		// use of sscanf
#include "headers.h"


#ifdef _M_X64
#  define TFTPD_SETTINGS_TITLE   "Tftpd64: Settings"
#else
#  define TFTPD_SETTINGS_TITLE   "Tftpd32: Settings"
#endif


#define  RESET_DEFAULT_TEXT  "Reset current configuration\nand destroy registry entries ?"


// The tabs are managed as in gui_move_win.c : The widget are hidden or shown according to the active tab
// Note that the widget are not resizeable !


// The tabs
enum { ONGLET_SETTINGS_GLOBAL = 0,
	   ONGLET_SETTINGS_TFTP,
       ONGLET_SETTINGS_DHCP,
       ONGLET_SETTINGS_SYSLOG,
       NB_ONGLETS };
// tab masks : 1>>Onglet
#define TAB_SETTINGS_GLOBAL   MakeMask(ONGLET_SETTINGS_GLOBAL)
#define TAB_SETTINGS_TFTP     MakeMask(ONGLET_SETTINGS_TFTP)
#define TAB_SETTINGS_SYSLOG   MakeMask(ONGLET_SETTINGS_SYSLOG)
#define TAB_SETTINGS_DHCP     MakeMask(ONGLET_SETTINGS_DHCP)
#define TAB_NONE              MakeMask(NB_ONGLETS)
#define TAB_ALL               (DWORD) -1



static struct S_Show
{
    DWORD  idCtrl;                      // The control ID
    LONG   x, y, width, height;         // size of the window at "normal" size
    DWORD  mView;                       // windows is displayed for Tabs ?
}
tResize [] =
{
	// GLOBAL 
	{	IDC_GRP_GLOBAL,              6, 36,228, 86,   TAB_SETTINGS_GLOBAL,  },
	{	IDC_CHECK_TFTP_SERVER,      14, 48, 60, 10,   TAB_SETTINGS_GLOBAL,  },    
	{	IDC_CHECK_TFTP_CLIENT,      14, 60, 60, 10,   TAB_SETTINGS_GLOBAL,  },
	{	IDC_CHECK_SNTP_SERVER,      14, 72, 60, 10,   TAB_SETTINGS_GLOBAL,  },
	{	IDC_CHECK_SYSLOG_SERVER,    14, 84, 60, 10,   TAB_SETTINGS_GLOBAL,  },
	{	IDC_CHECK_DHCP_SERVER,      14, 96, 60, 10,   TAB_SETTINGS_GLOBAL,  },
	{	IDC_CHECK_DNS_SERVER,       14,108, 60, 10,   TAB_SETTINGS_GLOBAL,  },
	// IPv6
	{	IDC_GRP_GLOBAL_IPv6,         6, 126,228, 26,   TAB_SETTINGS_GLOBAL,  },
	{	IDC_CHECK_IPv6,             14, 138, 60, 10,   TAB_SETTINGS_GLOBAL,  },



	// TFTP
	{	IDC_GRP_TFTP,                7,  30,228, 30,   TAB_SETTINGS_TFTP,  },
	{	IDC_BASEDIR,                14,  41,164, 12,   TAB_SETTINGS_TFTP,  },
	{	IDC_BUTTON_BROWSE,         185,  41, 39, 13,   TAB_SETTINGS_TFTP,  },

	{	IDC_GRP_TFTP_SEC,            6,  68, 78, 57,   TAB_SETTINGS_TFTP,  },
	{	IDC_RADIO_SECNONE,          14,  77, 65,  8,   TAB_SETTINGS_TFTP,  },
	{	IDC_RADIO_SECSTD,           14,  89, 65,  8,   TAB_SETTINGS_TFTP,  },
	{	IDC_RADIO_SECHIGH,          14, 101, 65,  8,   TAB_SETTINGS_TFTP,  },
	{	IDC_RADIO_SECRO,            14, 113, 65,  8,   TAB_SETTINGS_TFTP,  },

	{	IDC_GRP_TFTP_CFG,           91,  68,142, 57,   TAB_SETTINGS_TFTP,  },
	{	IDC_TXT_TFTP_TIMEOUT,       98,  77, 71, 10,   TAB_SETTINGS_TFTP,  },
	{	IDC_TXT_TFTP_RETRANSMIT,    98,  89, 71, 10,   TAB_SETTINGS_TFTP,  },
	{	IDC_TXT_TFTP_PORT,          98, 101, 68, 10,   TAB_SETTINGS_TFTP,  },
	{	IDC_TXT_TFTP_PORTS,         98, 113, 70, 10,   TAB_SETTINGS_TFTP,  },
	{	IDC_TIMEOUT,               174,  76, 50, 12,   TAB_SETTINGS_TFTP,  },
	{	IDC_MAXRETRANSMIT,         174,  87, 50, 12,   TAB_SETTINGS_TFTP,  },
	{	IDC_PORT,                  174,  99, 50, 12,   TAB_SETTINGS_TFTP,  },
	{	IDC_LOCAL_PORTS,           174, 110, 50, 12,   TAB_SETTINGS_TFTP,  },

	{	IDC_GRP_TFTP_ADVANCED,       6, 132,228,147,   TAB_SETTINGS_TFTP,  },
	{	IDC_CHECK_NEGOCIATE,        13, 143, 83,  8,   TAB_SETTINGS_TFTP,  },
	{	IDC_CHECK_PXE,              13, 155, 83,  8,   TAB_SETTINGS_TFTP,  },
	{	IDC_CHECK_PROGRESS,         13, 167, 99,  8,   TAB_SETTINGS_TFTP,  },
	{	IDC_CHECK_UNIX,             13, 179,104,  8,   TAB_SETTINGS_TFTP,  },
	{	IDC_CHECK_TFTPLOCALIP,      13, 191,103,  8,   TAB_SETTINGS_TFTP,  },
	{	IDC_CB_TFTPLOCALIP,        118, 191, 85, 45,   TAB_SETTINGS_TFTP,  },
  	{   IDC_CHECK_VROOT,            13, 203,104,  8,   TAB_SETTINGS_TFTP,  },
  	{   IDC_CHECK_WINSIZE,          13, 215,101,  8,   TAB_SETTINGS_TFTP,  },
  	{   IDC_WINSIZE,               118, 213, 24, 12,   TAB_SETTINGS_TFTP,  },
 	{   IDC_TXT_WINSIZE,           147, 215, 61,  8,   TAB_SETTINGS_TFTP,  },
	{	IDC_CHECK_HIDE,             13, 227, 90,  8,   TAB_SETTINGS_TFTP,  },
  	{   IDC_CHECK_DIRTEXT,          13, 239, 90,  8,   TAB_SETTINGS_TFTP,  },
  	{   IDC_CHECK_MD5,              13, 251, 90,  8,   TAB_SETTINGS_TFTP,  },
  	{   IDC_CHECK_BEEP,             13, 263, 90,  8,   TAB_SETTINGS_TFTP,  },
	
	// DHCP tab
 	{   IDC_GRP_DHCP,                6, 213,228, 61,   TAB_SETTINGS_DHCP,  },
 	{   IDC_CHECK_PING,             12, 222,115, 10,   TAB_SETTINGS_DHCP,  },
  	{   IDC_CHECK_PERS_LEASES,      12, 233, 72, 10,   TAB_SETTINGS_DHCP,  },
  	{   IDC_CHECK_DOUBLE_ANSWER,    12, 244,115, 10,   TAB_SETTINGS_DHCP,  },
 	{   IDC_CHECK_LOCALIP_DHCP,     12, 255,114, 10,   TAB_SETTINGS_DHCP,  },
 	{   IDC_CB_LOCALIP_DHCP,       139, 255, 85, 45,   TAB_SETTINGS_DHCP,  },

 	{   IDC_GRP_DHCP_POOL,          6,  21,228,   63, TAB_SETTINGS_DHCP,  },
    {   IDC_TXT_ADDRESS_POOL,      10,  33, 75,   10, TAB_SETTINGS_DHCP,  },
    {   IDC_DHCP_ADDRESS_POOL,     85,  33, 70,   10, TAB_SETTINGS_DHCP,  },
    {   IDC_TXT_POOL_SIZE,         10,  45, 60,   10, TAB_SETTINGS_DHCP,  },
    {   IDC_DHCP_POOL_SIZE,        85,  45, 20,   10, TAB_SETTINGS_DHCP,  },
    {   IDC_TXT_LEASE,		       10,  57, 60,   10, TAB_SETTINGS_DHCP,  },
    {   IDC_DHCP_LEASE,            85,  57, 30,   10, TAB_SETTINGS_DHCP,  },
    {   IDC_TXT_BOOT_FILE,         10,  70, 65,   10, TAB_SETTINGS_DHCP,  },
    {   IDC_DHCP_BOOT_FILE,        85,  70, 145,  10, TAB_SETTINGS_DHCP,  },

	{   IDC_GRP_DHCP_OPT,          6,   89,228,  110, TAB_SETTINGS_DHCP,  },
    {   IDC_TXT_DEFAULT_ROUTER,    10,  98, 75,   10, TAB_SETTINGS_DHCP,  },
    {   IDC_DHCP_DEFAULT_ROUTER,   85,  98, 70,   10, TAB_SETTINGS_DHCP,  },
    {   IDC_TXT_MASK,              10, 111, 75,   10, TAB_SETTINGS_DHCP,  },
    {   IDC_DHCP_MASK,             85, 111, 70,   10, TAB_SETTINGS_DHCP,  },
    {   IDC_TXT_DNS_SERVER,        10, 123, 75,   10, TAB_SETTINGS_DHCP,  },
    {   IDC_DHCP_DNS_SERVER,       85, 123, 70,   10, TAB_SETTINGS_DHCP,  },
    {   IDC_TXT_WINS_SERVER,       10, 135, 75,   10, TAB_SETTINGS_DHCP,  },
    {   IDC_DHCP_WINS_SERVER,      85, 135, 70,   10, TAB_SETTINGS_DHCP,  },
    {   IDC_TXT_OPTION42,          10, 147, 75,   10, TAB_SETTINGS_DHCP,  },
    {   IDC_DHCP_OPTION42,         85, 147, 70,   10, TAB_SETTINGS_DHCP,  },
    {   IDC_TXT_OPTION120,         10, 159, 65,   10, TAB_SETTINGS_DHCP,  },
    {   IDC_DHCP_OPTION120,        85, 159, 70,   10, TAB_SETTINGS_DHCP,  },
    {   IDC_TXT_DOMAINNAME,        10, 172, 65,   10, TAB_SETTINGS_DHCP,  },
    {   IDC_DHCP_DOMAINNAME,       85, 172,145,   10, TAB_SETTINGS_DHCP,  },
    {   IDC_TXT_ADDOPTION,         10, 185, 65,   10, TAB_SETTINGS_DHCP,  },
    {   IDC_DHCP_ADDOPTION_NB,     75, 185, 20,   10, TAB_SETTINGS_DHCP,  },
    {   IDC_DHCP_ADDOPTION_VALUE,  95, 185,135,   10, TAB_SETTINGS_DHCP,  },
    // {   IDC_DHCP_OK,              165,  44, 20,   85, TAB_SETTINGS_DHCP,  },
	
	// SYSLOG
	{	IDC_GRP_SYSLOG,             6, 33,228, 70,   TAB_SETTINGS_SYSLOG,  },
	{	IDC_CHECK_PIPE_SYSLOG,     14, 44,200, 19,   TAB_SETTINGS_SYSLOG,  },
	{	IDC_CHECK_SAVE_SYSLOG,     14, 66,100, 10,   TAB_SETTINGS_SYSLOG,  },
	{	IDC_TXT_SYSLOGTOFILE,      34, 80, 25, 10,   TAB_SETTINGS_SYSLOG,  },
	{	IDC_SYSLOG_FILE,           59, 80,170, 12,   TAB_SETTINGS_SYSLOG,  },


	// ALL
 	{   IDOK,                        6,289, 50, 14,   TAB_ALL,            },
 	{   IDC_BUTTON_DEFAULT,         64,289, 50, 14,   TAB_ALL,            },
  	{   IDC_TFTPD_HELP,            122,289, 50, 14,   TAB_ALL,            },
  	{   IDCANCEL,                  180,289, 50, 14,   TAB_ALL,            },
};


static LRESULT Settings_ChangeTabControl (HWND hDlgWnd);


// Retrieve the active tab on the parent window
static char *Settings_GetParentTabText(HWND hDlgWnd)
{
TC_ITEM TabCtrlItem;
static char szTabTxt[64] = "";
HWND   hTabWnd = GetDlgItem(GetParent(hDlgWnd), IDC_TAB_OPTION);
int nTab, Rc;

	TabCtrlItem.mask = TCIF_TEXT;
	TabCtrlItem.cchTextMax = sizeof szTabTxt;
	TabCtrlItem.pszText = szTabTxt;
	nTab = TabCtrl_GetCurSel(hTabWnd);
	Rc = TabCtrl_GetItem (hTabWnd, nTab, &TabCtrlItem);
return szTabTxt;
}  // char *Settings_GetParentTabText(HWND hDlgWnd)



static int Settings_InitTabControl (HWND hDlgWnd)
{
char *szParentTab;
TC_ITEM TabCtrlItem;
HWND   hTabWnd = GetDlgItem (hDlgWnd, IDC_TAB_SETTINGS);
int    nOnglet, Ark;
static const struct S_TabSettingsCtrlData
{
	int   tab;
	char *name;
	char *parentTxt;
}
tTabSettingsCtrlData [] =
{
	ONGLET_SETTINGS_GLOBAL,  "GLOBAL",   "default",
	ONGLET_SETTINGS_TFTP,    "TFTP",     "tftp server",
	ONGLET_SETTINGS_DHCP,    "DHCP",     "dhcp server",
	ONGLET_SETTINGS_SYSLOG,  "SYSLOG",   "syslog server",
};  // tTabCtrlData
static int bInit=FALSE;

	if (!bInit)
	{
	   // convert logical unit into physical units
       for (Ark=0 ; Ark<SizeOfTab(tResize) ; Ark++)
  		   MapDialogRect (hDlgWnd, (LPRECT) & tResize[Ark].x);
	   bInit = TRUE;
	}

    // Create tabs and add them label
    // code is really not optimized, but it is working
    // and since it is called only once...
    TabCtrlItem.mask = TCIF_TEXT | TCIF_PARAM   ;
    
	for (nOnglet = 0; nOnglet < SizeOfTab(tTabSettingsCtrlData) ; nOnglet++)
	{
		    TabCtrlItem.pszText = tTabSettingsCtrlData[nOnglet].name;
		    TabCtrlItem.lParam  = tTabSettingsCtrlData[nOnglet].tab ;
            TabCtrl_InsertItem(hTabWnd, nOnglet, (LPARAM) &  TabCtrlItem);
	} // all tabs

	// Move widget according to the array
    for (Ark=0 ; Ark<SizeOfTab(tResize) ; Ark++)
    {
        MoveWindow (GetDlgItem (hDlgWnd, tResize[Ark].idCtrl),
                    tResize[Ark].x, 
					tResize[Ark].y,
                    tResize[Ark].width,
                    tResize[Ark].height,
                    TRUE);
    }

	// Retrieve Child window tab, tries to open according tab
	szParentTab = Settings_GetParentTabText (hDlgWnd);
	for ( nOnglet=SizeOfTab(tTabSettingsCtrlData) - 1;    nOnglet > 0 ;    nOnglet--  )
		if (lstrcmpi(szParentTab,tTabSettingsCtrlData[nOnglet].parentTxt) == 0)    break;

    // display tab according to the parent window
	TabCtrl_SetCurSel(hTabWnd, nOnglet);
    Settings_ChangeTabControl (hDlgWnd);

return TRUE;
} // Settings_InitTabControl


static int ReadNewSettings (HWND hWnd)
{
char        szBrowsePath [MAX_PATH];
struct S_Tftpd32Settings sNewSettings;
char        sz [32];
//int         Ark;

	// enable all controls
// for (Ark=0 ; Ark<SizeOfTab(tResize) ; Ark++)
  //      ShowWindow (GetDlgItem (hWnd, tResize[Ark].idCtrl), SW_SHOW );


     // DHCP part imported from gui_bootd_settings. Do not check if not enabled
	 if (ISDLG_CHECKED (hWnd, IDC_CHECK_DHCP_SERVER))  
	 {
		 if (Gui_DHCPSaveConfig (hWnd))
			   PostMessage (GetParent (hWnd), WM_SAVE_DHCP_SETTINGS, 0, 0);
		 else 
			 return FALSE;
	 }
		   // Save Settings
     sNewSettings = sGuiSettings;
      // Directory
      GetDlgItemText (hWnd, IDC_BASEDIR, sNewSettings.szBaseDirectory, sizeof sNewSettings.szBaseDirectory);
      // Services
      sNewSettings.uServices = 0;
      if (ISDLG_CHECKED (hWnd, IDC_CHECK_TFTP_SERVER)  )  sNewSettings.uServices |= TFTPD32_TFTP_SERVER;
      if (ISDLG_CHECKED (hWnd, IDC_CHECK_TFTP_CLIENT)  )  sNewSettings.uServices |= TFTPD32_TFTP_CLIENT;
      if (ISDLG_CHECKED (hWnd, IDC_CHECK_SYSLOG_SERVER))  sNewSettings.uServices |= TFTPD32_SYSLOG_SERVER;
      if (ISDLG_CHECKED (hWnd, IDC_CHECK_DHCP_SERVER))    sNewSettings.uServices |= TFTPD32_DHCP_SERVER;
      if (ISDLG_CHECKED (hWnd, IDC_CHECK_DNS_SERVER))     sNewSettings.uServices |= TFTPD32_DNS_SERVER;
      if (ISDLG_CHECKED (hWnd, IDC_CHECK_SNTP_SERVER))    sNewSettings.uServices |= TFTPD32_SNTP_SERVER;

	  sNewSettings.bIPv6 = ISDLG_CHECKED (hWnd, IDC_CHECK_IPv6);
	  if (sNewSettings.bIPv6   &&  LOBYTE(LOWORD(GetVersion())) < 6)
	  {
            MY_WARNING ("IPv6 not correctly supported under\nWindows XP and Windows Server 2003");
			sNewSettings.bIPv6 = FALSE;
	  }

        // test if settings have changed
        // Syslog options
        if (ISDLG_CHECKED (hWnd, IDC_CHECK_SAVE_SYSLOG) )
        {
            GetDlgItemText (hWnd, IDC_SYSLOG_FILE, sNewSettings.szSyslogFile, sizeof sNewSettings.szSyslogFile);
        }
        else
        {
           sNewSettings.szSyslogFile[0] = 0;
        }
        sNewSettings.bSyslogPipe = ISDLG_CHECKED (hWnd, IDC_CHECK_PIPE_SYSLOG);

        // DHCP options 
        if (ISDLG_CHECKED (hWnd, IDC_CHECK_LOCALIP_DHCP))
		{char *p;
             ComboBox_GetText (GetDlgItem(hWnd, IDC_CB_LOCALIP_DHCP), sNewSettings.szDHCPLocalIP , sizeof sNewSettings.szDHCPLocalIP );
			 // Get The first word (do not copy the [Bad Address] flag)
			 sNewSettings.szDHCPLocalIP [sizeof sNewSettings.szDHCPLocalIP - 1]= 0;
			 p = strchr (sNewSettings.szDHCPLocalIP, ' ');
			 if (p!=NULL) *p=0;
		}
        else  sNewSettings.szDHCPLocalIP[0]=0;
        sNewSettings.bPersLeases  = ISDLG_CHECKED (hWnd, IDC_CHECK_PERS_LEASES);
        sNewSettings.bPing        = ISDLG_CHECKED (hWnd, IDC_CHECK_PING);
        sNewSettings.bDoubleAnswer= ISDLG_CHECKED (hWnd, IDC_CHECK_DOUBLE_ANSWER);

        // security Level -> Radio Box
        if (ISDLG_CHECKED (hWnd, IDC_RADIO_SECNONE))      sNewSettings.SecurityLvl = SECURITY_NONE;
        else if (ISDLG_CHECKED (hWnd, IDC_RADIO_SECRO))   sNewSettings.SecurityLvl = SECURITY_READONLY;
        else if (ISDLG_CHECKED (hWnd, IDC_RADIO_SECHIGH)) sNewSettings.SecurityLvl = SECURITY_HIGH;
        else                                             sNewSettings.SecurityLvl = SECURITY_STD;
       // Timeouts -> group timeout
        sNewSettings.Timeout = GetDlgItemInt (hWnd, IDC_TIMEOUT, NULL, FALSE);
        sNewSettings.Retransmit = GetDlgItemInt (hWnd, IDC_MAXRETRANSMIT, NULL, FALSE);
        sNewSettings.Port = GetDlgItemInt (hWnd, IDC_PORT, NULL, FALSE);

        // Advanced Options
        sNewSettings.bHide       = ISDLG_CHECKED (hWnd, IDC_CHECK_HIDE);
        sNewSettings.bProgressBar= ISDLG_CHECKED (hWnd, IDC_CHECK_PROGRESS);
        sNewSettings.WinSize     = GetDlgItemInt (hWnd, IDC_WINSIZE, NULL, FALSE);
        sNewSettings.bNegociate  = ISDLG_CHECKED (hWnd, IDC_CHECK_NEGOCIATE);
        sNewSettings.bDirText    = ISDLG_CHECKED (hWnd, IDC_CHECK_DIRTEXT);
        sNewSettings.bMD5        = ISDLG_CHECKED (hWnd, IDC_CHECK_MD5);
        sNewSettings.bUnixStrings= ISDLG_CHECKED (hWnd, IDC_CHECK_UNIX);
        sNewSettings.bBeep       = ISDLG_CHECKED (hWnd, IDC_CHECK_BEEP);
        sNewSettings.bVirtualRoot= ISDLG_CHECKED (hWnd, IDC_CHECK_VROOT);
        sNewSettings.bPXECompatibility = ISDLG_CHECKED (hWnd, IDC_CHECK_PXE);
        // Sécurité sur l'accès
        if (ISDLG_CHECKED (hWnd, IDC_CHECK_TFTPLOCALIP))
             ComboBox_GetText (GetDlgItem(hWnd, IDC_CB_TFTPLOCALIP), sNewSettings.szTftpLocalIP, sizeof sNewSettings.szTftpLocalIP);
        else sNewSettings.szTftpLocalIP [0] = 0;
        // local ports
        GetDlgItemText (hWnd, IDC_LOCAL_PORTS, sz, sizeof sz);

		// hide controls
  //      Settings_ChangeTabControl (hWnd);
		
		// parse the string, lcc stdio lib does not handle the format syntax %d%*[-:]
        if (   sscanf_s (sz, "%d:%d", & sNewSettings.nTftpLowPort, & sNewSettings.nTftpHighPort) != 2
            && sscanf_s (sz, "%d-%d", & sNewSettings.nTftpLowPort, & sNewSettings.nTftpHighPort) != 2 )
            sNewSettings.nTftpLowPort = sNewSettings.nTftpHighPort = 0;
        // validate data
        if (sNewSettings.Timeout==0  ||  sNewSettings.Retransmit==0  || sNewSettings.Port==0)
        {
            MY_WARNING ("TFTP: Timeouts and Ports should be numerical and can not be 0");
            return FALSE;
        }

        // field Base directory
        if ( IsGuiConnectedToRemoteService () )
              PostMessage (hWnd, WM_TFTP_CHG_WORKING_DIR, 0, 0);
        else
        {
            if (IsValidDirectory (sNewSettings.szBaseDirectory) )
            {HWND hCBWnd = GetDlgItem (GetParent(hWnd), IDC_CB_DIR);
                  GetFullPathName ( sNewSettings.szBaseDirectory, 
                                    sizeof szBrowsePath,
                                    szBrowsePath, 
                                    NULL );
                  TftpDir_AddEntry (hCBWnd, szBrowsePath);
                  lstrcpy (sNewSettings.szWorkingDirectory, sNewSettings.szBaseDirectory); 
                  // transfer to daemon
                  PostMessage (hWnd, WM_TFTP_CHG_WORKING_DIR, 0, 0);
            }
            else   MY_WARNING ("TFTP: Can not change directory");
        }

     // some change need a restart
		if (   sGuiSettings.bIPv6     != sNewSettings.bIPv6
			|| sGuiSettings.Port      != sNewSettings.Port
            || sGuiSettings.uServices != sNewSettings.uServices
            || sGuiSettings.bHide     != sNewSettings.bHide
   // PJO nov 2013     || lstrcmp (sGuiSettings.szDHCPLocalIP, sNewSettings.szDHCPLocalIP) != 0
            || lstrcmp (sGuiSettings.szTftpLocalIP, sNewSettings.szTftpLocalIP) != 0
         )
		{
            MY_WARNING ("You have to restart Tftpd32\nin order to apply the new settings");
		}

        sGuiSettings = sNewSettings;
return TRUE;
} // ReadNewSettings


// Helper functions which takes input from IP combo box and outputs 
// to TFTP/DHCP settings combo box

// to be moved into global header
struct S_If 
{
	char  *descr;
	char  *addr;
}; // Struct S_If




///////////////////////////////////////////////
// Tab control has changed : hide/show controls
static LRESULT Settings_ChangeTabControl (HWND hDlgWnd)
{
HWND   hTabWnd = GetDlgItem (hDlgWnd, IDC_TAB_SETTINGS);
DWORD  nCurOnglet = TabCtrl_GetCurSel (hTabWnd);
DWORD  Ark;
DWORD  TabMask;
TC_ITEM TabCtrlItem;

 // get the selected tab id
 if (nCurOnglet != (DWORD)-1)
   {
          TabCtrlItem.mask = TCIF_PARAM   ;
          TabCtrl_GetItem (hTabWnd, nCurOnglet, & TabCtrlItem);
          TabMask = MakeMask (TabCtrlItem.lParam);
   }
  else    TabMask = TAB_NONE;

 // according to TabMask and tResize table hide/display controls
 for (Ark=0 ; Ark<SizeOfTab(tResize) ; Ark++)
        ShowWindow (GetDlgItem (hDlgWnd, tResize[Ark].idCtrl),
                    (tResize[Ark].mView & TabMask) ? SW_SHOW : SW_HIDE);
/*   if (TabMask == TAB_SETTINGS_DHCP)
   {
       Gui_LoadDHCPConfig (hDlgWnd);
   }
*/
return TabCtrlItem.lParam ;
} // Settings_ChangeTabControl




/** Global handles **********************************************************/

static BOOL FormMain_OnNotify(HWND hwnd, INT wParam, LPNMHDR pnmh)
{
    switch(wParam)
	{
		case IDC_TAB_SETTINGS:
			if (pnmh->code == TCN_SELCHANGE) Settings_ChangeTabControl(hwnd);
			break;
		//
		// TODO: Add other control id case statements here. . .
		//
	}
    FORWARD_WM_NOTIFY (hwnd, wParam, pnmh, DefWindowProc );
	return FALSE;
}

void FormMain_OnClose(HWND hwnd)
{
	EndDialog(hwnd, 0);
}



BOOL FormMain_OnInitDialog(HWND hWnd, HWND hwndFocus, LPARAM lParam)
{
	// Set the window name to either tftpd32 or tftpd64
	SetWindowText (hWnd, TFTPD_SETTINGS_TITLE); 

    Settings_InitTabControl (hWnd);

     // Deactivate Browse Button
     if ( IsGuiConnectedToRemoteService () )
            Button_Enable (GetDlgItem (hWnd, IDC_BUTTON_BROWSE), FALSE);
     // Directory
     SetDlgItemText (hWnd, IDC_BASEDIR, sGuiSettings.szBaseDirectory);
     // Services
     CHECK_DLG_IF (hWnd, IDC_CHECK_TFTP_SERVER,  sGuiSettings.uServices  & TFTPD32_TFTP_SERVER);
     CHECK_DLG_IF (hWnd, IDC_CHECK_TFTP_CLIENT,  sGuiSettings.uServices  & TFTPD32_TFTP_CLIENT);
     CHECK_DLG_IF (hWnd, IDC_CHECK_SYSLOG_SERVER,sGuiSettings.uServices  & TFTPD32_SYSLOG_SERVER);
     CHECK_DLG_IF (hWnd, IDC_CHECK_DHCP_SERVER,  sGuiSettings.uServices  & TFTPD32_DHCP_SERVER);
     CHECK_DLG_IF (hWnd, IDC_CHECK_DNS_SERVER,   sGuiSettings.uServices  & TFTPD32_DNS_SERVER);
     CHECK_DLG_IF (hWnd, IDC_CHECK_SNTP_SERVER,  sGuiSettings.uServices  & TFTPD32_SNTP_SERVER);
	 // IPv6
	 CHECK_DLG_IF (hWnd, IDC_CHECK_IPv6,         sGuiSettings.bIPv6); 
     // Syslog Save option
     CHECK_DLG_IF (hWnd, IDC_CHECK_SAVE_SYSLOG, sGuiSettings.szSyslogFile[0]!=0);
     SetDlgItemText (hWnd, IDC_SYSLOG_FILE, sGuiSettings.szSyslogFile);
     Edit_Enable (GetDlgItem (hWnd, IDC_SYSLOG_FILE), sGuiSettings.szSyslogFile[0]!=0);
     CHECK_DLG_IF (hWnd, IDC_CHECK_PIPE_SYSLOG, sGuiSettings.bSyslogPipe);

	 // DHCP Persistant leases + Address + ping
     CHECK_DLG_IF (hWnd, IDC_CHECK_PERS_LEASES, sGuiSettings.bPersLeases);
     CHECK_DLG_IF (hWnd, IDC_CHECK_PING, sGuiSettings.bPing);
     CHECK_DLG_IF (hWnd, IDC_CHECK_DOUBLE_ANSWER, sGuiSettings.bDoubleAnswer);
//	       FillCBLocalIP (GetDlgItem (hWnd, IDC_CB_LOCALIP_DHCP), FALSE, sGuiSettings.szDHCPLocalIP);


	 // show IPv4 address for DHCP
	 CopyCBContent ( GetDlgItem (GetParent (hWnd), IDC_CB_IP),
					   GetDlgItem (hWnd, IDC_CB_LOCALIP_DHCP),
					   sGuiSettings.szDHCPLocalIP,
					   AF_INET );
     CHECK_DLG_IF (hWnd, IDC_CHECK_LOCALIP_DHCP, sGuiSettings.szDHCPLocalIP[0]!=0);
     ComboBox_Enable (GetDlgItem (hWnd, IDC_CB_LOCALIP_DHCP), ISDLG_CHECKED (hWnd, IDC_CHECK_LOCALIP_DHCP) );

     // security Level -> Radio Box
     CHECK_DLG_IF (hWnd, IDC_RADIO_SECNONE, sGuiSettings.SecurityLvl==SECURITY_NONE);
     CHECK_DLG_IF (hWnd, IDC_RADIO_SECSTD,  sGuiSettings.SecurityLvl==SECURITY_STD);
     CHECK_DLG_IF (hWnd, IDC_RADIO_SECHIGH, sGuiSettings.SecurityLvl==SECURITY_HIGH);
     CHECK_DLG_IF (hWnd, IDC_RADIO_SECRO,   sGuiSettings.SecurityLvl==SECURITY_READONLY);
     // Timeouts -> group timeout
     SetDlgItemInt (hWnd, IDC_TIMEOUT, sGuiSettings.Timeout, FALSE);
     SetDlgItemInt (hWnd, IDC_MAXRETRANSMIT, sGuiSettings.Retransmit, FALSE);
     SetDlgItemInt (hWnd, IDC_PORT, sGuiSettings.Port, FALSE);

     // Advanced Options
     CHECK_DLG_IF (hWnd, IDC_CHECK_HIDE,     sGuiSettings.bHide);
     CHECK_DLG_IF (hWnd, IDC_CHECK_PROGRESS, sGuiSettings.bProgressBar);
     CHECK_DLG_IF (hWnd, IDC_CHECK_NEGOCIATE,sGuiSettings.bNegociate);
     CHECK_DLG_IF (hWnd, IDC_CHECK_PXE,      sGuiSettings.bPXECompatibility);
     CHECK_DLG_IF (hWnd, IDC_CHECK_DIRTEXT,  sGuiSettings.bDirText);
     CHECK_DLG_IF (hWnd, IDC_CHECK_MD5,      sGuiSettings.bMD5);
     CHECK_DLG_IF (hWnd, IDC_CHECK_UNIX,     sGuiSettings.bUnixStrings);
     CHECK_DLG_IF (hWnd, IDC_CHECK_BEEP,     sGuiSettings.bBeep);
     CHECK_DLG_IF (hWnd, IDC_CHECK_VROOT,    sGuiSettings.bVirtualRoot);
     SetDlgItemInt (hWnd, IDC_WINSIZE, sGuiSettings.WinSize, FALSE);

     // Limitations des acces
//           FillCBLocalIP (GetDlgItem (hWnd, IDC_CB_LOCALIP), FALSE, sGuiSettings.szLocalIP);
	 CopyCBContent ( GetDlgItem (GetParent (hWnd), IDC_CB_IP),
					   GetDlgItem (hWnd, IDC_CB_TFTPLOCALIP),
					   sGuiSettings.szTftpLocalIP,
					   AF_UNSPEC);
     CHECK_DLG_IF (hWnd, IDC_CHECK_TFTPLOCALIP, sGuiSettings.szTftpLocalIP[0]!=0);
     ComboBox_Enable (GetDlgItem (hWnd, IDC_CB_TFTPLOCALIP), ISDLG_CHECKED (hWnd, IDC_CHECK_TFTPLOCALIP) );

     // Fenêtre d'anticipation
     CHECK_DLG_IF (hWnd, IDC_CHECK_WINSIZE, sGuiSettings.WinSize>0);
     Edit_Enable (GetDlgItem (hWnd, IDC_WINSIZE), sGuiSettings.WinSize > 0);
     // local ports
     if (sGuiSettings.nTftpLowPort!=0 && sGuiSettings.nTftpHighPort>=sGuiSettings.nTftpLowPort)
     {char sz[64];
          wsprintf (sz, "%d:%d", sGuiSettings.nTftpLowPort, sGuiSettings.nTftpHighPort);
          SetDlgItemText (hWnd, IDC_LOCAL_PORTS, sz);
     }

	// DHCP 
	 Gui_LoadDHCPConfig (hWnd);

     CenterChildWindow (hWnd, CCW_VISIBLE);
	return 0;
} // FormMain_OnInitDialog


void FormMain_OnCommand(HWND hWnd, int wParam, HWND hwndCtl, UINT codeNotify)
{
char        szSyslogFileName [MAX_PATH];
char        szBrowsePath [MAX_PATH];

   switch (wParam)
   {
        case IDC_CHECK_PXE :
        // uncheck other choice
        UNCHECK_DLG (hWnd, IDC_CHECK_NEGOCIATE);
           if ( ISDLG_CHECKED (hWnd, wParam) )
        {
            CHECK_DLG_IF (hWnd, IDC_CHECK_VROOT, TRUE);
            CHECK_DLG_IF (hWnd, IDC_CHECK_UNIX, TRUE);
        }
        break;

        case IDC_CHECK_NEGOCIATE :
     // uncheck other choice
        UNCHECK_DLG (hWnd, IDC_CHECK_PXE);
        break;


        case IDC_CHECK_TFTP_SERVER :
        case IDC_CHECK_DNS_SERVER :
        case IDC_CHECK_TFTP_CLIENT :
        case IDC_CHECK_SYSLOG_SERVER :
           if (!   (   ISDLG_CHECKED (hWnd, IDC_CHECK_TFTP_SERVER)
                     || ISDLG_CHECKED (hWnd, IDC_CHECK_TFTP_CLIENT)
                     || ISDLG_CHECKED (hWnd, IDC_CHECK_SYSLOG_SERVER)
                     || ISDLG_CHECKED (hWnd, IDC_CHECK_DHCP_SERVER) 
                     || ISDLG_CHECKED (hWnd, IDC_CHECK_DNS_SERVER) 
                     || ISDLG_CHECKED (hWnd, IDC_CHECK_SNTP_SERVER) 
                    )
            )
           {
             // re-check dlg item
               CHECK_DLG_IF (hWnd, wParam, TRUE);;
            MY_WARNING ("At least one service have to be checked");
         }
          break;
        case IDC_CHECK_TFTPLOCALIP :
            ComboBox_Enable ( GetDlgItem (hWnd, IDC_CB_TFTPLOCALIP), ISDLG_CHECKED (hWnd, wParam) );
            break;

        case IDC_CHECK_LOCALIP_DHCP :
            ComboBox_Enable ( GetDlgItem (hWnd, IDC_CB_LOCALIP_DHCP), ISDLG_CHECKED (hWnd, wParam) );
            break;


       case IDC_CHECK_SAVE_SYSLOG :
           Edit_Enable ( GetDlgItem (hWnd, IDC_SYSLOG_FILE), ISDLG_CHECKED (hWnd, wParam) );
          if (ISDLG_CHECKED (hWnd, wParam))
          {
             GetDlgItemText (hWnd, IDC_SYSLOG_FILE, szSyslogFileName, sizeof szSyslogFileName);
             if (szSyslogFileName[0] == 0)
                   SetDlgItemText (hWnd, IDC_SYSLOG_FILE, DEFAULT_SYSLOG_FILE);
           }
          break;

        case IDC_CHECK_WINSIZE :
           if ( ISDLG_CHECKED (hWnd, wParam) )
            {
               MY_WARNING ("Use an anticipation window acclerates transfers\nOn the other hand, it may not work with your TFTP client.");
               Edit_Enable ( GetDlgItem (hWnd, IDC_WINSIZE), TRUE );
            }
          else
           {
               // sNewSettings.WinSize = 0;
               SetDlgItemInt (hWnd, IDC_WINSIZE, 0, FALSE);
               Edit_Enable ( GetDlgItem (hWnd, IDC_WINSIZE), FALSE );
            }
            break;

        case IDC_BUTTON_BROWSE :
          GetDlgItemText (hWnd, IDC_BASEDIR, szBrowsePath, sizeof szBrowsePath);
             // Do not change dir, wait for OK button
             if (MyBrowseWindow (hWnd, szBrowsePath, szBrowsePath[0]!='.'))
                    SetDlgItemText (hWnd, IDC_BASEDIR, szBrowsePath);
             break;

        case IDC_BUTTON_DEFAULT :
             if (CMsgBox (hWnd, RESET_DEFAULT_TEXT, "Tftpd32", MB_YESNOCANCEL | MB_ICONQUESTION) == IDYES)
             {
                 PostMessage (GetParent (hWnd), WM_DESTROY_SETTINGS, 0, 0);  // destroy all Keys
                 MY_WARNING ("Settings entries have been deleted.\nYou have to restart Tftpd32 in order to use the new specifications");
                 EndDialog (hWnd, 0);         // and exit window -> do not save in reg
             }
             break;

        case IDOK :
			if ( ReadNewSettings (hWnd) )
			{
				PostMessage (GetParent (hWnd), WM_SAVE_SETTINGS, 0, 0);
				EndDialog (hWnd, 0);
			}
            break;

        case IDCANCEL :
            EndDialog (hWnd, -1);
            break;

        case IDC_TFTPD_HELP:
            // WinHelp(hWnd, szTftpd32Help, HELP_CONTENTS, 0);
            ShellExecute (hWnd, "open", szTftpd32Help,  NULL, NULL, SW_NORMAL);
            break;

			default: break;
	} // switch wPama

} // handleWM_COMMAND




/////////////////////////////////////////////////////////
//  Settings Window management
/////////////////////////////////////////////////////////

LRESULT CALLBACK SettingsProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

  switch (message)
  {
		HANDLE_MSG (hWnd, WM_CLOSE, FormMain_OnClose);
		HANDLE_MSG (hWnd, WM_COMMAND, FormMain_OnCommand);
		HANDLE_MSG (hWnd, WM_INITDIALOG, FormMain_OnInitDialog);
		HANDLE_MSG (hWnd, WM_NOTIFY, FormMain_OnNotify);
		//// TODO: Add dialog message crackers here...

	default: return FALSE;
  } // switch

return FALSE;
} // SettingsProc



/////////////////////////////////////////////////////////
// Save window position
/////////////////////////////////////////////////////////
BOOL Tftpd32SaveWindowPos (HWND hMainWnd)
{
RECT  R;
HKEY  hKey=INVALID_HANDLE_VALUE;
DWORD dwState;
INT   Rc;
char  sz[128];

    GetWindowRect (hMainWnd, &R);
    wsprintf (sz, "%d %d %d %d ", R.left, R.top, R.right, R.bottom);

    Rc = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                         TFTPD32_MAIN_KEY,
                         0,
                         NULL,
                         REG_OPTION_NON_VOLATILE,
                         KEY_WRITE,
                         NULL,
                        &hKey,
                        &dwState) == ERROR_SUCCESS;
    Rc &=      SAVEKEY (KEY_WINDOW_POS,sz,REG_SZ);
    if (hKey!=INVALID_HANDLE_VALUE)  RegCloseKey (hKey);
return Rc;
} // Tftpd32SaveWindowPos



BOOL Tftpd32RetrieveWindowPos (HWND hMainWnd)
{
RECT  R;
HKEY  hKey=INVALID_HANDLE_VALUE;
DWORD dwSize;
INT   Rc, Ark=0;
char  sz[128], *pCur, *pNext;


   Rc = RegOpenKeyEx (HKEY_LOCAL_MACHINE,    // Key handle at root level.
                      TFTPD32_MAIN_KEY,      // Path name of child key.
                      0,                      // Reserved.
                      KEY_READ,                // Requesting read access.
                    & hKey);                 // Address of key to be returned.
    // Lire les données dans cette entrée
    READKEY (KEY_WINDOW_POS, sz);
    if (hKey!=INVALID_HANDLE_VALUE)
            RegCloseKey (hKey);

  // parse entry
 for (sz [sizeof sz - 1] = 0, pNext=pCur=sz ; *pNext!=0 ; pNext++)
      if (*pNext==' ')
       {
          *pNext=0;
          switch (Ark++)
         {
             case 0 : R.left   = atoi(pCur); break;
             case 1 : R.top    = atoi(pCur); break;
             case 2 : R.right  = atoi(pCur); break;
             case 3 : R.bottom = atoi(pCur); break;
         }
          pCur=pNext+1;
      }

 // The window should be visible (inside the screen)
    if (Ark == 4
        && R.left + 150 < GetSystemMetrics(SM_CXSCREEN)
        && R.top  + 150 < GetSystemMetrics(SM_CYSCREEN)
        && R.left < R.right
        && R.top  < R.bottom
        && R.left >= 0
        && R.top  >= 0
     )
    {
      MoveWindow (hMainWnd, R.left, R.top, R.right-R.left, R.bottom-R.top, TRUE);
    }
return Rc;
} // Tftpd32RetrieveWindowPos






