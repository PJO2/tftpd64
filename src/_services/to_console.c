//////////////////////////////////////////////////////
//
// Projet TFTPD32.  January 2006 Ph.jounin
// Projet DHCPD32.  January 2006 Ph.jounin
// File threading.h:    Manage communication from service to GUI
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////


#include "headers.h"
#include <process.h>
#include <stdio.h>

#include "threading.h"
#include "bootpd_functions.h"
#include "tftpd_functions.h"

// -----------------------------------------------------------------------------------------------------
// IPC mechanisms
// The console is the thread which exports the data from the daemon part to the GUI
// This thread waits for the 3 events 
//      A msg is received from the GUI        --> read it and dispatch its processing
//      One log (or more) is ready to be sent --> pop them from linked list and send them
//      One msg is about to be sent           --> blocks caller thread and send it
// 
// -----------------------------------------------------------------------------------------------------

// First item -> structure belongs to the module and is shared by all threads
extern struct LL_TftpInfo *pTftpFirst;

// Event used to block the caller thread until the msg is sent
static HANDLE hEvMsgSent       = INVALID_HANDLE_VALUE;
// Semaphore to protect the structure S_CallerThreadData
static HANDLE hMsgRequestSemaf = INVALID_HANDLE_VALUE;



// -----------------------------------------------------------------------------------------------------
// API for worker threads
// Add a message into the queue then wake up console thread
// -----------------------------------------------------------------------------------------------------
// get and set console status
enum eGuiStatus  { NOT_CONNECTED=17, CONNECTED };
static int gGuiStatus = NOT_CONNECTED;
static void SetGuiStatus (int status) 
	{ LogToMonitor ("Console %sconnected", status==CONNECTED ? "" : "dis"); 
	  gGuiStatus = status; }
static int  GetGuiStatus (void)       { return gGuiStatus;   }


// init : just create the msg file and pass some arguments
//        for the LOG function
void StartAsyncMessagingFacility (void)
{
    LL_Create   (LL_ID_MSG_TO_GUI, MAX_MSG_IN_QUEUE);
} // StartAsyncMessagingFacility 

void StopAsyncMessagingFacility (void)
{
void *p;
    // empties list and close it
    while ( (p=LL_PopMsg (LL_ID_MSG_TO_GUI)) != NULL ) free (p);
    LL_Destroy (LL_ID_MSG_TO_GUI);
} // StopAsyncMessagingFacility 

int SendMsgRequest (int type, const void *msg_stuff, int size, BOOL bBlocking, BOOL bRetain)
{
int Rc;
static int InternalSemaf = FALSE;

	// 3.34 : check if GUI is listening to notification
	// added for service edition
	// 4.05 (modify start order --> wait for console
#ifdef SERVICE_EDITION
	if (GetGuiStatus () == NOT_CONNECTED) return 0;
#else
	while (GetGuiStatus () == NOT_CONNECTED) { Sleep (100); } 
#endif

    Rc = WaitForSingleObject( hMsgRequestSemaf, INFINITE );
	assert (Rc==WAIT_OBJECT_0);

    if (bBlocking &&  hMsgRequestSemaf!=INVALID_HANDLE_VALUE)
	{
        // Request the lock to ensure all other threads are waiting
		if (Rc==WAIT_OBJECT_0)
		{
			WaitForMsgQueueToFinish (LL_ID_MSG_TO_GUI);
			// push message 
			LL_PushTypedMsg (LL_ID_MSG_TO_GUI, msg_stuff, size, type | 0x10000);
			// wake up console thread and wait until it has got the msg
			WakeUpThread (TH_CONSOLE);
			Rc = WaitForSingleObject ( hEvMsgSent, INFINITE );
			assert (Rc==WAIT_OBJECT_0);

			WaitForMsgQueueToFinish (LL_ID_MSG_TO_GUI);
			// it is done
		}
	}
	else
	{
		// do not send message while blocking message is in progress
		while (InternalSemaf)
			Sleep (1);
		// if (bRetain)
		{
			// simply push the msg into the queue and signal event to the console
			LL_PushTypedMsg (LL_ID_MSG_TO_GUI, msg_stuff, size, type);
			WakeUpThread (TH_CONSOLE);
		}
	}
	
	Rc = ReleaseMutex (hMsgRequestSemaf);
	assert (Rc!=0);

return 1;    
} // SendMsgRequest


// -----------------------------------------------------------------------------------------------------
// The "server" function, started by console thread :
// sends collected msgs to the GUI
// -----------------------------------------------------------------------------------------------------
int ProcessAsyncMessages (SOCKET sDlg, HANDLE hEv)
{
void *p;
int   Ark=0;
int   Rc;
int   nLen;
int   type;
int   msg_id;
int   bwatchdog=TRUE; // check that queue contains only one blocking message

    do
    {
        p = LL_PopTypedMsg (LL_ID_MSG_TO_GUI, & nLen, & msg_id, & type);
        if (p != NULL)
        {
           Rc = SendMsg (sDlg, type & 0xFFFF, p, nLen);
           if (Rc==nLen) Ark++;
           free (p);
		   // 32th byte is set to require a event setting
		   // (thread is blocked until msg send)
		   if (type & 0x10000) 
		   { assert (bwatchdog);
		     Rc = SetEvent (hEv); 
			 assert (Rc!=0);
			 //bwatchdog = FALSE;
		   }
        }
    }
    while (p!=NULL);
return Ark;            
} // ProcessAsyncMessages




// -----------------------------------------------------------------------------------------------------
// Console
// -----------------------------------------------------------------------------------------------------


static int ProcessMsg (SOCKET s, const struct S_ConsoleMsg *pmsg)
{
struct LL_TftpInfo *pTftp;
int                 uServices;

LogToMonitor ("TFTPd console receive msg %d\n", pmsg->type);
    switch (pmsg->type)
    {
        case C_CONS_KILL_TRF :
            LOG (1, "transfer %d must be killed", pmsg->u.kill.dwTransferId);
            for ( pTftp=pTftpFirst ; 
                  pTftp!=NULL && pTftp->tm.dwTransferId != pmsg->u.kill.dwTransferId ;
                  pTftp = pTftp->next );
			if (pTftp != NULL) { nak (pTftp, ECANCELLED); pTftp->st.ret_code=TFTP_TRF_STOPPED; }
            break;
            
        case C_TFTP_TERMINATE :
LogToMonitor ("terminating TFTP service\n");
            tThreads[TH_TFTP].gRunning = FALSE;
            WakeUpThread (TH_TFTP);
            break;
        
        case C_DHCP_TERMINATE :
LogToMonitor ("terminating DHCP service\n");
            tThreads[TH_DHCP].gRunning = FALSE;
            // wake up DHCP thread
	        WakeUpThread (TH_DHCP);
            break;
            
        case C_TERMINATE :
LogToMonitor ("stopping services\n");
            TerminateWorkerThreads (FALSE); // keep management threads
            break;  

        case C_SUSPEND :
LogToMonitor ("suspending services\n");
            TerminateWorkerThreads (TRUE); // keep management threads
            break;            
            
        case C_START :
LogToMonitor ("starting services\n");
			StartMultiWorkerThreads (TRUE);
            break;            
            
        case C_DHCP_RRQ_SETTINGS :
LogToMonitor ("sending DHCP settings\n");
            SendMsg (s, C_DHCP_RPLY_SETTINGS, & sParamDHCP, sizeof sParamDHCP);
            break;
            
        case C_TFTP_RRQ_SETTINGS :
LogToMonitor ("sending TFTP settings\n");
            SendMsg (s, C_TFTP_RPLY_SETTINGS, & sSettings, sizeof sSettings);
            break;
            
        case C_DHCP_WRQ_SETTINGS :
LogToMonitor ("storing new DHCP settings\n");
            DHCPSaveConfig ( & pmsg->u.dhcp_settings );
            break;
            
        case C_TFTP_WRQ_SETTINGS :
LogToMonitor ("storing new TFTP settings\n");
			{static struct S_RestartTable sRestart;
			    sRestart.newservices = pmsg->u.tftp_settings.uServices;
				sRestart.oldservices = sSettings.uServices;
				sRestart.flapservices = 0;
				if (   sSettings.Port != pmsg->u.tftp_settings.Port
		            || lstrcmp (sSettings.szTftpLocalIP, pmsg->u.tftp_settings.szTftpLocalIP )!=0 )
					sRestart.flapservices |= TFTPD32_TFTP_SERVER;
				// restart syslog if its settings log has changed
				if (     sSettings.uServices &  TFTPD32_SYSLOG_SERVER 
					  && (   sSettings.bSyslogPipe != pmsg->u.tftp_settings.bSyslogPipe
					      || strcmp(sSettings.szSyslogFile,pmsg->u.tftp_settings.szSyslogFile)!= 0 )
				   )
					 sRestart.flapservices |= TFTPD32_SYSLOG_SERVER;

            sSettings = pmsg->u.tftp_settings;

            if ( IsValidDirectory ( pmsg->u.tftp_settings.szBaseDirectory ) )
                    lstrcpyn ( sSettings.szWorkingDirectory, 
                               pmsg->u.tftp_settings.szBaseDirectory, 
                               sizeof sSettings.szWorkingDirectory );
			_beginthread ( Tftpd32UpdateServices, 0, (void *) & sRestart );            
			Tftpd32SaveSettings ();
			}
			break;

        case C_TFTP_RESTORE_DEFAULT_SETTINGS :
LogToMonitor ("restore default settings\n");
            Tftpd32DestroySettings ();
            break;
            
        case C_TFTP_CHG_WORKING_DIR :
LogToMonitor ("changing working directory to <%s>\n", pmsg->u.working_dir);
            if ( IsValidDirectory ( pmsg->u.working_dir ) )
                    lstrcpyn ( sSettings.szWorkingDirectory, 
                               pmsg->u.working_dir, 
                               sizeof sSettings.szWorkingDirectory );
            break;
        case C_RRQ_WORKING_DIR :
LogToMonitor ("sending working directory <%s>\n", sSettings.szWorkingDirectory);
            SendMsg (s, C_REPLY_WORKING_DIR, 
                     sSettings.szWorkingDirectory, 
                     1 + lstrlen (sSettings.szWorkingDirectory) );
            break;
        
        case C_DELETE_ASSIGNATION :
LogToMonitor ("deleting DHCP entry %X\n", pmsg->u.del_lease.ip);
            { struct in_addr  addr;
              BOOL   dummy;
                addr.s_addr = pmsg->u.del_lease.ip;
                DHCPDestroyItem ( DHCPSearchByIP ( & addr, &dummy  ) );
            }
            break;

		case C_RRQ_GET_SERVICES :
LogToMonitor ("sending running services\n");
			uServices = GetRunningThreads ();
            SendMsg (s, 
				     C_REPLY_GET_SERVICES, 
					 & uServices,
					 sizeof uServices );
            break;

		case C_RRQ_GET_INTERFACES :
LogToMonitor ("sending IP interfaces");
			AnswerIPList ();
			break;

        case C_RRQ_DIRECTORY_CONTENT :
LogToMonitor ("sending Directory content");
            SendDirectoryContent ();
            break;

        case C_TFTP_GET_FULL_STAT :
LogToMonitor ("sending Directory content");
            ConsoleTftpGetStatistics ();
            break;

		default :
LogToMonitor ("Service received unknown message %d\n", pmsg->type);
            break;

    }   
return 1;    
} // ReadMsg



// wait until GUI is connected
SOCKET WaitForGuiConnection (void) 
{
static SOCKET     sListen = INVALID_SOCKET;
SOCKET            sDlg    = INVALID_SOCKET;
SOCKADDR_STORAGE  saSockAddr; /* specifications pour le Accept */
int               nAddrLen = sizeof saSockAddr;
int               Rc;

	do
	{
		sSettings.uConsolePort = TFTPD32_TCP_PORT;
		// sListen is static --> don't reopen 
		if (sListen == INVALID_SOCKET)
		   sListen = TcpGetListenSocket (AF_INET, "tftpd32", & sSettings.uConsolePort);
#ifdef STANDALONE_EDITION
		    // second chance (standalone edition):let the system choose its socket
		    // pass the port nb through the sGuiSettings structure
		    Rc = GetLastError();
		    if (sListen==INVALID_SOCKET && GetLastError()==WSAEADDRINUSE)
		    {
			    sGuiSettings.uConsolePort = 0;
			    sListen = TcpGetListenSocket (AF_INET, NULL, & sGuiSettings.uConsolePort);
		    }
#endif

		if (sListen==INVALID_SOCKET)
		{
			SVC_ERROR ("can not create listening socket\nError %d", GetLastError ());
		}
		else
		{HANDLE tEvents[2];
		 int nTriggeredEvent;
				// Create Socket Event to process either wake up or accept 
				tEvents [0] = WSACreateEvent();
				WSAEventSelect (sListen, tEvents[0], FD_ACCEPT); 
				// waits either internal sollicitation or msg recpetion
				tEvents[1] = tThreads[TH_CONSOLE].hEv;
				nTriggeredEvent = WaitForMultipleObjects (2, tEvents, FALSE, INFINITE);

				if (nTriggeredEvent==1) { WSACloseEvent (tEvents [0]); closesocket (sListen); continue; }

			// an accept is ready --> Establish session
			sDlg = accept (sListen, (struct sockaddr *) &saSockAddr, &nAddrLen);
			if (sDlg == INVALID_SOCKET) 
			{ 
					SVC_ERROR ("accept error %d", GetLastError());
			}
			// free listening socket
			WSACloseEvent (tEvents [0]);

			// sListen socket no more necessary
		    closesocket (sListen);  
			sListen = INVALID_SOCKET;

		} // sListen OK
		if (sDlg==INVALID_SOCKET && tThreads[TH_CONSOLE].gRunning) Sleep (1000);
	}
	while (sDlg==INVALID_SOCKET  &&  tThreads[TH_CONSOLE].gRunning);

    // detect if the client hangs and does not quit properly
    if (sDlg != INVALID_SOCKET)
    {int True=TRUE;
       Rc = setsockopt (sDlg, SOL_SOCKET, SO_KEEPALIVE, (char *) & True, sizeof True);
       if (Rc) LogToMonitor ("Error %d during setsockopt\n", GetLastError ());
    } // sDlg OK

return sDlg;
} // WaitForGuiConnection




///////////////////////////////
// thread main loop
///////////////////////////////

void TftpdConsole (void *param)
{
SOCKET sDlg;
int    Rc;
int nTriggeredEvent;
static struct S_ConsoleMsg msg;
enum e_Events { EV_TOGUI, EV_FROMGUI, EV_NUMBER  };
HANDLE tEvents [EV_NUMBER];

    // open logging facility
    StartAsyncMessagingFacility (  );
    
    do
    {
		LogToMonitor ("opening comm socket\n");
		SetGuiStatus (NOT_CONNECTED);
		sDlg = WaitForGuiConnection (); // fail only if thread ends
		if (sDlg==INVALID_SOCKET)  continue;
        // Verify Versions 
				LogToMonitor ("Verify Console/GUI parameters\n");

        Rc = TcpExchangeChallenge (sDlg, 0x56AE, CURRENT_PROTOCOL_VERSION,  NULL, sSettings.szConsolePwd);
        if ( Rc < 0 ) 
        {
            Sleep (1000);
            continue;
        }
	LogToMonitor ( "Version check OK\n" );
	SetGuiStatus (CONNECTED);


	// Startup blocking service by creating
    // one event and one semaphore to code the blocking sendmsgrequest    
    hMsgRequestSemaf  = CreateMutex(NULL, 0, NULL);
	hEvMsgSent        = CreateEvent(NULL,FALSE,FALSE,NULL);
    if (hMsgRequestSemaf==NULL)
        CMsgBox (NULL, "Can not create resource", APPLICATION, MB_OK | MB_ICONERROR);

	tThreads[TH_CONSOLE].bInit = TRUE;		// inits OK

         // Create Socket Event 
        tEvents [EV_FROMGUI] = WSACreateEvent();
        WSAEventSelect (sDlg, tEvents[EV_FROMGUI], FD_READ | FD_CLOSE); 

        do
        {
            // waits either internal sollicitation or msg reception
            tEvents[EV_TOGUI] = tThreads[TH_CONSOLE].hEv;
            nTriggeredEvent = WaitForMultipleObjects (EV_NUMBER, tEvents, FALSE, INFINITE);
            switch (nTriggeredEvent)
            {
                case EV_FROMGUI :
                    // WSA events should be manually reset
                    WSAEventSelect (sDlg, 0, 0); 
                    ResetEvent (tEvents[EV_FROMGUI]);
                    Rc = TcpPPRecv (sDlg, (void *) &msg, sizeof msg, 10, INVALID_HANDLE_VALUE);
                    if (Rc>0) LOG (9, "received %d bytes from console", Rc);
                    if (Rc>0)     ProcessMsg  (sDlg, &msg);  
                    else          LogToMonitor ("rcvd error %d/%d in console\n", Rc, GetLastError ());
                    WSAEventSelect (sDlg, tEvents[EV_FROMGUI], FD_READ | FD_CLOSE); 
                    break;
                    
                // message to be sent to Gui (except logs)
                case EV_TOGUI : 
                    ProcessAsyncMessages ( sDlg, hEvMsgSent );
                    break;
            
            } // switch signalled events
        }
        while (tThreads[TH_CONSOLE].gRunning  &&  (Rc>0 || Rc==TCP4U_TIMEOUT) );

        WSACloseEvent (tEvents [EV_FROMGUI]);   tEvents [EV_FROMGUI] = INVALID_HANDLE_VALUE;
        closesocket (sDlg);

		// blocking service not available
		CloseHandle (hEvMsgSent);		 hEvMsgSent = INVALID_HANDLE_VALUE;
		CloseHandle (hMsgRequestSemaf);  hMsgRequestSemaf = INVALID_HANDLE_VALUE;

		LogToMonitor ("end of GUI session\n");

    }
    while ( tThreads[TH_CONSOLE].gRunning );

	// terminate therad : don't listen anymore
	// closesocket (sListen);

	// unblock some threads
	SetEvent (hEvMsgSent); 	Sleep (10);  SetEvent (hEvMsgSent);
    // stop logging service
	StopAsyncMessagingFacility ();

LogToMonitor ("End of console thread\n");
_endthread ();    
} // TftpdConsole 

