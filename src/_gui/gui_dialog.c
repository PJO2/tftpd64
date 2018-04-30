//////////////////////////////////////////////////////
//
// Projet TFTPD32.   Mar 07 By  Ph.jounin
// File gui_dialog.c : decodes messages sent by service
//
// released under artistic license
//


#include "headers.h"


static struct S_TftpGui *pTftpGuiFirst=NULL;
const struct S_TftpGui *Gui_GetFirstGuiItem (void) { return pTftpGuiFirst; }



#ifdef OLD_CODE
// A debugging tool
// for debugging only
int CS_LogToDebugFile (const char *sz);


int CS_LogToDebugFile (const char *sz)
{
HANDLE hLog;
int dummy;
static bInit=FALSE;
static CRITICAL_SECTION CriticalSection;

		if (!bInit) 
		{int Rc;
			Rc = InitializeCriticalSectionAndSpinCount( & CriticalSection, 0x00000400);
			assert (Rc);
			bInit = TRUE;
		}


		EnterCriticalSection (& CriticalSection);

	    hLog = CreateFile ( "C:\\temp\\tftpd32_debug.txt", 
							GENERIC_WRITE, 
							FILE_SHARE_READ,	// permit read operations
							NULL, 
							OPEN_ALWAYS,		// open or create
							FILE_ATTRIBUTE_NORMAL,
							NULL );
		if (hLog==INVALID_HANDLE_VALUE) 
			  OutputDebugString ("unable to open debug log file");
		// stop if file has not been opened
		assert (hLog!=INVALID_HANDLE_VALUE);


		SetFilePointer (hLog, 0, NULL, FILE_END);
		WriteFile (hLog, sz, strlen(sz), & dummy, NULL);
		CloseHandle (hLog);


	   LeaveCriticalSection(&CriticalSection);
} // CS_LogToDebugFile


int CS_ParseLL (const char *header, int nId)
{ char sz[10240];
struct S_TftpGui *pTftpGui;
	// Logs LL states :
    int n;
    n = wsprintf (sz, "\n%s for transfert Id %d\n",header, nId );
    for ( pTftpGui=pTftpGuiFirst ;  pTftpGui != NULL  ; pTftpGui = pTftpGui->next )
		n += wsprintf (sz+n, "pointer %p, trf %d\n", pTftpGui, pTftpGui->dwTransferId );
    CS_LogToDebugFile (sz);

	return n;
}
#endif



// return TRUE if GUI is connected to a remote Host
int IsGuiConnectedToRemoteService (void)
{
return GetEnvironmentVariable (TFTP_HOST, NULL, 0);
} // IsGuiConnectedToRemoteService



////////////////////////////////////////////
// Messages sent TO the services
////////////////////////////////////////////

int Gui_RequestFullReport (SOCKET sService)
{
int Rc;
    LogToMonitor ("Requesting report from service");
    Rc = SendMsg (sService, C_TFTP_GET_FULL_STAT, "" , 1);
return Rc;
} // Gui_RequestFullReport

int Gui_AbortTftpTransfer (SOCKET sService, DWORD dwTransferId)
{
int Rc;
    LogToMonitor ("Transfer %d cancel by user", dwTransferId);
    LogToMonitor ("GUI aborting TFTP transfer %d\n", dwTransferId);
    Rc = SendMsg (sService, C_CONS_KILL_TRF, & dwTransferId, sizeof dwTransferId);
return Rc;
} // AbortTransfer

int Gui_SuppressDHCPAllocation (SOCKET sService, unsigned ip)
{
int Rc;
    LogToMonitor ("Deleting DHCP entry %X\n", ip );
    Rc = SendMsg (sService, C_DELETE_ASSIGNATION, & ip , sizeof ip);
return Rc;
} // Gui_SuppressDHCPAllocation

int Gui_DestroySettings (SOCKET sService)
{
int Rc;
    LogToMonitor ("Deleting settings entry\n");
    Rc = SendMsg (sService, C_TFTP_RESTORE_DEFAULT_SETTINGS, "" , 1);
return Rc;
} // Gui_DestroySettings


int Gui_StopTftpService (SOCKET sService)
{
int Rc;
    LogToMonitor ("GUI Stopping TFTP service\n");
    Rc = SendMsg (sService, C_TFTP_TERMINATE, "", 1);
return Rc;
} // Gui_StopTftpService 

int Gui_StopDhcpService (SOCKET sService)
{
int Rc;
    LogToMonitor ("GUI Stopping DHCP service\n");
    Rc = SendMsg (sService, C_DHCP_TERMINATE, "", 1);
return Rc;
} // Gui_StopDhcpService

int Gui_StopAllServices (SOCKET sService)
{
int Rc;
    LogToMonitor ("GUI Stopping all services\n");
    Rc = SendMsg (sService, C_TERMINATE, "", 1);
return Rc;
} // Gui_StopAllServices

int Gui_SuspendServices (SOCKET sService)
{
int Rc;
    LogToMonitor ("GUI Supsending services\n");
    Rc = SendMsg (sService, C_SUSPEND, "", 1);
return Rc;
} // Gui_SuspendServices

int Gui_StartAllServices (SOCKET sService)
{
int Rc;
    LogToMonitor ("GUI Starting all services\n");
    Rc = SendMsg (sService, C_START, "", 1);
return Rc;
} // Gui_StartAllServices

int Gui_AskDHCPSettings (SOCKET sService)
{
int Rc;
    LogToMonitor ("GUI Ask DHCP settings\n");
    Rc = SendMsg (sService, C_DHCP_RRQ_SETTINGS, "", 1);
return Rc;    
} // Gui_AskDHCPSettings

int Gui_AskTFTPSettings (SOCKET sService)
{
int Rc;
    LogToMonitor ("GUI Ask TFTP settings\n");
    Rc = SendMsg (sService, C_TFTP_RRQ_SETTINGS, "", 1);
return Rc;    
} // Gui_AskTFTPSettings

int Gui_ChangeWorkingDirectory (SOCKET sService, const char *szNewDir)
{
int Rc;
    LogToMonitor ("GUI Set working directory to %s\n", szNewDir);
    Rc = SendMsg (sService, C_TFTP_CHG_WORKING_DIR, szNewDir, lstrlen (szNewDir)+1);
return Rc;    
} // Gui_ChangeWorkingDirectory

int Gui_SaveSettings (SOCKET sService, struct S_Tftpd32Settings *pset)
{
int Rc;
    LogToMonitor ("Saving global Settings\n");
    Rc = SendMsg (sService, C_TFTP_WRQ_SETTINGS, pset, sizeof *pset);
return Rc;        
} // Gui_SaveSettings 

int Gui_SaveDhcpSettings (SOCKET sService, struct S_DHCP_Param *pset)
{
int Rc;
    LogToMonitor ("Saving DHCP Settings\n");
    Rc = SendMsg (sService, C_DHCP_WRQ_SETTINGS, pset, sizeof *pset);
return Rc;        
} // Gui_SaveSettings 

int Gui_RequestWorkingDir (SOCKET sService)
{
int Rc;
    LogToMonitor ("Requesting Working Directory\n");
    Rc = SendMsg (sService, C_RRQ_WORKING_DIR, "", 1);
return Rc;        
} // Gui_RequestWorkingDir 

int Gui_RequestRunningServices (SOCKET sService)
{
int Rc;
    LogToMonitor ("Requesting Running Services\n");
    Rc = SendMsg (sService, C_RRQ_GET_SERVICES, "", 1);
return Rc;        
} // Gui_RequestRunningServices

int Gui_RequestIPInterfaces (SOCKET sService)
{
int Rc;
    LogToMonitor ("Requesting list of IP interfaces\n");
    Rc = SendMsg (sService, C_RRQ_GET_INTERFACES, "", 1);
return Rc;        
} // Gui_RequestIPInterfaces

int Gui_RequestListDirectory (SOCKET sService)
{
int Rc;
    LogToMonitor ("Requesting Directory Content\n");
    Rc = SendMsg (sService, C_RRQ_DIRECTORY_CONTENT, "", 1);
return Rc;        
} // Gui_RequestWorkingDir 

////////////////////////////////////////////
// Messages receivd FROM the services
////////////////////////////////////////////

#ifdef OLD_CODE
CRITICAL_SECTION CriticalSection; 
static int bDone=0;

   if (!bDone)     // Initialize the critical section one time only.
			bDone = InitializeCriticalSectionAndSpinCount(&CriticalSection, 0x00000400);
	EnterCriticalSection(&CriticalSection);
#endif



// Create a pTftpGui structure and fill it with the msg
static int GuiTFTPNew (HWND hMainWnd, const struct S_TftpTrfNew *pTrf)
{
struct S_TftpGui *pTftpGui;


    pTftpGui = calloc (1, sizeof *pTftpGui);
    pTftpGui->dwTransferId = pTrf->dwTransferId;
    if (sGuiSettings.bProgressBar)      
            pTftpGui->hGaugeWnd = Gui_CreateGaugeWindow (hMainWnd, pTftpGui);
    pTftpGui->stat = pTrf->stat;
#ifdef MSVC    
    pTftpGui->filename = _strdup (pTrf->szFile);
#else    
    pTftpGui->filename = strdup (pTrf->szFile);
#endif    
    pTftpGui->opcode = pTrf->opcode;
    pTftpGui->stg_addr = pTrf->from_addr;

    pTftpGui->next = pTftpGuiFirst;    
    // places the leaf at the head of the structure
    pTftpGuiFirst = pTftpGui;

   Gui_TftpReporting (GetDlgItem (hMainWnd, IDC_LV_TFTP), pTftpGuiFirst);


return 0;    
} // GuiNewTrf

// terminates a transfer 
static int GuiTFTPEnd (HWND hMainWnd, struct S_TftpTrfEnd *pTrf)
{
struct S_TftpGui *pTftpGui, *pTftpPrev;


	// search mathing internal structure and get previous member
    for ( pTftpPrev=NULL, pTftpGui=pTftpGuiFirst ;
          pTftpGui != NULL && pTftpGui->dwTransferId != pTrf->dwTransferId ;
          pTftpGui = pTftpGui->next )
                pTftpPrev = pTftpGui;
	// in the standalone edition, the structure has been created by this
	// very process. 
#ifdef STANDALONE_EDITION
    if (pTftpGui == NULL) 
	{
		OutputDebugString ("transfert not found");
		return 0;
	}
#elif defined (SERVICE_EDITION)
	// in the service, the GUI may have missed the begining of the transfer
	if (pTftpGui==NULL)  return 0;
#endif

	
	pTftpGui->stat = pTrf->stat;
    Gui_TftpReporting (GetDlgItem (hMainWnd, IDC_LV_TFTP), pTftpGuiFirst);

	// detach leaf 
    if (pTftpPrev != NULL)  pTftpPrev->next = pTftpGui->next ;
    else                    pTftpGuiFirst   = pTftpGui->next ;

	// now we can play with the leaf : it belongs no more to the linked list
	// update stat 
	// pTftpGui->stat = pTrf->stat;
    // Gui_TftpReporting (GetDlgItem (hMainWnd, IDC_LV_TFTP), pTftpGuiFirst);
    
    // close gauge window
    if (pTftpGui->hGaugeWnd != NULL)
        PostMessage (pTftpGui->hGaugeWnd, WM_CLOSE, 0, 0);
    // free allocation
    free (pTftpGui->filename);
    free (pTftpGui);
	LogToMonitor ("GUI: transfer destroyed\n");

	// recall TftpReporting : it will notice the process
    Gui_TftpReporting (GetDlgItem (hMainWnd, IDC_LV_TFTP), pTftpGuiFirst);

	
return 0;    
 } // GuiTFTPEnd


static int GuiTFTPStat (struct subStats *pTrf, time_t dNow)
{
struct S_TftpGui *pTftpGui;
    // search mathing internal structure
    for ( pTftpGui=pTftpGuiFirst ; 
          pTftpGui!=NULL && pTftpGui->dwTransferId != pTrf->dwTransferId ;
          pTftpGui = pTftpGui->next );
    if (pTftpGui == NULL) return -1;
    assert ( pTftpGui != NULL ) ;
    pTftpGui->stat = pTrf->stat;
    if (pTftpGui->hGaugeWnd != NULL)       Gui_UpdateGaugeWindow (pTftpGui, dNow);
    time (& pTftpGui->stat.dLastUpdate) ;

return 0;    
} // GuiTFTPStat

// Verify working directory has been changed
static int Gui_VerifyWorkingDirectory (HWND hWnd, const char *szDir)
{
   TftpDir_AddEntry (GetDlgItem (hWnd, IDC_CB_DIR), szDir);
   if (lstrcmp (sGuiSettings.szWorkingDirectory, szDir)!=0)
       CMsgBox (hWnd, "Directory has not been changed", APPLICATION, MB_OK | MB_ICONHAND);
return 0;
}  //      Gui_VerifyWorkingDirectory

// ---------------------------------------------------
// The socket reception
// ---------------------------------------------------
static struct S_ConsoleMsg sMsg;

int Gui_GetMessage (HWND hWnd, SOCKET sService, int bBlocking, int nMsgType)
{
struct S_ConsoleMsg *pmsg = & sMsg;
int Rc;
int Ark;
static int bFirstTime=TRUE;

 do
 {
   Rc = TcpPPRecv (sService, (char *) & sMsg, sizeof sMsg, bBlocking ? 10 : TCP4U_DONTWAIT, 0);
   if (Rc >= sizeof pmsg->type)
   {
       // buf is big enough to map on a S_ConsoleMsg struct
       pmsg = & sMsg;
       switch (pmsg->type)
       {
           case C_LOG :
                LB_LOG ( GetDlgItem (hWnd, IDC_LB_LOG), pmsg->u.log );
                break;
		   case C_ERROR :
			   CMsgBox (hWnd, pmsg->u.error, APPLICATION, MB_OK | MB_ICONERROR);
			   break;
		   case C_WARNING :
			   CMsgBox (hWnd, pmsg->u.warning, APPLICATION, MB_OK | MB_ICONHAND);
			   break;

           case C_TFTP_TRF_NEW :
LogToMonitor ("GUI: beginning of trf %d\n", pmsg->u.trf_end.dwTransferId);
                GuiTFTPNew (hWnd, & pmsg->u.trf_new);
                break;
           case C_TFTP_TRF_END :
LogToMonitor ("GUI: end of trf %d\n", pmsg->u.trf_end.dwTransferId);
                GuiTFTPEnd (hWnd, & pmsg->u.trf_end);
                break;
           case C_TFTP_TRF_STAT :
				if (pmsg->u.trf_stat.nbTrf)
LogToMonitor ("GUI: receiving %d stats\n", pmsg->u.trf_stat.nbTrf);
                for (Ark=0 ; Ark<pmsg->u.trf_stat.nbTrf ; Ark++)
                    GuiTFTPStat (& pmsg->u.trf_stat.t[Ark], pmsg->u.trf_stat.dNow);
                Gui_TftpReporting (GetDlgItem (hWnd, IDC_LV_TFTP), pTftpGuiFirst);
                break;
            case C_DHCP_LEASE :
LogToMonitor ("GUI: receiving new DHCP lease\n", pmsg->u.trf_stat.nbTrf);
                DhcpRefresh_ListView ( pmsg->u.dhcp_lease.nb, pmsg->u.dhcp_lease.l );
                break;
                
            case C_TFTP_RPLY_SETTINGS :
LogToMonitor ("GUI: receive TFTP settings\n");
                sGuiSettings = pmsg->u.tftp_settings;
                // TftpDir_AddEntry (GetDlgItem (hWnd, IDC_CB_DIR), sGuiSettings.szWorkingDirectory);
#ifdef PREVIOUS
				if (sGuiSettings.uServices & TFTPD32_TFTP_CLIENT)
				{
					 TR_ChgTabControl (hWnd, TFTPD32_TFTP_CLIENT, SERVICE_RUNNING);
				}
#endif
  			   TR_ChgTabControl (hWnd, TFTPD32_TFTP_CLIENT, 
								 sGuiSettings.uServices & TFTPD32_TFTP_CLIENT ? SERVICE_RUNNING : SERVICE_STOPPED);
                break;

            case C_DHCP_RPLY_SETTINGS :
LogToMonitor ("GUI: receive DHCP settings\n");
				// just record the settings
				// since the DHCP window may not be created
				// we don't display them
                sGuiParamDHCP = pmsg->u.dhcp_settings;
                break;

            case C_REPLY_WORKING_DIR :
LogToMonitor ("GUI: receive working directory\n");
                if (! bFirstTime) Gui_VerifyWorkingDirectory (hWnd, pmsg->u.working_dir);
                bFirstTime = FALSE;
                lstrcpy (sGuiSettings.szWorkingDirectory, pmsg->u.working_dir);
                TftpDir_AddEntry (GetDlgItem (hWnd, IDC_CB_DIR), sGuiSettings.szWorkingDirectory);
                break;

			case C_REPLY_GET_SERVICES :
LogToMonitor ("GUI: Receive running services (%04X)", pmsg->u.uServices);
				// overwrite GUI settings
				sGuiSettings.uRunningServices = pmsg->u.uServices;
#ifdef SERVICE_EDITION
				if (hWnd!=NULL) TR_OpenAllTabs (hWnd, pmsg->u.uServices);
#endif
				break;

            case C_SYSLOG :
LogToMonitor ("GUI: receive syslog\n");
                AddSyslogItem ( GetDlgItem (hWnd, IDC_LB_SYSLOG), 
                                pmsg->u.syslog_msg.from, 
                                pmsg->u.syslog_msg.txt );
                break;
            
			case C_REPLY_GET_INTERFACES :
LogToMonitor ("GUI: receive IP address notification\n");
				ChangeIPAddress (hWnd, 
					             & pmsg->u.address );
			    break;

			case C_SERVICES_STARTED :
LogToMonitor ("GUI: receive end of init notifications\n");
				// nothing to do, but will unblock the GUI thread
				break;

            case C_REPLY_DIRECTORY_CONTENT :
LogToMonitor ("GUI: receive remote directory\n");
                OpenNewDialogBox (hWnd,
                                  IDD_DIALOG_SHDIR,
                                  (DLGPROC) ShDirProc, 
                                  (LPARAM) & pmsg->u.dir,
                                  NULL);
                break;

			case C_DNS_NEW_ENTRY :
LogToMonitor ("GUI: receive new DNS entry\n");
				// A DNS request
                AddDNSItem ( GetDlgItem (hWnd, IDC_LB_DNS), 
					  	     pmsg->u.dns.name,
						     pmsg->u.dns.ipv4,
							 pmsg->u.dns.ipv6 );
				break;

				case C_CHG_SERVICE :
LogToMonitor ("GUI: new service %d status %d\n", pmsg->u.chg.service, pmsg->u.chg.status);
				// display or delete the tab
				if (hWnd!=NULL)  TR_ChgTabControl (hWnd, pmsg->u.chg.service, pmsg->u.chg.status);
				break;

            default :
LogToMonitor ("GUI received unknown message %d, length %d\n", pmsg->type, Rc);
                break;
            
        }
    } // if Rc>0
  } // do until error, or msgtype ok (if blocking)
  while ( Rc>0 &&  bBlocking && (nMsgType!=pmsg->type && nMsgType>0) );
return Rc;        
} // GuiGetMessage

