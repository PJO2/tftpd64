//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Mai 98 Ph.jounin - June 2006
// File tftp.c:   worker threads management
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////


// number of permanent worker threads
#define TFTP_PERMANENTTHREADS 2

#define TFTP_MAXTHREADS     100

#undef min
#define min(a,b)  (((a) < (b)) ? (a) : (b))


// #define DEB_TEST

#include "headers.h"
#include <process.h>
#include <stdio.h>

#include "threading.h"
#include "tftpd_functions.h"

// First item -> structure belongs to the module and is shared by all threads
struct LL_TftpInfo *pTftpFirst;
static int gSendFullStat=FALSE;		// full report should be sent




// statistics requested by console 
// do not answer immediately since we are in console thread
// and pTftp data may change
void ConsoleTftpGetStatistics (void)
{
	gSendFullStat = TRUE;
} // 



////////////////////////////////////////////////////////////
// TFTP daemon --> Runs at main level
////////////////////////////////////////////////////////////



static SOCKET TftpBindLocalInterface (void)
{
SOCKET             sListenSocket = INVALID_SOCKET;
int                Rc;
ADDRINFO           Hints, *res;
char               szServ[NI_MAXSERV];
char               szIPv4 [MAXLEN_IPv6];

   memset (& Hints, 0, sizeof Hints);
   if ( sSettings.bIPv4  &&  ! sSettings.bIPv6  )
			Hints.ai_family = AF_INET;    // force IPv4 
   else if (sSettings.bIPv6  &&  ! sSettings.bIPv4   )
			Hints.ai_family = AF_INET6;  // force IPv6
   else     Hints.ai_family = AF_UNSPEC;    // use IPv4 or IPv6, whichever


   Hints.ai_socktype = SOCK_DGRAM;
   Hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
   wsprintf (szServ, "%d", sSettings.Port==0 ? TFTP_DEFPORT : sSettings.Port );
   // retrieve IPv4 address assigned to a physical Interface
   if (sSettings.bTftpOnPhysicalIf)
   {
	   Rc = GetIPv4Address (sSettings.szTftpLocalIP, szIPv4);
	   if (Rc!=0)  return sListenSocket;
   	   Rc = getaddrinfo ( szIPv4, 
						    sSettings.Port == 69 ? "tftp" : szServ, 
			               &Hints, &res);
	   if (Rc!=0)  return sListenSocket;
   }
   else
   {
       Rc = getaddrinfo ( isdigit (sSettings.szTftpLocalIP[0]) ? sSettings.szTftpLocalIP : NULL, 
		    			  sSettings.Port == 69 ? "tftp" : szServ, 
			             &Hints, &res);
		if (Rc!=0)
		{
			if (GetLastError() == WSAHOST_NOT_FOUND)
			{
	  		    SVC_ERROR ("Error %d\n%s\n\n"
					   "Tftpd32 tried to bind the %s port\n"
					   "to the interface %s\nwhich is not available for this host\n"
					   "Either remove the %s service or suppress %s interface assignation",
 					    GetLastError (), LastErrorText (),
						"tftp", sSettings.szTftpLocalIP, "tftp", sSettings.szTftpLocalIP); 
			}
			else
			{
				SVC_ERROR ("Error : Can't create socket\nError %d (%s)", GetLastError(), LastErrorText() );
			}
			return sListenSocket;
		}
   }
	// bind it to the port we passed in to getaddrinfo():

	sListenSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sListenSocket == INVALID_SOCKET)
	{
			SVC_ERROR ("Error : Can't create socket\nError %d (%s)", GetLastError(), LastErrorText() );
			freeaddrinfo (res);
			return sListenSocket;
	}


   	// REUSEADDR option in order to allow thread to open 69 port
	if (sSettings.bPortOption==0)
	{int True=1;
		Rc = setsockopt (sListenSocket, SOL_SOCKET, SO_REUSEADDR, (char *) & True, sizeof True);
		LogToMonitor (Rc==0 ? "Port %d may be reused" : "setsockopt error", sSettings.Port);
	}

	// if family is AF_UNSPEC, allow both IPv6 and IPv4 by disabling IPV6_ONLY (necessary since Vista)
	// http://msdn.microsoft.com/en-us/library/windows/desktop/bb513665(v=vs.85).aspx
	if (sSettings.bIPv4 && sSettings.bIPv6)
	{int False=0;
	   Rc = setsockopt(sListenSocket, IPPROTO_IPV6, IPV6_V6ONLY, (char*)& False, sizeof False );
	   // do not check Rc, since XP does not support this settings -> error
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
						"tftp", sSettings.szTftpLocalIP, "tftp", sSettings.szTftpLocalIP); 
				break;
			case WSAEINVAL :
			case WSAEADDRINUSE :
	  		    SVC_ERROR ("Error %d\n%s\n\n"
					   "Tftpd32 can not bind the %s port\n"
					   "an application is already listening on this port",
 					    GetLastError (), LastErrorText (),
						"tftp" );
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

} // TftpBindLocalInterface


static void PopulateTftpdStruct (struct LL_TftpInfo *pTftp)
{
struct LL_TftpInfo *pTmp;
static DWORD TransferId=467;    // unique identifiant

    // init or reinit struct
    pTftp->s.dwTimeout = sSettings.Timeout;
    pTftp->s.dwPacketSize = TFTP_SEGSIZE;  // default
    pTftp->r.skt = INVALID_SOCKET;
    pTftp->r.hFile = INVALID_HANDLE_VALUE;
    pTftp->c.bMCast = FALSE;    // may be updated later
	pTftp->c.nOAckPort = 0;		// use classical port for OAck
    pTftp->tm.dwTransferId = TransferId++;

    // init statistics
    memset (& pTftp->st, 0, sizeof pTftp->st);
    time (& pTftp->st.StartTime);
    pTftp->st.dLastUpdate = pTftp->st.StartTime;
	pTftp->st.ret_code = TFTP_TRF_RUNNING;
    // count the transfers (base 0)
    for ( pTftp->st.dwTransfert=0, pTmp = pTftpFirst->next ;
          pTmp!=NULL ;
          pTmp = pTmp->next, pTftp->st.dwTransfert++ ) ;
    LOG (9, "Transfert #%d", pTftp->st.dwTransfert);

   // init MD5 structure
    pTftp->m.bInit = sSettings.bMD5;

    // clear buffers
    memset (& pTftp->b, 0, sizeof pTftp->b);
} // PopulateTftpdStruct

// Suppress structure item
static struct LL_TftpInfo *TftpdDestroyThreadItem (struct LL_TftpInfo *pTftp)
{
struct LL_TftpInfo *pTmp=pTftp;

    LOG (9, "thread %d has exited", pTftp->tm.dwThreadHandle);

LogToMonitor ("removing thread %d (%p/%p/%p)\n", pTftp->tm.dwThreadHandleId, pTftp, pTftpFirst, pTftpFirst->next);
    // do not cancel permanent Thread
    if (! pTftp->tm.bPermanentThread )
    {
        if (pTftp!=pTftpFirst)
        {
              // search for the previous struct
            for (pTmp=pTftpFirst ; pTmp->next!=NULL && pTmp->next!=pTftp ; pTmp=pTmp->next);
            pTmp->next = pTftp->next;   // detach the struct from list
        }
        else pTftpFirst = pTmp = pTftpFirst->next;

        memset (pTftp, 0xAA, sizeof *pTftp); // fill with something is a good debugging tip
        free (pTftp);
    }

return pTmp;	// pointer on previous item
} // TftpdDestroyThreadItem


// --------------------------------------------------------
// Filter incoming request
// add-on created on 24 April 2008
// return TRUE if message should be filtered
// --------------------------------------------------------
static int TftpMainFilter (SOCKADDR_STORAGE *from, int from_len, char *data, int len)
{
static char LastMsg[PKTSIZE];
static int  LastMsgSize;
static time_t LastDate;
static SOCKADDR_STORAGE LastFrom;

	if (len > PKTSIZE) return TRUE;	// packet should really be dropped
	// test only duplicated packets
	if (    len==LastMsgSize  
		&&  memcmp (data, LastMsg, len)==0
		&&  memcmp (from, & LastFrom, from_len) == 0
		&&  time (NULL) == LastDate )
	{char szAddr[MAXLEN_IPv6]="", szServ[NI_MAXSERV]="";
	 int Rc;
		Rc = getnameinfo ( (LPSOCKADDR) from, sizeof from, 
		                    szAddr, sizeof szAddr, 
				            szServ, sizeof szServ,
				            NI_NUMERICHOST | NI_NUMERICSERV );
		 
		LOG (1, "Warning : received duplicated request from %s:%s", szAddr, szServ);
		Sleep (250);	// time for the first TFTP thread to start
		return FALSE;	// accept message nevertheless
	}
	// save last frame

	LastMsgSize = len;
	memcpy (LastMsg, data, len);
	LastFrom = *from;
	time (&LastDate);
	return FALSE; // packet is OK
} // TftpMainFilter


// activate a new thread and pass control to it 
static int TftpdChooseNewThread (SOCKET sListenerSocket)
{
struct LL_TftpInfo *pTftp, *pTmp;
int             fromlen;
int             bNewThread;
int             Rc;
int             nThread=0;

    for (  pTmp = pTftpFirst ;  pTmp!=NULL ;  pTmp = pTmp->next)   
		nThread++;
	// if max thread reach read datagram and quit
    if (nThread >= sSettings.dwMaxTftpTransfers)
    {char dummy_buf [PKTSIZE];
     SOCKADDR_STORAGE    from;
        fromlen = sizeof from;
        // Read the connect datagram to empty queue
        Rc = recvfrom (sListenerSocket, dummy_buf, sizeof dummy_buf, 0,
                       (struct sockaddr *) & from, & fromlen);
        if (Rc>0) 
		{char szAddr[MAXLEN_IPv6];
		     getnameinfo ( (LPSOCKADDR) & from, sizeof from, 
		                    szAddr, sizeof szAddr, 
				            NULL, 0,
				            NI_NUMERICHOST );
             LOG (1, "max number of threads reached, connection from %s dropped", szAddr );
		}
        return -1;
    }

    // search a permanent thread in waiting state
    for (pTftp=pTftpFirst ; pTftp!=NULL  ; pTftp=pTftp->next)
        if ( pTftp->tm.bPermanentThread  &&  ! pTftp->tm.bActive )  break;
    bNewThread = (pTftp==NULL);

    if (bNewThread)
    {
        // search for the last thread struct
        for ( pTftp=pTftpFirst ;  pTftp!=NULL && pTftp->next!=NULL ; pTftp=pTftp->next );
        if (pTftp==NULL)   pTftp=pTftpFirst =calloc (1, sizeof *pTftpFirst);
        else               pTftp=pTftp->next=calloc (1, sizeof *pTftpFirst);
        // note due the calloc if thread has just been created
        //   pTftp->tm.dwThreadHandle == NULL ;
        pTftp->next = NULL ;
    }

    PopulateTftpdStruct (pTftp);

    // Read the connect datagram (since this use a "global socket" port 69 its done here)
    fromlen = sizeof pTftp->b.cnx_frame;
    Rc = recvfrom (sListenerSocket, pTftp->b.cnx_frame, sizeof pTftp->b.cnx_frame, 0,
               (struct sockaddr *)&pTftp->b.from, &fromlen);
    if (Rc < 0)
    {
        // the Tftp structure has been created --> suppress it
        LOG (0, "Error : RecvFrom returns %d: <%s>", WSAGetLastError(), LastErrorText());
        if (! pTftp->tm.bPermanentThread )
        {
            // search for the last thread struct
            for ( pTmp=pTftpFirst ;  pTmp->next!=pTftp ; pTmp=pTmp->next );
            pTmp->next = pTftp->next ; // remove pTftp from linked list
            free (pTftp);
        }
    }
	// should the message be silently dropped
    else if (TftpMainFilter (& pTftp->b.from, sizeof pTftp->b.from, pTftp->b.cnx_frame, Rc))
	{char szAddr[MAXLEN_IPv6];
		getnameinfo ( (LPSOCKADDR) & pTftp->b.from, sizeof pTftp->b.from,  
		               szAddr, sizeof szAddr, 
				       NULL, 0,
				       NI_NUMERICHOST | AI_NUMERICSERV );
        // If this is an IPv4-mapped IPv6 address; drop the leading part of the address string so we're left with the familiar IPv4 format.
		// Hack copied from the Apache source code
		if ( pTftp->b.from.ss_family == AF_INET6 
			 && IN6_IS_ADDR_V4MAPPED ( & (* (struct sockaddr_in6 *) & pTftp->b.from ).sin6_addr ) )
        {
			memmove (szAddr, szAddr + sizeof ("::ffff:") - 1, strlen (szAddr + sizeof ("::ffff:") -1) +1 );        
		}
        LOG (1, "Warning : Unaccepted request received from %s", szAddr);
        // the Tftp structure has been created --> suppress it
		if (! pTftp->tm.bPermanentThread )
        {
            // search for the last thread struct
            for ( pTmp=pTftpFirst ;  pTmp->next!=pTftp ; pTmp=pTmp->next );
            pTmp->next = pTftp->next ; // remove pTftp from linked list
            free (pTftp);
        }
	}
	else	// message is accepted
    {char szAddr[MAXLEN_IPv6], szServ[NI_MAXSERV];
		getnameinfo ( (LPSOCKADDR) & pTftp->b.from, sizeof pTftp->b.from, 
		               szAddr, sizeof szAddr, 
				       szServ, sizeof szServ,
				       NI_NUMERICHOST | AI_NUMERICSERV );
        // If this is an IPv4-mapped IPv6 address; drop the leading part of the address string so we're left with the familiar IPv4 format.
		if ( pTftp->b.from.ss_family == AF_INET6 
			 && IN6_IS_ADDR_V4MAPPED ( & (* (struct sockaddr_in6 *) & pTftp->b.from ).sin6_addr ) )
        {
			memmove (szAddr, szAddr + sizeof ("::ffff:") - 1, strlen (szAddr + sizeof ("::ffff:") -1) +1 );        
		}
        LOG (1, "Connection received from %s on port %s", szAddr, szServ);
#if (defined DEBUG || defined DEB_TEST)
        BinDump (pTftp->b.cnx_frame, Rc, "Connect:");
#endif		

        // mark thread as started (will not be reused)
        pTftp->tm.bActive=TRUE ;

        // start new thread or wake up permanent one
        if (bNewThread)
        {
            // create the worker thread
            // pTftp->tm.dwThreadHandle = (HANDLE) _beginthread (StartTftpTransfer, 8192, (void *) pTftp);
            pTftp->tm.dwThreadHandle = CreateThread (NULL,
                                                     8192,
                                                     StartTftpTransfer,
                                                     pTftp,
                                                     0,
                                                     & pTftp->tm.dwThreadHandleId);
LogToMonitor ("Thread %d transfer %d started (records %p/%p)\n", pTftp->tm.dwThreadHandleId, pTftp->tm.dwTransferId, pTftpFirst, pTftp);
            LOG (9, "thread %d started", pTftp->tm.dwThreadHandle);

        }
        else                 // Start the thread
        {
    LogToMonitor ("waking up thread %d for transfer %d\n",
                   pTftp->tm.dwThreadHandleId,
                   pTftp->tm.dwTransferId );
            if (pTftp->tm.hEvent!=NULL)       SetEvent (pTftp->tm.hEvent);
        }
       // Put the multicast hook which adds the new client if the same mcast transfer
       // is already in progress

    } // recv ok --> thread has been started

return TRUE;
} // TftpdStartNewThread


static void SendStatsToGui (BOOL bFullStats)
{
static struct S_TftpTrfStat sMsg;
struct LL_TftpInfo         *pTftp;
int                         Ark;

   // if full report should be sent, one message per transfer is generates
   if (bFullStats)
   {
      for ( pTftp=pTftpFirst ;  pTftp!=NULL ; pTftp=pTftp->next )
	  {
		  if (pTftp->tm.bActive) ReportNewTrf (pTftp);   // from tftp_thread !
	  }
   }
   else
   {
	   for ( Ark=0,  pTftp=pTftpFirst ;  Ark<SizeOfTab(sMsg.t)  &&  pTftp!=NULL ; pTftp=pTftp->next )
	   {
		  if (pTftp->tm.bActive )
		  {
			  sMsg.t[Ark].dwTransferId = pTftp->tm.dwTransferId;
			  sMsg.t[Ark].stat = pTftp->st;
			  Ark++ ;
		  }
	   }
	   sMsg.nbTrf = Ark;
	   time (& sMsg.dNow);
	   //if (Ark>0)
			SendMsgRequest (  C_TFTP_TRF_STAT, 
							& sMsg , 
							  sMsg.nbTrf * sizeof (sMsg.t[0]) + offsetof (struct S_TftpTrfStat, t[0]),
							  TRUE,		// block thread until msg sent
							  FALSE );		// if no GUI return
   }
} // SendStatsToGui


////////////////////////////////////////////////////////////
// Init TFTP daemon
////////////////////////////////////////////////////////////

static int CreatePermanentThreads (void)
{
int Ark;
struct LL_TftpInfo *pTftp;


    // inits socket
    ////////////////////////////////////////////
    // Create the permanents threads
    for ( Ark=0  ; Ark < TFTP_PERMANENTTHREADS ; Ark++ )
    {
        if (pTftpFirst==NULL)  pTftp=pTftpFirst= calloc (1, sizeof *pTftpFirst);
        else                   pTftp=pTftp->next=calloc (1, sizeof *pTftpFirst);
        pTftp->next = NULL;
        pTftp->tm.bPermanentThread = TRUE;
        pTftp->tm.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        pTftp->tm.N = Ark+1 ;
        //pTftp->tm.dwThreadHandle = (HANDLE) _beginthread (StartTftpTransfer, 4096, (void *) pTftp);
        pTftp->tm.dwThreadHandle = CreateThread (NULL,
                                                 8192,
                                                 StartTftpTransfer,
                                                 pTftp,
                                                 0,
                                                 & pTftp->tm.dwThreadHandleId);
        pTftp->r.hFile=INVALID_HANDLE_VALUE ;
    }

return TRUE;
}  // CreatePermanentThreads


static int TftpdCleanup (SOCKET sListenerSocket)
{
struct LL_TftpInfo *pTftp, *pTmp;
   // suspend all threads
    for (pTftp=pTftpFirst ; pTftp!=NULL ; pTftp=pTftp->next )
        if (pTftp->tm.dwThreadHandle!=NULL) SuspendThread (pTftp->tm.dwThreadHandle);

    if (WSAIsBlocking ())  WSACancelBlockingCall ();   // the thread is probably blocked into a select

    // then frees resources
    for (pTftp=pTftpFirst ; pTftp!=NULL ; pTftp=pTmp )
    {
        pTmp=pTftp->next ;

        if (pTftp->r.skt!=INVALID_SOCKET)          closesocket (pTftp->r.skt),   pTftp->r.skt=INVALID_SOCKET;
        if (pTftp->r.hFile!=INVALID_HANDLE_VALUE)  CloseHandle(pTftp->r.hFile),  pTftp->r.hFile!=INVALID_HANDLE_VALUE;
        if (pTftp->tm.hEvent!=NULL)                CloseHandle(pTftp->tm.hEvent),pTftp->tm.hEvent!=NULL;
        free (pTftp);
    }
    Sleep (100);
   // kill the threads
    for (pTftp=pTftpFirst ; pTftp!=NULL ; pTftp=pTftp->next )
        if (pTftp->tm.dwThreadHandle!=NULL) TerminateThread (pTftp->tm.dwThreadHandle, 0);

     // close main socket
     closesocket (sListenerSocket);
return TRUE;
} // TftpdCleanup


// a watch dog which reset the socket event if data are available
static int ResetSockEvent (SOCKET s, HANDLE hEv)
{
long dwData;
int Rc;
   Rc = ioctlsocket ( s ,  FIONREAD, & dwData);
   if (dwData==0) ResetEvent (hEv);
return Rc;   
}


// ---------------------------------------------------------------
// Main
// ---------------------------------------------------------------
void TftpdMain (void *param)
{
int Rc;
int parse;
HANDLE hSocketEvent = INVALID_HANDLE_VALUE;
struct LL_TftpInfo *pTftp;
// events : either socket event or wake up by another thread
enum { E_TFTP_SOCK=0, E_TFTP_WAKE, E_TFTP_EV_NB };
HANDLE tObjects [E_TFTP_EV_NB];

    // creates socket and starts permanent threads
        if (pTftpFirst==NULL)  CreatePermanentThreads ();


	 tThreads [TH_TFTP].bInit = TRUE;  // inits OK

	// Socket was not opened at the start since we have to use interface 
    // once an address as been assigned
	while ( tThreads[TH_TFTP].gRunning  )
	{
		// if socket as not been created before
		if (tThreads[TH_TFTP].skt == INVALID_SOCKET)
		{
		   tThreads[TH_TFTP].skt = TftpBindLocalInterface ();
		} // open the socket
	   if (tThreads[TH_TFTP].skt == INVALID_SOCKET)
	   {
		   break;
	   }

	    // create event for the incoming Socket
		hSocketEvent = WSACreateEvent();
		Rc = WSAEventSelect (tThreads[TH_TFTP].skt, hSocketEvent, FD_READ);
		Rc = GetLastError ();

		tObjects[E_TFTP_SOCK] = hSocketEvent;
		tObjects[E_TFTP_WAKE] = tThreads[TH_TFTP].hEv;

		// stop only when TFTP is stopped and all threads have returned

		// waits for either incoming connection or thread event
		Rc = WaitForMultipleObjects ( E_TFTP_EV_NB,
										tObjects,
										FALSE,
										sSettings.dwRefreshInterval );
#ifdef RT                                      
if (Rc!=WAIT_TIMEOUT) LogToMonitor ( "exit wait, object %d\n", Rc);
#endif
		if (! tThreads[TH_TFTP].gRunning ) break;

		switch (Rc)
		{
			case E_TFTP_WAKE :   // a thread has exited
									// update table
				do
				{
					parse=FALSE;
					for ( pTftp=pTftpFirst ; pTftp!=NULL ; pTftp=pTftp->next ) 
					{
						if  (! pTftp->tm.bPermanentThread && ! pTftp->tm.bActive )
						{
if (pTftp==NULL) { LogToMonitor ("NULL POINTER pTftpFirst:%p\n", pTftpFirst); Sleep (5000); break; }
							CloseHandle (pTftp->tm.dwThreadHandle);
							pTftp = TftpdDestroyThreadItem (pTftp);
							parse = TRUE;
							break;
						} // thread no more active
					}
				} // parse all threads (due to race conditions, we can have only one event)
				while (parse);
				break;

			// we have received a message on the port 69
			case E_TFTP_SOCK :    // Socket Msg
				WSAEventSelect (tThreads[TH_TFTP].skt, 0, 0);
				TftpdChooseNewThread (tThreads[TH_TFTP].skt);
				ResetEvent( hSocketEvent );
				WSAEventSelect (tThreads[TH_TFTP].skt, hSocketEvent, FD_READ);
				// ResetSockEvent (sListenerSocket, hSocketEvent);
				break;

			case  WAIT_TIMEOUT :
				SendStatsToGui(gSendFullStat); // full stat flag may be set by console
				gSendFullStat = FALSE;         // reset full stat flag
				// ResetSockEvent (sListenerSocket, hSocketEvent);
				break;
			case -1 :
	LogToMonitor ( "WaitForMultipleObjects error %d\n", GetLastError() );
				LOG (1, "WaitForMultipleObjects error %d", GetLastError());
				break;
		}   // switch


		CloseHandle(hSocketEvent);

    } // endless loop



    // TftpdCleanup (sListenerSocket, hSemaphore);

	// Should the main thread kill other threads ?
	if ( tThreads[TH_TFTP].bSoftReset )
				LogToMonitor ("do NOT signal worker threads\n");
	else
	{
		LogToMonitor ("signalling worker threads\n");
			/////////////////////////////////
			// wait for end of worker threads
			for (pTftp=pTftpFirst ; pTftp!=NULL  ; pTftp=pTftp->next)
			{
				if (pTftp->tm.bActive)                nak (pTftp, ECANCELLED);
				else if (pTftp->tm.bPermanentThread)  SetEvent (pTftp->tm.hEvent);
			}
		LogToMonitor ("waiting for worker threads\n");

			while ( pTftpFirst != NULL )
			{
				WaitForSingleObject (pTftpFirst->tm.dwThreadHandle, 10000);
				LogToMonitor ("End of thread %d\n", pTftpFirst->tm.dwThreadHandleId);
				pTftpFirst->tm.bPermanentThread = FALSE;
				TftpdDestroyThreadItem (pTftpFirst);
			}
	} // Terminate sub threads

	Rc = closesocket (tThreads[TH_TFTP].skt); 
	tThreads[TH_TFTP].skt=INVALID_SOCKET;
    WSACloseEvent (hSocketEvent);

LogToMonitor ("main TFTP thread ends here\n");
_endthread ();
} // Tftpd main thread


