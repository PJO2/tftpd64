//////////////////////////////////////////////////////
//
// Projet TFTPD32.   Feb 99 By  Ph.jounin
// File start_threads.c:  Thread management
//
// Started by the main thread
// The procedure is itself a thread
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////

// Quick documentation for the 
// The first thread started fork the thread StartTftpd32Services then call OpenDialogBox
// and act as the GUI
// StartTftpd32Services start one thread by service (DHCP, DNS, TFTP server, Syslog, ...)
// and 3 utility threads : 
//		- The console which controls the messages between GUI and threads
//		- The registry threads which is in charge to read and write the settings
//		- The Scheduler which logs the status of the worker threads
// Tftpd32UpdateServices may be launched by the console thread in order to kill or start 
// some threads.



#include "headers.h"
#include <process.h>

#include "threading.h"



#define BOOTPD_PORT   67
#define BOOTPC_PORT   68
#define TFTP_PORT     69
#define SNTP_PORT    123
#define DNS_PORT      53
#define SYSLOG_PORT  514

const int BootPdPort = BOOTPD_PORT;
const int SntpdPort  = SNTP_PORT;
const int DnsPort    = DNS_PORT;
const int SyslogPort = SYSLOG_PORT;

static const struct S_MultiThreadingConfig
{
    char       *name;                       // name of the service
    int         serv_mask;                  // identify service into sSettings.uServices
    void     ( *thread_proc )( void * );    // the service main thread
    BOOL        manual_event;               // automatic/manual reset of its event
    int         stack_size;                 // 
    int         family;                     // socket family
    int         type;                       // socket type
    char       *service;                    // the port to be bound ascii
    const int  *def_port;                   // the port to be bound numerical
	int	    	rfc_port;					// default port taken from RFC
    char       *sz_interface;               // the interface to be opened
    int         wake_up_by_ev;              // would a SetEvent wake up the thread, FALSE if thread blocked on recvfrom
	int         restart;                    // should the thread by restarted  
	BOOL        gui;						// thread is linked to a tab in the GUI
}
tThreadsConfig [] =
{
	// Order is the same than enum in threading.h
	// TFTPD32_SCHEDULER is the last pseudo service
    "Console",       TFTPD32_CONSOLE,  TftpdConsole,          FALSE, 16384,           0,          0,     NULL,            NULL,  0,          NULL,                     TRUE,  FALSE, FALSE,
    "Registry",     TFTPD32_REGISTRY,  AsyncSaveKeyBckgProc,  FALSE,  4096,           0,          0,     NULL,            NULL,  0,          NULL,                     TRUE,  FALSE, FALSE,
	"Scheduler",   TFTPD32_SCHEDULER,  Scheduler,             FALSE,  4096,           0,          0,     NULL,            NULL,  0,          NULL,                     TRUE,  FALSE, FALSE,
    "DHCP",      TFTPD32_DHCP_SERVER,  ListenDhcpMessage,     FALSE,  8192,     AF_INET, SOCK_DGRAM, "bootps",     & BootPdPort, BOOTPD_PORT,NULL,                    FALSE,   TRUE,  TRUE,
    "TFTP",      TFTPD32_TFTP_SERVER,  TftpdMain,             FALSE,  4096,   AF_UNSPEC,          0,   "tftp", & sSettings.Port, TFTP_PORT,  sSettings.szTftpLocalIP,  TRUE,   TRUE,  TRUE,
    "SNTP",      TFTPD32_SNTP_SERVER,  SntpdProc,		      FALSE,  4096,   AF_UNSPEC, SOCK_DGRAM,    "ntp",      & SntpdPort, SNTP_PORT,  "",                      FALSE,  FALSE,  TRUE,
    "DNS",       TFTPD32_DNS_SERVER,   ListenDNSMessage,      FALSE,  4096,   AF_UNSPEC, SOCK_DGRAM, "domain",        & DnsPort, DNS_PORT,   NULL,                    FALSE,  FALSE,  TRUE,
    "Syslog",  TFTPD32_SYSLOG_SERVER,  SyslogProc,            FALSE,  4096,   AF_UNSPEC, SOCK_DGRAM, "syslog",     & SyslogPort, SYSLOG_PORT,  "",                    FALSE,  FALSE,  TRUE,
};



/////////////////////////////////////////////////////////////////
// return TRUE IPv6 is enabled on the local system
/////////////////////////////////////////////////////////////////
BOOL IsIPv6Enabled (void)
{
SOCKET s=INVALID_SOCKET;
int Rc=0;
	// just try to open an IPv6 socket
	s = socket (AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	Rc = GetLastError ();  // should be WSAEAFNOSUUPORT 10047
    closesocket (s);
return s!=INVALID_SOCKET;
} // IsIPv6Enabled

/////////////////////////////////////////////////////////////////
// common socket operations :
//      - bind its socket
//     - send a fake message on its listen port
/////////////////////////////////////////////////////////////////

// bind the thread socket 
SOCKET BindServiceSocket (const char *name, int family, int type, const char *service, int def_port, int rfc_port, const char *sz_if)
{
SOCKET             sListenSocket = INVALID_SOCKET;
int                Rc;
ADDRINFO           Hints, *res;
char               szServ[NI_MAXSERV];

   memset (& Hints, 0, sizeof Hints);
   if ( sSettings.bIPv4  &&  ! sSettings.bIPv6 && (family==AF_INET6 || family==AF_UNSPEC) )
			Hints.ai_family = AF_INET;    // force IPv4 
   else if (sSettings.bIPv6  &&  ! sSettings.bIPv4  && (family==AF_INET || family==AF_UNSPEC) )
			Hints.ai_family = AF_INET6;  // force IPv6
   else     Hints.ai_family = family;    // use IPv4 or IPv6, whichever


   Hints.ai_socktype = type;
   Hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
   wsprintf (szServ, "%d", def_port);
   Rc = getaddrinfo (sz_if!=NULL && sz_if[0]!=0 ? sz_if : NULL, 
					 def_port==rfc_port ? service : szServ, 
					 &Hints, &res);
   if (Rc!=0)
   {
         SVC_ERROR ("Error : Can't create socket\nError %d (%s)", GetLastError(), LastErrorText() );
         return sListenSocket;
   }
// bind it to the port we passed in to getaddrinfo():

   sListenSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
   if (sListenSocket == INVALID_SOCKET)
   {
         SVC_ERROR ("Error : Can't create socket\nError %d (%s)", GetLastError(), LastErrorText() );
         return sListenSocket;
   }

   	// share bindings for UDP sockets
	if (type==SOCK_DGRAM)
	{int True=1;
		Rc = setsockopt (sListenSocket, SOL_SOCKET, SO_REUSEADDR, (char *) & True, sizeof True);
		LogToMonitor (Rc==0 ? "Port %d may be reused\n" : "setsockopt error\n",
					( (struct sockaddr_in *) res->ai_addr)->sin_port);
	}


	// if family is AF_UNSPEC, allow both IPv6 and IPv4 by disabling IPV6_ONLY (necessary since Vista)
	// http://msdn.microsoft.com/en-us/library/windows/desktop/bb513665(v=vs.85).aspx
	// does not work under XP
	if (family == AF_UNSPEC)
	{int False=0;
	   Rc = setsockopt(sListenSocket, IPPROTO_IPV6, IPV6_V6ONLY, (char*)& False, sizeof False );
	}


   // bind the socket to the active interface
   Rc = bind(sListenSocket, res->ai_addr, res->ai_addrlen);

   if (Rc == INVALID_SOCKET)
   {char szAddr[MAXLEN_IPv6]="unknown", szServ[NI_MAXSERV]="unknown";
    int KeepLastError = GetLastError();
      // retrieve localhost and port
		getnameinfo (  res->ai_addr, res->ai_addrlen, 
		               szAddr, sizeof szAddr, 
				       szServ, sizeof szServ,
				       NI_NUMERICHOST | AI_NUMERICSERV );
		SetLastError (KeepLastError); // getnameinfo has reset LastError !
	   // 3 causes : access violation, socket already bound, bind on an adress 
	   switch (GetLastError ())
	   {
			case WSAEADDRNOTAVAIL :   // 10049
	  		    SVC_ERROR ("Error %d\n%s\n\n"
					   "Tftpd32 tried to bind the %s port\n"
					   "to the interface %s\nwhich is not available for this host\n"
					   "Either remove the %s service or suppress %s interface assignation",
 					    GetLastError (), LastErrorText (),
						name, sz_if, name, sz_if); 
				break;
			case WSAEINVAL :
			case WSAEADDRINUSE :
	  		    SVC_ERROR ("Error %d\n%s\n\n"
					   "Tftpd32 can not bind the %s port\n"
					   "an application is already listening on this port",
 					    GetLastError (), LastErrorText (),
						name );
				break;
			default :
				SVC_ERROR ("Bind error %d\n%s",
 					    GetLastError (), LastErrorText () );
				break;
	   } // switch error type
       closesocket (sListenSocket);
	   LogToMonitor ("bind port to %s port %s failed\n", szAddr, szServ);

   }
   freeaddrinfo (res);
return   Rc == INVALID_SOCKET ? Rc : sListenSocket;
} // BindServiceSocket


static void FreeThreadResources (int Idx)
{
	if (tThreads[Idx].skt  != INVALID_SOCKET)       closesocket (tThreads[Idx].skt);
	if (tThreads[Idx].hEv  != INVALID_HANDLE_VALUE) CloseHandle (tThreads[Idx].hEv);
    tThreads[Idx].skt = INVALID_SOCKET;
    tThreads[Idx].hEv = INVALID_HANDLE_VALUE;
	tThreads[Idx].bSoftReset = FALSE;
} //  FreeThreadResources (Ark);


/////////////////////////////////////////////////
// Wake up a thread :
// two methods : either use SetEvent or 
//               send a "fake" message (thread blocked on recvfrom)
/////////////////////////////////////////////////

static int FakeServiceMessage (const char *name, int family, int type, const char *service, int def_port, const char *sz_if)
{
SOCKET  s;
int Rc;
ADDRINFO           Hints, *res;
char               szServ[NI_MAXSERV];

   memset (& Hints, 0, sizeof Hints);
   if ( sSettings.bIPv4  &&  ! sSettings.bIPv6 && (family==AF_INET6 || family==AF_UNSPEC) )
			Hints.ai_family = AF_INET;    // force IPv4 
   else if (sSettings.bIPv6  &&  ! sSettings.bIPv4  && (family==AF_INET || family==AF_UNSPEC) )
			Hints.ai_family = AF_INET6;  // force IPv6
   else     Hints.ai_family = family;    // use IPv4 or IPv6, whichever

   Hints.ai_socktype = type;
   Hints.ai_flags = AI_NUMERICHOST;
   wsprintf (szServ, "%d", def_port);
   Rc = getaddrinfo(  (sz_if==NULL || sz_if[0]==0) ? "127.0.0.1" : sz_if, 
	                   service==NULL ? service : szServ, 
					 & Hints,
					 & res );
   if (Rc==0)
   {
      s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
      Rc = sendto (s, "wake up", sizeof ("wake up"), 0, res->ai_addr, res->ai_addrlen);
      closesocket (s);
      freeaddrinfo (res);
   }
return Rc>0;    
} // FakeServiceMessage
 
int WakeUpThread (int Idx)
{
int Rc;
   if (tThreadsConfig[Idx].wake_up_by_ev) 
   {
            Rc = SetEvent (tThreads[Idx].hEv);
			assert ( ! tThreads[Idx].gRunning  || Rc!=0);
   }
   else     FakeServiceMessage (tThreadsConfig[Idx].name,
                                tThreadsConfig[Idx].family,
                                tThreadsConfig[Idx].type,
                                tThreadsConfig[Idx].service,
                              * tThreadsConfig[Idx].def_port,
                                tThreadsConfig[Idx].sz_interface );
return TRUE;
} // WakeUpThread


// return a OR between the running threads
int GetRunningThreads (void)
{
int Ark;
int uServices = TFTPD32_NONE;
   for ( Ark=0 ;  Ark<TH_NUMBER ; Ark++ )
	   if (tThreads[Ark].gRunning)
		   uServices |= tThreadsConfig[Ark].serv_mask;
return uServices;
} // GetRunningThreads


/////////////////////////////////////////////////
// of threads life and death
/////////////////////////////////////////////////
static int StartSingleWorkerThread (int Ark)
{
   if (tThreads [Ark].gRunning)  return 0;
	   // first open socket
   if (  tThreadsConfig[Ark].type >= SOCK_STREAM )
   {
       tThreads [Ark].gRunning  = FALSE;
       tThreads[Ark].skt = BindServiceSocket (  tThreadsConfig[Ark].name,
                                                tThreadsConfig[Ark].family,
                                                tThreadsConfig[Ark].type,
                                                tThreadsConfig[Ark].service,
                                              * tThreadsConfig[Ark].def_port,
                                                tThreadsConfig[Ark].rfc_port,
                                                tThreadsConfig[Ark].sz_interface );
	   // on error try next thread
       if ( tThreads[Ark].skt  == INVALID_SOCKET ) return FALSE;
   }
   else tThreads[Ark].skt = INVALID_SOCKET ;

   // Create the wake up event
   if (tThreadsConfig [Ark].wake_up_by_ev )
   {
		tThreads [Ark].hEv  = CreateEvent ( NULL, tThreadsConfig [Ark].manual_event, FALSE, NULL );
		if ( tThreads [Ark].hEv == INVALID_HANDLE_VALUE )
		{
			FreeThreadResources (Ark);
			return FALSE;
		}
   }
   else tThreads [Ark].hEv = INVALID_HANDLE_VALUE ;
   

   tThreads[Ark].bSoftReset = FALSE;

   // now start the thread
   tThreads [Ark].tTh  = (HANDLE) _beginthread ( tThreadsConfig [Ark].thread_proc,
                                                 tThreadsConfig [Ark].stack_size,
                                                 NULL );
   if (tThreads [Ark].tTh == INVALID_HANDLE_VALUE)
   {
		FreeThreadResources (Ark);
		return FALSE;
   }
   else
   {
	   // all resources have been allocated --> status OK
	   tThreads [Ark].gRunning  = TRUE;
	   if (tThreadsConfig [Ark].gui)
	   {
		struct S_Chg_Service chgmsg;
	   // change display : ie add its tab in the GUI
		   chgmsg.service = tThreadsConfig [Ark].serv_mask;
		   chgmsg.status = SERVICE_RUNNING;
   		   SendMsgRequest (   C_CHG_SERVICE, 
  							  & chgmsg, 						
							  sizeof chgmsg,
							  FALSE,	  	    // don't block thread until msg sent
							  FALSE );		// if no GUI return
	   } // tell the gui a new service is running
   } // service correctly started
   if (Ark>TH_SCHEDULER)   SetEvent ( tThreads[TH_SCHEDULER].hEv );
return TRUE;
} // StartSingleWorkerThread


// Start all threads
int StartMultiWorkerThreads (BOOL bSoft)
{
#define INIT_MAX_ATTEMPS 30
int Ark, nToBeStarted=0, nThreadInitialized, nAttempts=0 ;
#ifdef _DEBUG
int Rc;
#endif

  for ( Ark=0 ;  Ark<TH_NUMBER ; Ark++ )
  {
		// process mangement threads and 
		if (    ( !bSoft   &&   TFTPD32_MNGT_THREADS & tThreadsConfig[Ark].serv_mask )
			 ||   sSettings.uServices  & tThreadsConfig[Ark].serv_mask )
		{
			StartSingleWorkerThread (Ark);
#ifdef SERVICE_EDITION
			// for service, do not wait for console to be connected to GUI 
			// bInit set by console only when TCP connection between service and gui done
			if (tThreadsConfig[Ark].serv_mask!=TFTPD32_CONSOLE)
#endif
				nToBeStarted++;
			// Pause to synchronise GUI and console
		} // process all threads
  }

  // GUI should run faster than the other threads.
  SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);


#ifdef _DEBUG
  // stress test synchronisation
  Rc = SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
  Rc = SetThreadPriority (tThreads [TH_CONSOLE].tTh, THREAD_PRIORITY_BELOW_NORMAL);
  if (Rc==0) Rc=GetLastError();
#endif

  // waits until the services are started
  do
  {
	  nAttempts++;
	  Sleep (100);
	  // count number of services initialized
	  for ( Ark=nThreadInitialized=0 ;  Ark<TH_NUMBER ; Ark++ ) 
		  if (tThreads [Ark].bInit) nThreadInitialized++;
  }
  while (nThreadInitialized < nToBeStarted  &&  nAttempts <INIT_MAX_ATTEMPS);
  if (nAttempts >= INIT_MAX_ATTEMPS)
  {
	// at least on service has not been started -> find which one
    for ( Ark=nThreadInitialized=0 ;  Ark<TH_NUMBER ; Ark++ ) 
		if (    (
				 ( !bSoft   &&   TFTPD32_MNGT_THREADS & tThreadsConfig[Ark].serv_mask )
				   ||   sSettings.uServices  & tThreadsConfig[Ark].serv_mask 
				)
				&& ! tThreads [Ark].bInit 
		    )
			LogToMonitor ("service %s not started", tThreadsConfig[Ark].name);
  } // log errors
  else LogToMonitor ("--- all services started, init done");


   // wake up GUI
   SendMsgRequest (C_SERVICES_STARTED, NULL, 0, FALSE, FALSE);
   // let time for the GUI to pool the services
   //Sleep (500);

  if (IsIPv6Enabled() ) 
		LogToMonitor ("IPv6 enabled");
  for (Ark=0 ; Ark<SizeOfTab (tThreadsConfig) ; Ark++ )
  	   if (tThreadsConfig [Ark].gui  &&  tThreads[Ark].gRunning)
	   {
		struct S_Chg_Service chgmsg;
	   // change display : ie add its tab in the GUI
		   chgmsg.service = tThreadsConfig [Ark].serv_mask;
		   chgmsg.status = SERVICE_RUNNING;
   		   SendMsgRequest (   C_CHG_SERVICE, 
  							  & chgmsg, 						
							  sizeof chgmsg,
							  FALSE,	  	    // don't block thread until msg sent
							  FALSE );		// if no GUI return
	   } // tell the gui a new service is running

return TRUE;
} // StartMultiWorkerThreads


void TerminateWorkerThreads (BOOL bSoft)
{
int Ark;
HANDLE tHdle[TH_NUMBER];
int nCount;
    for ( Ark=0, nCount=0 ;  Ark<TH_NUMBER ; Ark++ )
    {
	  // if bSoft do not kill management threads
	  if ( bSoft  &&  TFTPD32_MNGT_THREADS & tThreadsConfig[Ark].serv_mask)
		  continue;

      if (tThreads [Ark].gRunning)  
	  {
		  tThreads [Ark].gRunning = FALSE;
		  WakeUpThread (Ark);
          tHdle[nCount++] = tThreads [Ark].tTh;
	  } // if service is running
    }
    // wait for end of threads
    WaitForMultipleObjects (nCount, tHdle, TRUE, 5000);

    for ( Ark=0 ;  Ark<TH_NUMBER ; Ark++ )
    {
		if ( ! (bSoft  &&  TFTPD32_MNGT_THREADS & tThreadsConfig[Ark].serv_mask) )
				FreeThreadResources (Ark);
    }
    LogToMonitor ("all level 1 threads have returned\n");
} // TerminateWorkerThreads


// ---------------------------------------------------------------
// Settings has been changed : kill old threads and start new threads
// ---------------------------------------------------------------
void Tftpd32UpdateServices (void *lparam)
{
int Ark;
struct S_RestartTable *pRestart = (struct S_RestartTable *) lparam;

	// first worker thread is DHCP, last utility is SCHEDULER
	assert (TH_DHCP==TH_SCHEDULER+1);

	// scan all worker threads
	for ( Ark=TH_DHCP ;  Ark<TH_NUMBER ; Ark++)
	{BOOL bOld  = pRestart->oldservices & tThreadsConfig [Ark].serv_mask;
	 BOOL bNew  = pRestart->newservices & tThreadsConfig [Ark].serv_mask;
	 BOOL bFlap = tThreads[Ark].gRunning && (pRestart->flapservices & tThreadsConfig [Ark].serv_mask);
		// do not restart a service which is not running
		// signal thread
		if ( bFlap || (bOld && !bNew) )
		{
		    LogToMonitor ("terminating %s service\n", tThreadsConfig [Ark].name);
            tThreads[Ark].gRunning = FALSE;
			tThreads[Ark].bSoftReset = TRUE;
            WakeUpThread (Ark);
			// Now wait long enough since Scheduler has to record the end of each thread
			Sleep (200);
		}
		// not running but should be started
		if ( bFlap || (! bOld && bNew) )
		{
		    LogToMonitor ("starting %s service\n", tThreadsConfig [Ark].name);
			StartSingleWorkerThread (Ark);
		}
	} // scan all events
} // Tftpd32UpdateServices


// ---------------------------------------------------------------
//a thread which wake up periodically to check interfaces status
// ---------------------------------------------------------------

void Scheduler (void *param)
{
int Ark, Rc;
HANDLE tHdle[TH_NUMBER];
int nCount;
int  nTime=1000;

	// Increase thread priority in order to return asap in the Wait function
     SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    tThreads [TH_SCHEDULER].bInit = TRUE;  // inits OK

    do
    {
		tHdle[0] = tThreads [TH_SCHEDULER].hEv;
        for ( Ark=0, nCount=1 ;  Ark<TH_NUMBER ; Ark++ )
             if (tThreads [Ark].gRunning)     tHdle[nCount++] = tThreads [Ark].tTh;
		// wake up every 30 seconds
        Rc = WaitForMultipleObjects (nCount, tHdle, FALSE, 30000);
		if (Rc == WAIT_TIMEOUT)  
			PoolNetworkInterfaces ();	// so not trigger messages to GUI
		// a process has terminated 
		else if ( Rc - WAIT_OBJECT_0 < nCount )
		{
		struct S_Chg_Service chgmsg;

			 // thread itself is signalled, since the number of service has changed
			 if (Rc == WAIT_OBJECT_0)
			 { 
				LogToMonitor ("Scheduler signal received"); 
				ResetEvent (tThreads[Ark].hEv); 
				PoolNetworkInterfaces ();	// GUI has waked up this thread, send it fresh info
				continue; 
			 }

			for ( Ark=0 ;  Ark<SizeOfTab(tThreads)  &&  tHdle[Rc - WAIT_OBJECT_0]!=tThreads [Ark].tTh ;  Ark++ ); 
			if (Ark>=SizeOfTab(tThreads)) continue;

			LogToMonitor ("process %s has terminated\n", tThreadsConfig[Ark].name);
			// free resources allocated by StartSingleWorkerThread
			FreeThreadResources (Ark);
     		if (tThreadsConfig [Ark].gui)
 			{
				// change display : ie add its tab in the GUI
				chgmsg.service = tThreadsConfig [Ark].serv_mask;
				chgmsg.status = SERVICE_STOPPED;
   				SendMsgRequest (C_CHG_SERVICE,& chgmsg, sizeof chgmsg, FALSE, FALSE );	
			}

			// wait since uServices may be changed by Console thread !!
			if (    tThreadsConfig [Ark].restart 
				&&  sSettings.uServices & tThreadsConfig [Ark].serv_mask 
				&&  tThreads [Ark].gRunning )
				StartSingleWorkerThread (Ark);

		} // WAitMultipleObject
    }
    while ( tThreads[TH_SCHEDULER].gRunning );

	LogToMonitor ("end of ip pooling thread\n");
_endthread ();        
} // ListIPInterfaces
