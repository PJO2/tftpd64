//////////////////////////////////////////////////////
//
// Projet Tftpd32.   Mars 2000 Ph.jounin
// File tftp_srv.c:  Display management
//
//
//////////////////////////////////////////////////////



#include "headers.h"

#define TOOLBAR_HEIGHT  0        // pas de fenêtre Toolbar


// The tabs
enum { ONGLET_TFTP_SERVER = 0,
       ONGLET_TFTP_CLIENT,
       ONGLET_DHCP_SERVER,
       ONGLET_SYSLOG_SERVER,
       ONGLET_SNTP_SERVER,
       ONGLET_DNS_SERVER,
       ONGLET_EVENTS_VIEWER,
       NB_ONGLETS };
// tab masks : 1>>Onglet
#define TAB_TFTP_SERVER     MakeMask(ONGLET_TFTP_SERVER)
#define TAB_TFTP_CLIENT     MakeMask(ONGLET_TFTP_CLIENT)
#define TAB_DHCP_SERVER     MakeMask(ONGLET_DHCP_SERVER)
#define TAB_SYSLOG_SERVER   MakeMask(ONGLET_SYSLOG_SERVER)
#define TAB_SNTP_SERVER     MakeMask(ONGLET_SNTP_SERVER)
#define TAB_DNS_SERVER      MakeMask(ONGLET_DNS_SERVER)
#define TAB_EVENTS_VIEWER   MakeMask(ONGLET_EVENTS_VIEWER)
#define TAB_NONE            MakeMask(NB_ONGLETS)
#define TAB_ALL            (DWORD) -1


// The sizing table
static struct S_Resize
{
    DWORD  idCtrl;                      // The control ID
    LONG   x, y, width, height;         // size of the window at "normal" size
    LONG   dx, dy, dwidth, dheight;     // proportional size increase (1 means 10%)
    DWORD  mView;                       // windows is displayed for Tabs ?
}
tResize [] =
{
// COMMON
    { IDC_TXT_BAD_SERVICES, 10,  60, 170,  30,    0,  0,  0,  0, TAB_NONE }, // visible only if services is 0
    { IDC_TAB_OPTION,        3,  36, 215, 102,    0,  0, 10, 10, TAB_ALL },
    { IDC_ABOUT_BUTTON,      6, 141,  62,  12,    0, 10,  1,  0, TAB_ALL },
#ifndef TFTP_CLIENT_ONLY
	{ IDC_SETTINGS_BUTTON,  80, 141,  62,  12,    4, 10,  2,  0, TAB_ALL },
#else
	{ IDC_SETTINGS_BUTTON,  80, 141,  62,  12,    4, 10,  2,  0, 0 },
#endif
    { IDC_TFTPD_HELP,      154, 141,  62,  12,    9, 10,  1,  0, TAB_ALL },
    { IDC_BASE_DIRECTORY,   63,   7, 110,  12,    0,  0,  7,  0, 0 },
    { IDC_CB_IP,            68,  22, 105,  65,    0,  0,  7,  0, TAB_ALL },
    { IDC_CB_DIR,           68,   7, 105,  41,    0,  0,  7,  0, TAB_ALL },
    { IDC_SHDIR_BUTTON,    180,  22,  37,  12,    8,  0,  2,  0, TAB_ALL },
    { IDC_BROWSE_BUTTON,   180,   7,  37,  12,    8,  0,  2,  0, TAB_ALL },
    { IDC_TXT_ADDRESS,       6,  23,  60,   9,    0,  0,  0,  0, TAB_ALL },
    { IDC_TXT_BASEDIR,       6,   8,  60,   9,    0,  0,  0,  0, TAB_ALL },
// TFTP SERVER
    { IDC_LV_TFTP,           6,  49, 210,  80,    0,  0, 10, 10, TAB_TFTP_SERVER },
// EVENTS VIEWER
    { IDC_LB_LOG,                 6,  49, 210,  68,    0,  0, 10, 10, TAB_EVENTS_VIEWER },
//    { IDC_CURRENT_ACTION,       135, 123,  76,  12,    2, 10,  8,  0, TAB_EVENTS_VIEWER },
    { IDC_TXT_CURACTION,         85, 125,  70,   9,    2, 10,  8,  0, TAB_EVENTS_VIEWER },
    { IDC_TFTP_CLEAR,             6, 123,  32,  12,    0, 10,  0,  0, TAB_EVENTS_VIEWER },
    { IDC_TFTP_COPYTOCLIP,       40, 123,  32,  12,    0, 10,  0,  0, TAB_EVENTS_VIEWER },
// TFTP CLIENT
    { IDC_TXT_CLIENTHOST,        11,  55,  22,  10,    1,  1,  0,  0, TAB_TFTP_CLIENT },
    { IDC_TXT_CLIENTPORT,       137,  55,  16,  10,    7,  1,  0,  0, TAB_TFTP_CLIENT },
    { IDC_TXT_CLIENT_LOCALFILE,  11,  69,  44,  10,    1,  2,  0,  0, TAB_TFTP_CLIENT },
    { IDC_TXT_CLIENT_REMOTEFILE, 11,  83,  44,  10,    1,  3,  0,  0, TAB_TFTP_CLIENT },
    { IDC_CLIENT_HOST,           37,  53,  87,  12,    1,  1,  4,  0, TAB_TFTP_CLIENT },
    { IDC_CLIENT_PORT,          158,  53,  27,  12,    7,  1,  2,  0, TAB_TFTP_CLIENT },
    { IDC_CLIENT_LOCALFILE,      59,  66, 104,  12,    2,  2,  5,  0, TAB_TFTP_CLIENT },
    { IDC_CLIENT_REMOTEFILE,     59,  80, 104,  12,    2,  3,  5,  0, TAB_TFTP_CLIENT },
	// display #block during transfer
    { IDC_CLIENT_BLOCK,           6, 115,  55,  18,    1,  6,  0,  0, TAB_TFTP_CLIENT },
    { IDC_TXT_CLIENTBLOCKSIZE,   11,  94,  44,  10,    1,  4,  0,  0, TAB_TFTP_CLIENT },  
    { IDC_CB_DATASEG,            59,  94,  46,  12,    2,  4,  3,  0, TAB_TFTP_CLIENT },  

    { IDC_CLIENT_BROWSE,        170,  66,  17,  12,    8,  2,  1,  0, TAB_TFTP_CLIENT },  
    //   { IDC_CLIENT_FULL_PATH,      95,  94,  94,  12,    0,  0,  0,  0, 0 },  // suppressed
    { IDC_CLIENT_GET_BUTTON,     60, 110,  25,  12,    3,  6,  1,  0, TAB_TFTP_CLIENT },
    { IDC_CLIENT_SEND_BUTTON,    90, 110,  25,  12,    4,  6,  1,  0, TAB_TFTP_CLIENT },
    { IDC_CLIENT_BREAK_BUTTON,  147, 110,  25,  12,    6,  6,  1,  0, TAB_TFTP_CLIENT },
    { IDC_CLIENT_PROGRESS,       11, 124, 178,  12,    1,  7,  8,  0, TAB_TFTP_CLIENT },
    { IDC_TEXT_TFTPCLIENT_HELP,  11, 135, 0,  0,    0,  0,  0,  0, 0 },  // suppressed

// DHCP SERVER
	{ IDC_LV_DHCP,                6,  49, 210,  68,    0,  0, 10, 10, TAB_DHCP_SERVER },

// SYSLOG SERVER
    { IDC_LB_SYSLOG,             6, 49, 210,  68,    0,  0, 10, 10, TAB_SYSLOG_SERVER },
    { IDC_SYSLOG_CLEAR,         16, 122, 70,  12,    0, 10,  0,  0, TAB_SYSLOG_SERVER },
    { IDC_SYSLOG_COPY,          90, 122, 70,  12,    0, 10,  0,  0, TAB_SYSLOG_SERVER },
//    { IDC_SYSLOG_SAVE,       126, 112, 80,  12,   10, 10,  0,  0, TAB_SYSLOG_SERVER },

// SNTP SERVER
    { IDC_TXT_SNTP,             20,  60, 135, 60,    0,   0, 0,  0, TAB_SNTP_SERVER },

// DNS SERVER
    { IDC_TXT_DNS,              20,  60, 135, 60,    0,   0, 0,  0, TAB_DNS_SERVER },
	{ IDC_LB_DNS,                6, 49, 210,  68,    0,  0, 10, 10, TAB_DNS_SERVER },

}; // tResize

static RECT SizeInit =    {40, 30, 40+230, 30+180 };
static RECT RectMinMax  = { 0, 0, 200, 192 };
static RECT RectTftpClient =  { 10, 50, 154, 53 };

///////////////////////////////////////////////
// Resize until ..
int TR_MinMaxInfo (HWND hDlgWnd, LPMINMAXINFO lpInfo)
{
    lpInfo->ptMinTrackSize.x = RectMinMax.right;
    lpInfo->ptMinTrackSize.y = RectMinMax.bottom;
return FALSE;
}   // TR_MinMaxInfo



///////////////////////////////////////////////
// Resize the main window --> resize each control
int TR_ResizeWindow (HWND hDlgWnd, BOOL bInit)
{
static BOOL bConvert2Physique=FALSE;
DWORD Ark;
RECT  RClient;
SIZE  SClient;
int  InflateX, InflateY;       // tenth

 // at first call : convert logical data to physical
  if (! bConvert2Physique)
  {
      bConvert2Physique = TRUE;
      for (Ark=0 ; Ark<SizeOfTab(tResize) ; Ark++)
           MapDialogRect (hDlgWnd, (LPRECT) & tResize[Ark].x);
      MapDialogRect (hDlgWnd, & SizeInit);
      MapDialogRect (hDlgWnd, & RectMinMax);
      MapDialogRect (hDlgWnd, & RectTftpClient);
   } 

  if (bInit)
        MoveWindow (hDlgWnd, SizeInit.left, SizeInit.top, SizeInit.right, SizeInit.bottom, FALSE);

    // get window size and calculate the inflate rate
    GetWindowRect (hDlgWnd, & RClient);
    SClient.cx = RClient.right - RClient.left;
    SClient.cy = RClient.bottom - RClient.top;
    InflateX = 10 * ( SClient.cx - (SizeInit.right - SizeInit.left) );
    InflateY = 10 * ( SClient.cy - (SizeInit.bottom - SizeInit.top) );

    for (Ark=0 ; Ark<SizeOfTab(tResize) ; Ark++)
    {
        MoveWindow (GetDlgItem (hDlgWnd, tResize[Ark].idCtrl),
                    tResize[Ark].x + (tResize[Ark].dx * InflateX) / 100,
                    tResize[Ark].y + (tResize[Ark].dy * InflateY) / 100,
                    tResize[Ark].width + (tResize[Ark].dwidth * InflateX) / 100,
                    tResize[Ark].height+ (tResize[Ark].dheight * InflateY) / 100,
                    TRUE);
    }

    InvalidateRect (hDlgWnd, NULL, FALSE);

return Ark;
} //TR_ResizeWindow




static int CR_Redisplay;
///////////////////////////////////////////////
// Note du changement dans la selection Tree
void CR_TreeSelectionHasChanged(void)
{
   CR_Redisplay = TRUE;
}

///////////////////////////////////////////////
// Add or delete item in the Tab control

// ordened list of the tab to be displayed
static  struct S_TabCtrlData
{
	int   tab;
	char *name;
	int   service;
	int   status;
} tTabCtrlData [] =
{
	ONGLET_TFTP_SERVER,   "Tftp Server" ,  TFTPD32_TFTP_SERVER,	   SERVICE_STOPPED,
	ONGLET_TFTP_CLIENT,   "Tftp Client",   TFTPD32_TFTP_CLIENT,    SERVICE_STOPPED,
	ONGLET_DHCP_SERVER,   "DHCP server",   TFTPD32_DHCP_SERVER,    SERVICE_STOPPED,
	ONGLET_SYSLOG_SERVER, "Syslog server", TFTPD32_SYSLOG_SERVER,  SERVICE_STOPPED,
	ONGLET_SNTP_SERVER,   "SNTP server",   TFTPD32_SNTP_SERVER,    SERVICE_STOPPED,
	ONGLET_DNS_SERVER,    "DNS server",    TFTPD32_DNS_SERVER,     SERVICE_STOPPED,
	ONGLET_EVENTS_VIEWER, "Log viewer",    TFTPD32_NONE,           SERVICE_STOPPED,
};  // tTabCtrlData

// An add-on to the tab control : the Z order of the dialog item has to be changed
// for the TFTP client window in order to make the drag and drop work
static int TR_ChgZOrder (HWND hDlgWnd, HWND   hTabWnd)
{
int Ark;
// Ordonated list of the Client Controls 
static const int tTftpClientCtrl [] = 
{ 
    IDC_CLIENT_HOST, IDC_CLIENT_PORT, IDC_CLIENT_LOCALFILE, IDC_CLIENT_REMOTEFILE,
    IDC_CLIENT_GET_BUTTON, IDC_CLIENT_SEND_BUTTON
};  
	SetWindowPos (hTabWnd, HWND_BOTTOM, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);
    for ( Ark=SizeOfTab (tTftpClientCtrl) ; Ark>0 ; Ark-- )
	    SetWindowPos ( GetDlgItem (hDlgWnd, tTftpClientCtrl [Ark-1]), 
		               HWND_TOP, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE );
return Ark;
} // TR_ChgZOrder




int TR_ChgTabControl (HWND hDlgWnd, int nService, int nStatus)
{
TC_ITEM TabCtrlItem;
HWND   hTabWnd = GetDlgItem (hDlgWnd, IDC_TAB_OPTION);
// Ark will point on the tTabCtrlData tab entry, nOnglet the position of the tab
int    nOnglet, Ark;

	// Find position of the tab item 
	for ( Ark = 0, nOnglet = 0;  
		  Ark < SizeOfTab (tTabCtrlData) &&  tTabCtrlData[Ark].service!=nService ; 
		  Ark++ )
		  if (tTabCtrlData[Ark].status == SERVICE_RUNNING) 
			  nOnglet++;
	if (tTabCtrlData[Ark].service!=nService)
	{
		LogToMonitor ("can not find service %d", nService);
		return -1;
	}

	// Service to be added ?
	if (nStatus == SERVICE_RUNNING &&  tTabCtrlData[Ark].status == SERVICE_STOPPED)
	{
		// Create tabs and add them label
		// code is really not optimized, but it is working
		// and since it is called only once...
		TabCtrlItem.mask = TCIF_TEXT | TCIF_PARAM   ;
		TabCtrlItem.pszText = tTabCtrlData[Ark].name;
		TabCtrlItem.lParam  = tTabCtrlData[Ark].tab ;
        TabCtrl_InsertItem(hTabWnd, nOnglet, (LPARAM) &  TabCtrlItem);
		tTabCtrlData[Ark].status = SERVICE_RUNNING;
	} // all tabs
	// Service to be suppressed
	if (nStatus == SERVICE_STOPPED &&  tTabCtrlData[Ark].status == SERVICE_RUNNING)
	{
		tTabCtrlData[Ark].status = SERVICE_STOPPED;
        TabCtrl_DeleteItem(hTabWnd, nOnglet);	
	}

    // First available tab is selected
    TabCtrl_SetCurSel (hTabWnd, 0);
    TR_ChangeTabControl (hDlgWnd);

	// Place Tab ctrl at bottom of Z-order and all client control at top
	// Local File control will then accept drag an drop
    // (set also the other windows just to ease navigation with TAB)
	if (nService==TFTPD32_TFTP_CLIENT  &&  nStatus==SERVICE_RUNNING)
			TR_ChgZOrder (hDlgWnd, hTabWnd);
return TRUE;
} // InitTabControl


///////////////////////////////////////////////
// Tab control has changed : hide/show controls
LRESULT TR_ChangeTabControl (HWND hDlgWnd)
{
HWND   hTabWnd = GetDlgItem (hDlgWnd, IDC_TAB_OPTION);
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

  // DHCP tab has been selected : display the settings
  if (TabMask == TAB_DHCP_SERVER)
  {
	 Gui_LoadDHCPConfig (hDlgWnd);
  }

  // refresh IP address combo
  RedrawWindow (GetDlgItem (hDlgWnd, IDC_CB_IP), NULL, NULL, RDW_ERASENOW | RDW_INVALIDATE);

return TabCtrlItem.lParam ;
} // ChangeTabControl


// Open all tabs refering to a started service
// used to answer to message C_REPLY_GET_SERVICES
int TR_OpenAllTabs (HWND hDlgWnd, int nService)
{
int Ark;
	for ( Ark=0 ;  Ark < SizeOfTab (tTabCtrlData) ; Ark++ )
	{
	   TR_ChgTabControl (hDlgWnd, nService & tTabCtrlData[Ark].service, SERVICE_RUNNING);
	}
return 0;
}


// return the service matching the current tab
DWORD TR_GetCurrentTab (HWND hDlgWnd)
{
HWND   hTabWnd = GetDlgItem (hDlgWnd, IDC_TAB_OPTION);
DWORD  nCurOnglet = TabCtrl_GetCurSel (hTabWnd);
DWORD  Ark;
TC_ITEM TabCtrlItem;

 TabCtrlItem.lParam= -1;
 // get the selected tab id
 if (nCurOnglet != (DWORD)-1)
   {
          TabCtrlItem.mask = TCIF_PARAM   ;
          TabCtrl_GetItem (hTabWnd, nCurOnglet, & TabCtrlItem);
   }
   // retrieve index of array tTabCtrlData
   for (Ark=0 ; Ark<SizeOfTab (tTabCtrlData) && TabCtrlItem.lParam!=tTabCtrlData[Ark].tab; Ark++);
return Ark>=SizeOfTab (tTabCtrlData) ? 0 : tTabCtrlData[Ark].service;
} // TR_GetCurrentTab 

