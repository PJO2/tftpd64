//////////////////////////////////////////////////////
//
// Projet TFTPD32.  January 2006 Ph.jounin
// Projet DHCPD32.  January 2006 Ph.jounin
// File bootp.c:    Manage BOOTP/DHCP protocols
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////

#define _WINSOCK_DEPRECATED
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#ifndef TFTPD32
#  pragma message ("                  Dhcpd32 compilation")
#  include <windows.h>
#  include <windowsx.h>
#  include <winsock.h>
#  include "cmsgbox.h"
#  include "dhcpd.h"
#  include "dhcpd32.h"
#  include <shellapi.h>
#  include <stddef.h>       // offsetof
#  include <commctrl.h>
#else
// TFTPD32's compilation
#  // pragma message ("                  Tftpd32 compilation")
#  include "headers.h"
#endif

#include <stdio.h>          // sscanf is used
#include <process.h>        // endthread + beginthread
#include <iphlpapi.h>

#include "_libs/ping_api/ping_api.h"
#include "threading.h"
#include "bootpd_functions.h"

#include <Mswsock.h>

#define CLASS_A_LOOPBACK  127
#define DHO_CUSTOM 254


// check magic cookie
#define IsDHCP(x)    ( * (DWORD *) ((x).options) == * (DWORD *) DHCP_OPTIONS_COOKIE )


#define BOOTPC_PORT  68
#define BOOTPS_PORT  67
#define DHCP_PINGTIMEOUT    500

int gDhcpServiceRunning = TRUE;

//
// A table to display the message received
// DHCP message
struct S_DHCP_type
{
    int nType;
    char *sType;
};
static struct S_DHCP_type tDHCPType[] =
     {
        {0,             "BootP",           },
        {DHCPDISCOVER,  "DHCP Discover",   },
        {DHCPOFFER,     "DHCP Offer",      },
        {DHCPREQUEST,   "DHCP Rqst",       },
        {DHCPDECLINE,   "DHCP decline",    },
        {DHCPACK,       "DHCP Ack",        },
        {DHCPNAK,       "DHCP Nak",        },
        {DHCPRELEASE,   "DHCP release",    },
        {DHCPINFORM,    "DHCP inform"      },
        {DHCPINFORM+1,  "Invalid Pkt"     },
    };

//The bound address
static struct in_addr g_boundaddr;

// global variable : the DHCP settings
struct S_DHCP_Param  sParamDHCP;



// DHCP options
struct S_DhcpOptions
{
   unsigned nDHCPOpt;
   char     nLen;
   int      nServices;
} ; // struct S_DhcpOptions



///////////////////////
// DHCP assignations data base
// this reverse-sorted linked list contains the assignated addresses
// DHCP leases database
struct LL_IP **tFirstIP;   // sorted array of pointers on struct LL_IP
struct LL_IP **tMAC;		  //sorted Pointers to elements in tFirstIP, indexed by MAC addr
int            nAllocatedIP;    // number of item allocated (even if never acked)



static BOOL scanforleases = FALSE;
void setleaseprompt(BOOL doprompt) { 	scanforleases = doprompt; }


///////////////////////////////////////////////////////
// Address assignation
///////////////////////////////////////////////////////

// DHCP Management
// Search an option in the DHCP extension
char *DHCPSearchOptionsField (unsigned char *pOpt, int nField, int *pLength)
{
int            Ark;
unsigned char *p;
 if (* (DWORD *) pOpt != * (DWORD *) DHCP_OPTIONS_COOKIE )   return NULL;

    for ( Ark = 0,  p =  pOpt + (sizeof DHCP_OPTIONS_COOKIE - 1)  ;
          Ark<DHCP_OPTION_LEN-3  && p[Ark]!=nField ;
          Ark += (p[Ark]==DHO_PAD ? 1 : 2+p[Ark+1]) );

    if (Ark<DHCP_OPTION_LEN-3  &&  Ark+p[Ark+1] < DHCP_OPTION_LEN && p[Ark] == nField)
    {
        if (pLength!=NULL)  *pLength = p[Ark+1];
        return &p[Ark+2];
    }
return NULL;
} // DHCPSearchField


//////////////////////////////////////////////////////////////////////////////////////////////
// Struct Management
//////////////////////////////////////////////////////////////////////////////////////////////

// DHCP Scan
// for debugging purposes only
void DHCPScan (void)
{
int  Ark;
time_t tNow;
 time (&tNow);
      for (Ark = 0 ;   Ark<nAllocatedIP  ;  Ark++ )
       LOG (15, "Item %d: IP %s, Mac %s, Age %d sec, %s",
               Ark,
               inet_ntoa (tFirstIP[Ark]->dwIP),
               haddrtoa(tFirstIP[Ark]->sMacAddr, 6,':'),
               tNow - tFirstIP[Ark]->tAllocated,
               tFirstIP[Ark]->tRenewed ? "Ack" : "Nak" );
} // DHCPScan


// Create or realloc an item
struct LL_IP *DHCPReallocItem (struct LL_IP *pCur, DWORD dwNewIP, const unsigned char *pMac, int nMacLen)
{
   if (pCur==NULL)
   {
      pCur = tFirstIP[nAllocatedIP] = malloc (sizeof *tFirstIP[0]);
      pCur->dwAllocNum = nAllocatedIP;
	  tMAC[nAllocatedIP] = pCur;
      IncNumAllocated();
   }
   SetIP(pCur, dwNewIP);
   SetAllocTime(pCur);
   ZeroRenewTime(pCur);
   SetMacAddr(pCur, pMac, nMacLen);
   // sort whole array
   qsort (tFirstIP, nAllocatedIP, sizeof *tFirstIP, QsortCompare);
   qsort (tMAC, nAllocatedIP, sizeof *tMAC, MACCompare);
   ReorderLeases();
return pCur;
} // DHCPReallocItem

// frees an item (may crash if allocation in progress)
void DHCPDestroyItem (struct LL_IP *pCur)
{
	if (pCur!=NULL)
	{
		LOG (5, "Freeing item %s %s", 
							 inet_ntoa (pCur->dwIP),
							 haddrtoa  (pCur->sMacAddr, 6,':') ) ;
		// put item at the end of the list and resort array
		SetIP(pCur, INADDR_NONE);   // will be the last item
		memset(pCur->sMacAddr, 0, 6);
		qsort (tFirstIP, nAllocatedIP, sizeof *tFirstIP, QsortCompare);
		qsort (tMAC, nAllocatedIP, sizeof *tMAC, MACCompare);
		DecNumAllocated(); 
		ReorderLeases();
		free ( tFirstIP[nAllocatedIP] ); // free the last item
	}
 // wake up DHCP thread --> actualizes GUI with SendLeases
    WakeUpThread (TH_DHCP);
} // DHCPDestroyItem



//////////////////////////////////////////////////////////////////////////////////////////////
// Data Base Queries
// Retrieve a DHCP item in the data base
//////////////////////////////////////////////////////////////////////////////////////////////

// Assignation by MAC Address
DWORD DHCP_StaticAssignation (struct dhcp_packet *pPkt)
{
char          szIP[20];

return ( pPkt->htype==HTYPE_ETHER || pPkt->htype==HTYPE_IEEE802 )
        && pPkt->hlen==6
        && ReadKey ( TFTPD32_DHCP_KEY, 
                     haddrtoa(pPkt->chaddr, pPkt->hlen,':'), 
                     szIP, sizeof szIP, 
                     REG_SZ, 
                     szTftpd32IniFile )
        ?  inet_addr (szIP) : INADDR_NONE;

} // DHCP_StaticAssignation


//////////////////////////////////////////////////////////////////////////////////////////////
// IP assignation
//////////////////////////////////////////////////////////////////////////////////////////////


struct LL_IP *DHCP_IPAllocate2(struct in_addr *pPreviousAddr, const unsigned char *pMac, int nMacLen)
{
time_t  tNow;
int     Ark;
struct LL_IP *pCurIP /*, *pOldestIP*/;
#define TWO_MINUTES 120
	
//If true, the client has requested an IP address that we should try to honor
int useprev = (pPreviousAddr->s_addr != INADDR_ANY) && (AddrFitsPool(pPreviousAddr));



   // search for the previously allocated mac address
   if (nMacLen>=6) // Ethernet Mac address
   {
	   // search if mac the mac address is already known
       pCurIP = DHCPSearchByMacAddress (pMac, nMacLen);
	   if(pCurIP) 
	   {
		   //We found the previous MAC record.  If the address requested is invalid, use
		   //the address in the record.  If it is valid, erase the record, since we should
		   //give the requested address
		   if(!useprev || (pPreviousAddr->s_addr == pCurIP->dwIP.s_addr))
		   {
				LOG (12, "Reply with previously allocated : %s", inet_ntoa (pCurIP->dwIP));
				SetAllocTime(pCurIP);
				return pCurIP;
		   }
		   else //Remove the old address, and continue with the allocation logic
		   {
			   //Destroying the item takes care of the macaddr table as well.
			   DHCPDestroyItem (pCurIP);
			   pCurIP = NULL;
		   }
       } // mac address found

    } // mac address not valid


    // search if requested address can be granted
	if(useprev)
    {
		// is the address already allocated but not renewed (or expired)
		BOOL wasexpired=FALSE;
		pCurIP = DHCPSearchByIP (pPreviousAddr, &wasexpired);
		if (pCurIP!=NULL)
		{
			//Only allocate if it's to the same address, or the lease expired,
			//otherwise reset for the real allocation
			if(wasexpired || (0 == memcmp(pCurIP->sMacAddr, pMac, nMacLen)))
			{
				LOG (5, "Request for %s granted", inet_ntoa (pCurIP->dwIP));
				pCurIP = DHCPReallocItem (pCurIP, pPreviousAddr->s_addr, pMac, nMacLen);
				return pCurIP ;
			}
			else
				pCurIP = NULL;
		}
		else	//Address not allocated before, just grant it
		{
			pCurIP = DHCPReallocItem (NULL, pPreviousAddr->s_addr, pMac, nMacLen);
			LOG (12, "Reply with requested address : %s", inet_ntoa (pCurIP->dwIP));
			return pCurIP;
		} //
   } // Requested address asked

  // A new IP address should be allocated :
  // First check if the pool is large enough in order to allocate a new address
   if (sParamDHCP.nPoolSize>0   &&  nAllocatedIP < sParamDHCP.nPoolSize)
   {
    // search for an "hole" in the struct or take last elem + 1
    // if an item was allocated and the first item is the first in pool
   //Don't allocate ip addresses ending in 0 or 255
    if (nAllocatedIP>0   &&  tFirstIP[0]->dwIP.s_addr == inet_addr (sParamDHCP.szAddr))
     {
          for ( Ark=1 ;
                Ark<nAllocatedIP
               &&  ntohl (tFirstIP[Ark]->dwIP.s_addr) == AddrInc(tFirstIP[Ark-1]->dwIP);
               Ark ++ );
           pCurIP = DHCPReallocItem (NULL, htonl (AddrInc(tFirstIP[Ark-1]->dwIP)), pMac, nMacLen);
       }
      else   pCurIP = DHCPReallocItem (NULL, inet_addr (sParamDHCP.szAddr), pMac, nMacLen);
       // New address : ntohl (tFirstIP[Ark]->dwIP.s_addr) + 1
    // it is OK if Ark has reach nAllocatedIP
      LOG (12, "Reply with new : %s", inet_ntoa (pCurIP->dwIP));
    return pCurIP;
    } // new allocation

    // no free address, have to reuse an "old" one
    // try addresses which have not been acknowledged (tAllocated+2 minutes),
    // or that have expired
    //
   time (&tNow);
   for (Ark=0 ;   Ark<nAllocatedIP;   Ark++)
    {
        pCurIP = tFirstIP[Ark];
        if (    pCurIP->tAllocated+TWO_MINUTES < tNow
            &&  ((pCurIP->tRenewed==0) || (tNow > pCurIP->tRenewed + (sParamDHCP.nLease * 60))))
        {
               pCurIP = DHCPReallocItem (pCurIP, pCurIP->dwIP.s_addr, pMac, nMacLen);
             LOG (12, "Reply with reuse : %s", inet_ntoa (pCurIP->dwIP));
                return pCurIP;
        }

    } // reuse an unacknowledged address

/* Since we are replacing holes, unacked, and expired addresses, all addresses are currently used
   // search for the oldest one (use tAllocated and tRenewed)
   for (Ark=0, pOldestIP=NULL ;   Ark<nAllocatedIP;   Ark++)
    {
        pCurIP = tFirstIP[Ark];
        if (        pCurIP->tRenewed!=0
                &&  pCurIP->tRenewed < (unsigned) (pOldestIP==NULL ? 0xFFFFFFFF : pOldestIP->tRenewed )
                &&  PingApi (&pCurIP->dwIP, DHCP_PINGTIMEOUT, NULL)==PINGAPI_TIMEOUT
                &&  PingApi (&pCurIP->dwIP, DHCP_PINGTIMEOUT, NULL)==PINGAPI_TIMEOUT
                &&  PingApi (&pCurIP->dwIP, DHCP_PINGTIMEOUT, NULL)==PINGAPI_TIMEOUT
           )  pOldestIP=pCurIP;
    } // search for oldest item in pOldestIP
    if (pOldestIP != NULL)
    {
       pCurIP = DHCPReallocItem (pOldestIP, pOldestIP->dwIP.s_addr, pMac, nMacLen);
       LOG (12, "Reply with reuse : %s", inet_ntoa (pCurIP->dwIP));
       return pCurIP;  // can be NULL
    }
*/
return NULL;
} // DHCP_IPAllocate2


struct LL_IP *DHCP_IPAllocate(int nDhcpType, struct in_addr *pPreviousAddr, const unsigned char *pMac, int nMacLen)
{
int Rc=1;
struct LL_IP *pCurIP;
ULONG dummy_mac[2];
ULONG  dummy_maclen = sizeof dummy_mac;
DWORD mask = inet_addr(sParamDHCP.szMask);
BOOL  bFound=FALSE;
  do
  {
     pCurIP = DHCP_IPAllocate2 (pPreviousAddr, pMac, nMacLen);
	 // 2010/09/27  Colin from Shangai points out that ICMP check should be done only for DISCOVER
     //  if (pCurIP!=NULL  &&  sSettings.bPing  && nDhcpType==DHCPDISCOVER)
     if (pCurIP==NULL  ||  ! sSettings.bPing  ||  nDhcpType!=DHCPDISCOVER)  
		 bFound=TRUE;
	 else
     {
	   
       // frees the ARP cache and send an ARP request 
	   // nov 2013 : ping has been suppressed, trust ARP alone
	   if  (FindNearestServerAddress ( (struct in_addr *) & pCurIP->dwIP, (struct in_addr *) & mask, TRUE) != NULL)
	   {
		   // same network 
			ArpDeleteHost (pCurIP->dwIP);
			Rc = SendARP(pCurIP->dwIP.s_addr, 0, dummy_mac, &dummy_maclen);
			if (Rc == NO_ERROR)
		    {
			   LOG (2, "Suppress arp-able address %s", inet_ntoa (pCurIP->dwIP));
			   DHCPReallocItem (pCurIP, pCurIP->dwIP.s_addr, FREE_DHCP_ADDRESS, 6);
			   SetRenewTime (pCurIP);
		    }
			else bFound=TRUE;
	   }
	   else
	   {
		    Rc =    PingApi (&pCurIP->dwIP, DHCP_PINGTIMEOUT, NULL) ;
		    if (Rc>0)
		    {
		 	   LOG (2, "Suppress pingable address %s", inet_ntoa (pCurIP->dwIP));
	 		   DHCPReallocItem (pCurIP, pCurIP->dwIP.s_addr, FREE_DHCP_ADDRESS, 6);
			   SetRenewTime (pCurIP);
		    }
		    else if (Rc==PINGAPI_TIMEOUT  ||  Rc==PINGAPI_UNREACHABLE  ||  Rc==PINGAPI_TTLEXPIRE) 
					bFound=TRUE;
		    else { LOG (1, "Ping Error %d (%s)", WSAGetLastError (), LastErrorText() ); Sleep (100); bFound=TRUE; }
	   } // different network use ping instead of ARP
     } // bPing Settings
  } 
  while (pCurIP!=NULL && ! bFound); // should exit by bFound
return pCurIP;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// fill DHCP fields
//////////////////////////////////////////////////////////////////////////////////////////////
int DHCPOptionsReply(struct dhcp_packet* pDhcpPkt, int nDhcpType, struct sockaddr_in* receivingAddress, unsigned short iLastArch)
{
unsigned char  *pOpt = (unsigned char *) (pDhcpPkt->options + (sizeof DHCP_OPTIONS_COOKIE - 1));
HANDLE            hFile;
struct in_addr *pNearest;
struct in_addr  in_Aux;
int             Ark, Evan;
char            sz[256];
unsigned char  *p;
unsigned char  szDhcpAgentOpt[1024];
int				nDhcpAgentOpt=0;
unsigned char  *pSub82;
unsigned char szDhcpAgentOptSub1[128],szDhcpAgentOptSub2[128];
int           msgSize;

static struct S_DhcpOptions sDhcpOpt [] =       // 0 for unspecified
{
    DHO_DHCP_MESSAGE_TYPE,      1,	  ~TFTPD32_NONE,
    DHO_DHCP_SERVER_IDENTIFIER, 4,	  ~TFTPD32_NONE,
    DHO_SUBNET_MASK,            4,	  ~TFTPD32_NONE,
	// change : gateway not mandatory anymore
    DHO_ROUTERS,                0,	  ~TFTPD32_NONE,
    DHO_DOMAIN_NAME_SERVERS,    0,	  ~TFTPD32_NONE,
    DHO_NETBIOS_NAME_SERVERS,   0,	  ~TFTPD32_NONE,
    DHO_DHCP_LEASE_TIME,        4,	  ~TFTPD32_NONE,
    DHO_DHCP_RENEWAL_TIME,      4,	  ~TFTPD32_NONE,
    DHO_DHCP_REBINDING_TIME,    4,	  ~TFTPD32_NONE,
    DHO_LOG_SERVERS,            4,	   TFTPD32_SYSLOG_SERVER,
    DHO_NTP_SERVERS,			0,	  ~TFTPD32_NONE,
    DHO_SIP_SERVERS,			0,	  ~TFTPD32_NONE,
    DHO_BOOT_SIZE,              0,	   TFTPD32_TFTP_SERVER,
    DHO_TFTP_SERVER,			0,     TFTPD32_TFTP_SERVER,
    DHO_DOMAIN_NAME,            0,	  ~TFTPD32_NONE,
    DHO_DHCP_AGENT_OPTIONS,     0,	  ~TFTPD32_NONE,
    DHO_CUSTOM,                 0,	  ~TFTPD32_NONE,
    DHO_END,                    0,	  ~TFTPD32_NONE,
};



	// Save Agent Options
       p = DHCPSearchOptionsField (pDhcpPkt->options, DHO_DHCP_AGENT_OPTIONS, NULL);
       if (p!=NULL)   
	   {
		   p--;
		   nDhcpAgentOpt = *p;
           memcpy (szDhcpAgentOpt, p+1,  nDhcpAgentOpt);
	   }

	//Always pack the magic cookie again, just in case it was corrupted
	*(DWORD*)(pDhcpPkt->options) = * (DWORD*) DHCP_OPTIONS_COOKIE;

	// pNearest points on the "good" LAN interface
//   pNearest = FindNearestServerAddress (&pDhcpPkt->yiaddr, & sParamDHCP.dwMask, FALSE);
   in_Aux.s_addr=inet_addr(sParamDHCP.szMask);
   pNearest = FindNearestServerAddress (&pDhcpPkt->yiaddr, & in_Aux, TRUE);
   // from Philip Taff : choose the receiving interface instead of a loopback
   if (!pNearest) 
       pNearest = &(receivingAddress->sin_addr);
  
   //HACK -- If we are the bootp server, we are also the tftpserver
   if (sSettings.uServices & TFTPD32_TFTP_SERVER) 
      pDhcpPkt->siaddr = *pNearest;   // Next server (TFTP server is enabled)
   pDhcpPkt->siaddr = *pNearest;   // always fill siaddr
   for (Ark=0 ; Ark<SizeOfTab(sDhcpOpt) ; Ark++)
   {
	 // skip if linked to a service which is not started (change suggested by Colin)
	 if (! (sSettings.uServices & sDhcpOpt[Ark].nServices)) 
			continue;

     if (sDhcpOpt[Ark].nLen!=0) 
     {
        *pOpt++ = (unsigned char) sDhcpOpt[Ark].nDHCPOpt ; 
        *pOpt++ = (unsigned char) sDhcpOpt[Ark].nLen;
     }
     switch (sDhcpOpt[Ark].nDHCPOpt)
     {
	case DHO_DHCP_MESSAGE_TYPE       :  * pOpt = (unsigned char) nDhcpType ; break ; 
	case DHO_LOG_SERVERS             :  // fallthrough
	case DHO_DHCP_SERVER_IDENTIFIER  :  * (DWORD *) pOpt = pNearest->s_addr; break ;
	case DHO_TFTP_SERVER             :  
                  *pOpt++ = DHO_TFTP_SERVER;
				  *pOpt   = lstrlen (sSettings.szTftpLocalIP[0]!=0 ? sSettings.szTftpLocalIP : inet_ntoa (* pNearest) );
                   memcpy (pOpt+1, sSettings.szTftpLocalIP[0]!=0 ? sSettings.szTftpLocalIP : inet_ntoa (* pNearest), *pOpt);
                   pOpt += 1+*pOpt; 
				   break;

	case DHO_SUBNET_MASK             :  * (DWORD *) pOpt = inet_addr(sParamDHCP.szMask); break ;
//       case DHO_ROUTERS                 :  * (DWORD *) pOpt = (sParamDHCP.dwGateway.s_addr == 0xffffffff ? pDhcpPkt->yiaddr.s_addr : sParamDHCP.dwGateway.s_addr); break ;
	case DHO_ROUTERS                 : 
			// if gateway is not valid, forget this option
		   if ( inet_addr(sParamDHCP.szGateway)!=INADDR_NONE )
		   {
			   *pOpt++ = (unsigned char) DHO_ROUTERS ; 
               *pOpt++ = 4;
  			   * (DWORD *) pOpt = inet_addr(sParamDHCP.szGateway); 
				pOpt += 4;
		   }
		   break ;
//       case DHO_DOMAIN_NAME_SERVERS     :  * (DWORD *) pOpt = sParamDHCP.dwDns.s_addr;  break; 
	 case DHO_DOMAIN_NAME_SERVERS: 

			if ( (strlen(sParamDHCP.szDns1)!=0  &&  inet_addr (sParamDHCP.szDns1)!=0x0) )
			{
				BOOL bkp_dns = (strlen(sParamDHCP.szDns2)!=0  &&  inet_addr (sParamDHCP.szDns2)!=0x0);
                  *pOpt++ = DHO_DOMAIN_NAME_SERVERS;
                  *pOpt++ = bkp_dns ? 8 : 4;
				  * (DWORD *) pOpt = inet_addr (sParamDHCP.szDns1);
				  if (bkp_dns) * (DWORD *) (pOpt+4) = inet_addr (sParamDHCP.szDns2);
				   pOpt += bkp_dns ? 8 : 4;
            }
            break;		   
     case DHO_NETBIOS_NAME_SERVERS    :
			if (!(strlen(sParamDHCP.szWins)==0 || inet_addr (sParamDHCP.szWins)==0x0))
            {
                  *pOpt++ = DHO_NETBIOS_NAME_SERVERS;
                  *pOpt++ = 4;
				  * (DWORD *) pOpt = inet_addr (sParamDHCP.szWins);
				   pOpt += 4;
            }
            break;
     case DHO_DHCP_LEASE_TIME         :  * (DWORD *) pOpt = htonl (sParamDHCP.nLease * 60);  break ;
     case DHO_DHCP_RENEWAL_TIME       :  * (DWORD *) pOpt = htonl (sParamDHCP.nLease/2 * 60);  break ;
     case DHO_DHCP_REBINDING_TIME     :  * (DWORD *) pOpt = htonl ((sParamDHCP.nLease*80)/100 * 60);  break ;
     case DHO_BOOT_SIZE               :
              // translate $IP$ and $MAC$ from boot file name
              TranslateExp (sParamDHCP.szBootFile, sz, pDhcpPkt->yiaddr, pDhcpPkt->chaddr, iLastArch);
              hFile = CreateFile(sz,        // open the file
                                 GENERIC_READ,                 // open for reading
                                 FILE_SHARE_READ,              // share for reading
                                 NULL,                         // no security
                                 OPEN_EXISTING,                // existing file only
                                 FILE_ATTRIBUTE_NORMAL,       // normal file
                                 NULL);                       // no attr. template
               if (hFile != INVALID_HANDLE_VALUE)
               {
                  *pOpt++ = DHO_BOOT_SIZE ;
                  *pOpt++ = sizeof (unsigned short);
                  * (unsigned short *) pOpt = htons ( (unsigned short) (1+GetFileSize (hFile, NULL) / 512) ) ;
                  pOpt += sizeof (unsigned short);
                  CloseHandle( hFile ) ;         // close the file
               }
              break;
     case DHO_NTP_SERVERS             :
		if (!(strlen(sParamDHCP.szOpt42)==0 || inet_addr (sParamDHCP.szOpt42)==0x0))
             {
                  *pOpt++ = DHO_NTP_SERVERS;
                  *pOpt++ = 4;
				  * (DWORD *) pOpt = inet_addr (sParamDHCP.szOpt42);
				   pOpt += 4;
             }
             break;
     case DHO_SIP_SERVERS             :
		if (!(strlen(sParamDHCP.szOpt120)==0 || inet_addr (sParamDHCP.szOpt120)==0x0))
             {
                  *pOpt++ = DHO_SIP_SERVERS;
                  *pOpt++ = 5;
				  *pOpt++ = 1;
				  * (DWORD *) pOpt = inet_addr (sParamDHCP.szOpt120);
				   pOpt += 4;
             }
             break;
     case DHO_DOMAIN_NAME             :
             if (sParamDHCP.szDomainName[0]!=0)
             {
                  *pOpt++ = DHO_DOMAIN_NAME;
                  *pOpt   = lstrlen (sParamDHCP.szDomainName);
                   memcpy (pOpt+1, sParamDHCP.szDomainName, *pOpt);
                   pOpt += 1+*pOpt; 
              }
              break;
     case DHO_DHCP_AGENT_OPTIONS       :
             if (nDhcpAgentOpt!=0)
             {
                 *pOpt++ = DHO_DHCP_AGENT_OPTIONS;
                 *pOpt++ = nDhcpAgentOpt;
                  memcpy (pOpt, szDhcpAgentOpt, nDhcpAgentOpt);
                  pOpt += nDhcpAgentOpt; 
				  pSub82=szDhcpAgentOpt;
				  szDhcpAgentOptSub1[0]=szDhcpAgentOptSub2[0]=0;
				  while (pSub82!=NULL && pSub82 < szDhcpAgentOpt+nDhcpAgentOpt-2)
				  {
					 if (pSub82[0]==0x01 && pSub82[1] < 127)
					 {
						memcpy (szDhcpAgentOptSub1, pSub82+2, pSub82[1]);
						szDhcpAgentOptSub1[pSub82[1]]=0;
						pSub82+=pSub82[1]+2;
					 }
					 else if (pSub82[0]==0x02 && pSub82[1] < 127)
					 {
						memcpy (szDhcpAgentOptSub2, pSub82+2, pSub82[1]);
						szDhcpAgentOptSub2[pSub82[1]]=0;
						pSub82+=pSub82[1]+2;
					 }
					 else pSub82=NULL;
				  }
				  LOG (5, "Adding option 82, length: %d ; CirID: <%s>; RemID: <%s>; ",
					  nDhcpAgentOpt, szDhcpAgentOptSub1,szDhcpAgentOptSub2);
             }
             break;
	 case DHO_CUSTOM : // Manage custom options
         for (Evan=0 ; Evan < SizeOfTab (sParamDHCP.t) ; Evan++)
                if (sParamDHCP.t[Evan].nAddOption != 0)
                {
                   *pOpt++ = (unsigned char) sParamDHCP.t[Evan].nAddOption;
				   *pOpt = TranslateParam2Value (pOpt+1, 64, sParamDHCP.t[Evan].szAddOption, 
								 pDhcpPkt->yiaddr, pDhcpPkt->chaddr, iLastArch);
                   pOpt += 1+*pOpt;
               }
              break;
     case DHO_END                     : 
                 *pOpt++ = DHO_END;
 //              *pOpt++ = DHO_PAD;
 //              *pOpt++ = DHO_PAD;
                 break;
     } // switch option
     pOpt += sDhcpOpt[Ark].nLen ;    // points on next field
   } // for all option

   // 2017-11-05 pad to 64 bytes
   msgSize = (int) (pOpt - (unsigned char*) pDhcpPkt);
   if  (msgSize < 64)
   {
	   memset (pOpt, 0, 64 - msgSize);
	   msgSize = 64;
   }
return msgSize;
} // DHCPOptionsReply


///////////
// Process DHCP msg : return TRUE if an answer has been prepared

int iCounterDHCP=0;
int ProcessDHCPMessage (struct dhcp_packet *pDhcpPkt, int *pSize,  struct sockaddr_in* receivingAddress)
{
unsigned char *p=NULL;
struct LL_IP  *pCurIP=NULL, *pProposedIP=NULL;	// Thanks Sam Leitch !
int            Ark, nDhcpType = 0;
struct in_addr in_RequestedAddr;
DWORD sStaticIP;
unsigned short iLastArch;	// architecture required by client

    if (IsDHCP (*pDhcpPkt))
    {
       // search DHCP message type
       p = DHCPSearchOptionsField (pDhcpPkt->options, DHO_DHCP_MESSAGE_TYPE, NULL);
       if (p!=NULL)        nDhcpType = *p;
     }
    if (pDhcpPkt->yiaddr.s_addr!=INADDR_ANY  &&  pDhcpPkt->yiaddr.s_addr!=INADDR_NONE )
            return FALSE ; // address already assigned

	// Get Architecture requested by client
	p = DHCPSearchOptionsField(pDhcpPkt->options, DHO_PXE_CLIENT_ARCH_ID, NULL);
	if (p != NULL)
	{
		iLastArch = ntohs(* (unsigned short *) p);
		LOG(3, "iLastArch %d", iLastArch);
	}

     // the tab has one undef raw
     for (Ark=0 ; Ark<SizeOfTab(tDHCPType)-1 && nDhcpType!=tDHCPType[Ark].nType ; Ark++) ;
     LOG (5, "Rcvd %s Msg for IP %s, Mac %s",
                      tDHCPType[Ark].sType,
                      inet_ntoa (pDhcpPkt->ciaddr),
                      haddrtoa(pDhcpPkt->chaddr, pDhcpPkt->hlen,':'));


    // if (sParamDHCP.nPoolSize==0) return FALSE;   // no allocation pool --> listen only

     switch (nDhcpType)
     {
        case 0           :    // BootP
            if(sParamDHCP.nIgnoreBootp)
            {
               LOG (5, "Ignoring Bootp request");
               break;
            }
        case DHCPDISCOVER :
            sStaticIP = DHCP_StaticAssignation (pDhcpPkt);
			in_RequestedAddr.s_addr=pDhcpPkt->ciaddr.s_addr;
			if (sStaticIP != INADDR_NONE)
			{
               LOG (0, "%s: statically assigned to address %s", 
								haddrtoa(pDhcpPkt->chaddr, pDhcpPkt->hlen,':'), 
								inet_ntoa (* (struct in_addr *) & sStaticIP) );
               pDhcpPkt->yiaddr.s_addr = sStaticIP;
			}
			else
			{
               p  = DHCPSearchOptionsField (pDhcpPkt->options, DHO_DHCP_REQUESTED_ADDRESS, NULL);
               if (p!=NULL)
              {
// Change to avoid ciaddr to be returned if not comming. ciaddr returned will be the received one
				   in_RequestedAddr = * (struct in_addr *) p;
                   LOG (5, "Client requested address %s", inet_ntoa (in_RequestedAddr));
              }
              pProposedIP  = DHCP_IPAllocate (nDhcpType, & in_RequestedAddr, pDhcpPkt->chaddr, pDhcpPkt->hlen);
              if (pProposedIP == NULL)
              {
                  LOG (1, "no more address or address previously allocated by another server");
                  return FALSE;
              }
              pDhcpPkt->yiaddr.s_addr = pProposedIP->dwIP.s_addr;
              LOG (2, "%s: proposed address %s", IsDHCP(*pDhcpPkt) ? "DHCP" : "BOOTP", inet_ntoa (pProposedIP->dwIP) );
			  iCounterDHCP=0;
              //If this is a bootp, there is no other response from the client.  
              //Since we don't want leases expiring (or being mistaken for unAcked DHCP offers),
              //set renewed to a distant time
              if(nDhcpType == 0  &&  sStaticIP == INADDR_NONE)     // patched by Rolf Offermanns
                  ForceRenewTime(pProposedIP, 0x66666666);         // fixed by Sam Leitch
            } // dynamically assigned address

            // populate the packet to be returned
            pDhcpPkt->op = BOOTREPLY;
            // translate $IP$ and $MAC$ from boot file name
            TranslateExp (sParamDHCP.szBootFile, pDhcpPkt->file, pDhcpPkt->yiaddr, pDhcpPkt->chaddr, iLastArch);
           *pSize = DHCPOptionsReply (pDhcpPkt, DHCPOFFER, receivingAddress, iLastArch);
            break ;

		//NJW Changed how requests are handled to mimic linux -- requests are responded to even if we didn't originally allocate, but only if the requested address is in our pool range

        case DHCPREQUEST :
         {BOOL bSERVER = FALSE;  // TRUE if Tftpd32 has assigned this address
           // Static Allocation ?
             // search field REQUEST ADDR in options
            sStaticIP = DHCP_StaticAssignation (pDhcpPkt);
			if (sStaticIP != INADDR_NONE)
			{
			    // populate the packet to be returned
                   pDhcpPkt->op = BOOTREPLY;
                   pDhcpPkt->yiaddr.s_addr = sStaticIP;
                 // translate $IP$ and $MAC$ from boot file name
                 TranslateExp (sParamDHCP.szBootFile, pDhcpPkt->file, pDhcpPkt->yiaddr, pDhcpPkt->chaddr, iLastArch);
                   *pSize = DHCPOptionsReply (pDhcpPkt, DHCPACK, receivingAddress, iLastArch);
				   break;
			}

           // has tftpd32 dynamically assigned this address
			pCurIP = DHCPSearchByMacAddress (pDhcpPkt->chaddr, pDhcpPkt->hlen);
			if (pCurIP==NULL)  return FALSE; // not attributed by Tftpd32 --> do not answer

			// ok we have proposed an IP address for this MAC Address, 
			// but does the client request for our assignation 
			// look either at the field ciaddr or at the DHO_DHCP_REQUESTED_ADDRESS option

            // search field REQUEST ADDR in options
            // if specified should fit database
			in_RequestedAddr.s_addr= pDhcpPkt->ciaddr.s_addr;
			p  = DHCPSearchOptionsField (pDhcpPkt->options, DHO_DHCP_REQUESTED_ADDRESS, NULL);
            if (p!=NULL)
			{
// Change to avoid ciaddr to be returned if not comming. ciaddr returned will be the received one
//				pDhcpPkt->ciaddr = * (struct in_addr *) p;
				in_RequestedAddr = * (struct in_addr *) p;
			}
//			if(AddrFitsPool(&pDhcpPkt->ciaddr))
			if(AddrFitsPool(&in_RequestedAddr))
			{
				//Look up the address, if it's not found, or the owner is this macaddr,
				//or the lease was expired, allow the serving.
				BOOL wasexpired = FALSE;
//				pProposedIP = DHCPSearchByIP(&pDhcpPkt->ciaddr, &wasexpired);
				pProposedIP = DHCPSearchByIP(&in_RequestedAddr, &wasexpired);
				bSERVER = !pProposedIP || wasexpired || (0 == memcmp(pProposedIP->sMacAddr, pDhcpPkt->chaddr, 6));
			}

			if (bSERVER  ||  (pDhcpPkt->ciaddr.S_un.S_addr==0 && sSettings.bPXECompatibility) )
			{

//				pProposedIP  = DHCP_IPAllocate (nDhcpType, & pDhcpPkt->ciaddr, pDhcpPkt->chaddr, pDhcpPkt->hlen);
				pProposedIP  = DHCP_IPAllocate (nDhcpType, & in_RequestedAddr, pDhcpPkt->chaddr, pDhcpPkt->hlen);
				if (pProposedIP == NULL)
				{
					  LOG (1, "no more addresses or address previously allocated by another server");
					  return FALSE;
				}
				if (pProposedIP->tAllocated==0) SetAllocTime(pProposedIP);
				SetRenewTime(pProposedIP);
				LOG (5, "Previously allocated address %s acked", inet_ntoa (pProposedIP->dwIP));
				// populate the packet to be returned
				pDhcpPkt->op = BOOTREPLY;
				pDhcpPkt->yiaddr.s_addr = pProposedIP->dwIP.s_addr;
				TranslateExp (sParamDHCP.szBootFile, pDhcpPkt->file, pDhcpPkt->yiaddr, pDhcpPkt->chaddr, iLastArch);
				*pSize = DHCPOptionsReply (pDhcpPkt, DHCPACK, receivingAddress, iLastArch);
			}
			else
			{
				LOG (5, "Client requested address %s which was not allocated by tftpd32 and is either outside our pool or is used by someone else",
							  inet_ntoa (in_RequestedAddr) );
				return FALSE ; // do not answer
 			}
           } // Block for bSERVER declaration
           break;


        case DHCPDECLINE :
             // search current item and its precedent
          pCurIP = DHCPSearchByMacAddress (pDhcpPkt->chaddr, pDhcpPkt->hlen);
           if (pCurIP!=NULL)
           {
             p  = DHCPSearchOptionsField (pDhcpPkt->options, DHO_DHCP_REQUESTED_ADDRESS, NULL);
             if (p!=NULL) 
              {
                 in_RequestedAddr.s_addr = * (DWORD *) p;
                 if ( pCurIP->dwIP.s_addr==in_RequestedAddr.s_addr) 
                 {
                     DHCPDestroyItem (pCurIP);
                     LOG (5, "item destroyed");
                 }
             }
           }
		   //The decline is sent when an address is already in use.  Do an ARP and 
		   //add a lease for the in-use address
		   {
			 ULONG mac[2];
			 ULONG maclen = 6;
			// search field REQUEST ADDR in options
			// if specified should fit database
			p  = DHCPSearchOptionsField (pDhcpPkt->options, DHO_DHCP_REQUESTED_ADDRESS, NULL);
			if (p!=NULL)
			{
				pDhcpPkt->ciaddr = * (struct in_addr *) p;
			}

			 if(NO_ERROR == SendARP(pDhcpPkt->ciaddr.s_addr, 0, mac, &maclen))
			 {
				pProposedIP  = DHCP_IPAllocate (nDhcpType, & pDhcpPkt->ciaddr, (unsigned char*)mac, maclen);
				if (pProposedIP)
				{
					if (pProposedIP->tAllocated==0) SetAllocTime(pProposedIP);
					ForceRenewTime(pProposedIP, 0x66666666);   //Give a bootp lease, since the device may not do dhcp      
					LOG (5, "Added lease for existing address %s", inet_ntoa (pProposedIP->dwIP));
				}
			 }
		   }
           break;

        case DHCPRELEASE :
            // do not destroy the item but mark it free
           pCurIP = DHCPSearchByMacAddress (pDhcpPkt->chaddr, pDhcpPkt->hlen);
           if (pCurIP!=NULL) // then mac address found in table
           {
                ZeroAllocTime(pCurIP);
                ZeroRenewTime(pCurIP);
                LOG (5, "item %s released", haddrtoa(pDhcpPkt->chaddr, pDhcpPkt->hlen,':') );
           }
           break;
       } // switch type


DHCPScan();

// answer only to BootP, Request or discover msg
return  (nDhcpType==0 || nDhcpType==DHCPDISCOVER || nDhcpType==DHCPREQUEST);
} // ProcessDHCPMessage


//===========================================================
//NJW Added these to ping the range with one socket
//===========================================================


SOCKET g_rawsocket = INVALID_SOCKET;
BOOL g_terminateping = FALSE;



//Added this for use by the ping scan.  This is not thread safe with
//the main stuff, so don't call when you are listening for DHCP messages.
void ArpAndAdd (struct in_addr *paddr)
{
	 ULONG mac[2];
	 ULONG maclen = 6;

	 if(NO_ERROR == SendARP(paddr->s_addr, 0, mac, &maclen))
	 {
		struct LL_IP* pProposedIP  = DHCP_IPAllocate (DHCPDISCOVER, paddr, (unsigned char*)mac, maclen);
		if (pProposedIP)
		{
			if (pProposedIP->tAllocated==0) SetAllocTime(pProposedIP);
			ForceRenewTime(pProposedIP, 0x66666666);   //Give a bootp lease, since the device may not do dhcp      
			LOG (5, "Added lease for existing address %s", inet_ntoa (pProposedIP->dwIP));
		}
	 }
}   // ArpAndAdd

//This is the recv thread to receive the ping responses
void PingRecv(void* h)
{
	while(!g_terminateping)
	{
		ECHOREPLY echoReply;
		ICMPHDR *pIcmpReply = & echoReply.echoRequest.icmpHdr;
		struct sockaddr_in saFrom;
		int nAddrLen = sizeof(struct sockaddr_in);
		int nRet;

	    memset (& saFrom, 0, sizeof saFrom);

        nRet = recvfrom(g_rawsocket,                  // socket
                  (LPSTR)&echoReply,  // buffer
                  sizeof(ECHOREPLY),  // size of buffer
                  0,                  // flags
                  (LPSOCKADDR)&saFrom,    // From address
                  &nAddrLen);         // pointer to address len

		if(!g_terminateping && (nRet > 0) && (pIcmpReply->Type == ICMP_ECHO_REPLY))
			ArpAndAdd(& saFrom.sin_addr);
	}
	
}



//This will attempt to ping and arp the entire range and add them to the lease file
//Don't call this while you are processing DHCP, as the leases are not thread safe.
void PingRange(struct in_addr* pstart, DWORD count)
{
	struct sockaddr_in saDest;
	ECHOREQUEST echoReq;
	unsigned short nSeq;
	HANDLE hread;
	int nRet;

	if(!pstart)
		return;

	//For each addr, skipping .0 and .255, we'll issue a ping and sleep for 20ms.
	//After all are issued, we'll wait 5 seconds before terminating the thread.

	g_rawsocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (g_rawsocket == SOCKET_ERROR)  
		return;

	//Start the thread
	g_terminateping = FALSE;
	hread = (HANDLE) _beginthread (PingRecv, 0, NULL);
	if((int) hread == -1)
    {
        LOG (1, "can create thread for ping Range");
		return;
    }
	//We'll do most of the packing once
    memset (& saDest, 0, sizeof saDest);
	saDest.sin_addr.s_addr = pstart->s_addr;  //Will change over time
    saDest.sin_family = AF_INET;
    saDest.sin_port = 0;
    // Fill in echo request static parts
    echoReq.icmpHdr.Type        = ICMP_ECHO_REQUEST;
    echoReq.icmpHdr.Code        = 0;
    echoReq.icmpHdr.Checksum    = 0;
    echoReq.icmpHdr.ID          = htons(PINGAPI_MYID);
   // Fill in some data to send
   for (nRet = 0; nRet < REQ_DATASIZE; nRet++)     echoReq.cData[nRet] = ' '+nRet;
   
   for(nSeq = 0; nSeq < count; ++nSeq)
   {
		echoReq.icmpHdr.Seq = htons(nSeq);
		echoReq.dwTime = htonl(GetTickCount());
		// When we compute checksum, we must remove the chksum from the old cycle
		echoReq.icmpHdr.Checksum = 0;
		echoReq.icmpHdr.Checksum = in_cksum((unsigned short *)&echoReq, sizeof(ECHOREQUEST));
	
		sendto (g_rawsocket, (LPSTR) &echoReq,  sizeof echoReq, 0,
                 (LPSOCKADDR) & saDest, sizeof saDest);

		//Just to make sure it got out, send another one shortly after
		Sleep(5);
		sendto (g_rawsocket, (LPSTR) &echoReq,  sizeof echoReq, 0,
                 (LPSOCKADDR) & saDest, sizeof saDest);

		//Smart increment curaddr
		saDest.sin_addr.s_addr = htonl(AddrInc(saDest.sin_addr));

		Sleep(20);
   }

   //We're done, wait for some more results and terminate
   Sleep(5000);
   g_terminateping = TRUE;
   closesocket(g_rawsocket);
   g_rawsocket = INVALID_SOCKET;

}


// ---------------------------------
// send the answer using broadcast and the source address specified into siaddr files
// PJO : Nov 2013

int DHCPSingleSend (SOCKET sDHCPskt, struct sockaddr_in *pTo, struct dhcp_packet *pDhcpPkt, int nSize, BOOL bUniCast)
{
int Rc;
struct servent *lpServEnt;
int save_port = pTo->sin_port;
    // really simple if DHCP bound to one address : use it for answer
	// if no source address was specified in the incoming request reply with a broadcast
	if (!bUniCast)
		pTo->sin_addr.s_addr = htonl (INADDR_BROADCAST);	// broadcast

	// answer to incoming port (67 or 68)
	Rc = sendto ( sDHCPskt,
			  		(char *) pDhcpPkt,
					nSize,
					0,
					(struct sockaddr *) pTo,
					sizeof *pTo);
	LOG (15, "Thread 0x%X: send %d bytes", GetCurrentThreadId(), nSize );
	if (Rc<nSize)
		LOG (1, "sendto error %d: %s", GetLastError(), LastErrorText ());

	if (     sSettings.bDoubleAnswer 
		 &&  (pDhcpPkt->giaddr.s_addr!=htonl(INADDR_ANY)  || pDhcpPkt->giaddr.s_addr!=htonl(INADDR_BROADCAST)) 
		)
		{
			lpServEnt = getservbyname ("bootpc", "udp") ;
			// duplicates answer to sends to port bootps
			pTo->sin_port =  (lpServEnt != NULL) ?  lpServEnt->s_port : htons (BOOTPC_PORT);
			Rc = sendto ( sDHCPskt,
						 (char *) pDhcpPkt,
						  nSize,
						  0,
						 (struct sockaddr *) pTo,
						  sizeof *pTo);
			pTo->sin_port = save_port;
	} // double answer for relay
return Rc;
}  // DHCPSingleSend


// Do what Windows should have done : send the broadcast 
int DHCPSendFrom (struct sockaddr_in *pFrom, struct sockaddr_in *pTo, struct dhcp_packet *pDhcpPkt, int nSize)
{
SOCKET snd;
int True = TRUE;
int Rc;
struct servent *lpServEnt;
int save_port = pTo->sin_port;


	// create new socket
	snd = socket(AF_INET, SOCK_DGRAM, 0);
	if (snd == INVALID_SOCKET)
	{
		Rc = WSAGetLastError ();
		return -1;
	}
	Rc = setsockopt (snd, SOL_SOCKET, SO_REUSEADDR, (const char *) & True, sizeof True);

	lpServEnt = getservbyname ("bootps", "udp") ;
	pFrom->sin_port = (lpServEnt != NULL) ?  lpServEnt->s_port : htons (BOOTPS_PORT);
	// bind it with info provided by getaddrinfo (specify source interface and source port)
	Rc = bind(snd, (struct sockaddr *) pFrom, sizeof *pFrom);
	if (Rc==-1)
	{
		Rc = WSAGetLastError ();
		closesocket (snd);
		return -1;
	}
	// add broadcast permission to socket
	Rc = setsockopt (snd, SOL_SOCKET, SO_BROADCAST, (char *) & True, sizeof True);
	if (Rc==-1)
	{
		Rc = WSAGetLastError ();
		closesocket (snd);
		return -1;
	}

	pTo->sin_addr.s_addr = htonl (INADDR_BROADCAST);	// broadcast
	// answer to incoming port (67 or 68)
	Rc = sendto ( snd,
			  	(char *) pDhcpPkt,
				 nSize,
				 0,
				(struct sockaddr *) pTo,
				 sizeof *pTo);
	LOG (15, "Thread 0x%X: send %d bytes", GetCurrentThreadId(), nSize );
	if (Rc<nSize)
		LOG (1, "sendto error %d: %s", GetLastError(), LastErrorText ());

	// Added : DHCP relay detection --> send replies to port 67 and 68
	// Colin and others point out that this is wrong. I guess they are right.
	// However it should not be an issue and i am sure that the host receive an address !
	// Added a setting for this hack
	if (sSettings.bDoubleAnswer && 
	(pDhcpPkt->giaddr.s_addr!=htonl(INADDR_ANY)  || pDhcpPkt->giaddr.s_addr!=htonl(INADDR_BROADCAST)))
	{
		// duplicates answer to sends to port bootps
		lpServEnt = getservbyname ("bootps", "udp") ;
		pTo->sin_port =  (lpServEnt != NULL) ?  lpServEnt->s_port : htons (BOOTPC_PORT);
		Rc = sendto ( snd,
						(char *) pDhcpPkt,
						nSize,
						0,
						(struct sockaddr *) pTo,
						sizeof *pTo);
		pTo->sin_port = save_port;
	}
	closesocket (snd);
return Rc;
}




// recv a broadcast and note the address of the interface which has received the broadcast
int SktRcvAndGetAddrOfIncomingIf (SOCKET skt, char *buf, int bufsize, struct sockaddr_in *from, struct sockaddr_in *to)
{ 
GUID WSARecvMsg_GUID = WSAID_WSARECVMSG;
static LPFN_WSARECVMSG WSARecvMsg;
WSABUF wsaBuf;
WSAMSG wsaMsg;
DWORD BytesRecv=-1, NumberOfBytes = 0;
DWORD Flags = 0;
char ControlBuffer[1024];
WSACMSGHDR *pMsgHdr;
IN_PKTINFO *pPktInfo;
ULONG outBufLen;
IP_ADAPTER_ADDRESSES       *pAddresses=NULL, *pCurrAddresses;
int Rc;

	memset (to, 0, sizeof *to);
	// first get pointer on WSARecvMsg call
    if (WSARecvMsg==NULL)
	{
		Rc = WSAIoctl( tThreads[TH_DHCP].skt, SIO_GET_EXTENSION_FUNCTION_POINTER,
					   & WSARecvMsg_GUID, sizeof WSARecvMsg_GUID,
					   & WSARecvMsg, sizeof WSARecvMsg,
					   & NumberOfBytes, NULL, NULL);
		if (Rc==-1) { SetLastError (WSAVERNOTSUPPORTED); return -1; }
	}

	//-----------------------------------------------
	// Call the WSARecvMsg function to receive datagrams
	// on the bound socket.
	wsaBuf.len = bufsize;
	wsaBuf.buf = buf;
	wsaMsg.name = (LPSOCKADDR) from;
	wsaMsg.namelen = sizeof *from;
	wsaMsg.lpBuffers = & wsaBuf;
	wsaMsg.dwBufferCount = 1;
	wsaMsg.Control.len = sizeof ControlBuffer;
	wsaMsg.Control.buf = ControlBuffer;
	wsaMsg.dwFlags = 0;
	Rc = WSARecvMsg  (skt, &wsaMsg, &BytesRecv, NULL, NULL);
	if (Rc == SOCKET_ERROR) {
		Rc = WSAGetLastError();
		return -1;
	}

	// ok WSARecvMsg has succeeded, IP_PKINFO should be available in the headers
	pMsgHdr = WSA_CMSG_FIRSTHDR(&wsaMsg);
	for( pMsgHdr = WSA_CMSG_FIRSTHDR( &wsaMsg ); 
		 pMsgHdr!=NULL  &&  pMsgHdr->cmsg_type!=IP_PKTINFO; 
		 pMsgHdr = WSA_CMSG_NXTHDR( &wsaMsg, pMsgHdr  ) )	;

	// not found (?)
	if (pMsgHdr==NULL)  return BytesRecv;

	pPktInfo = (IN_PKTINFO *)WSA_CMSG_DATA(pMsgHdr);

	// now search in the interfaces list, GetAdaptersAddresses called twice : one to have the size to be allocated
	pAddresses = NULL;
	outBufLen = 0;
	Rc = GetAdaptersAddresses ( AF_INET, 
								GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_SKIP_MULTICAST, 
								NULL, 
								pAddresses, 
								& outBufLen);
	if (Rc==ERROR_BUFFER_OVERFLOW)
		pAddresses = (IP_ADAPTER_ADDRESSES *) malloc (outBufLen);
	Rc = GetAdaptersAddresses (AF_INET, 
								GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_SKIP_MULTICAST, 
								NULL, 
								pAddresses, 
								& outBufLen);
	if (Rc == NO_ERROR) 
	{
		// parse all interfaces until match pPktInfo->ipi_ifindex
		for ( pCurrAddresses = pAddresses ;
				pCurrAddresses != NULL && pCurrAddresses->IfIndex!=pPktInfo->ipi_ifindex; 
				pCurrAddresses=pCurrAddresses->Next ) ;
		// bug found by Chris Morris
		if (pCurrAddresses!=NULL  && pCurrAddresses->FirstUnicastAddress !=NULL) 
			* to =  * (struct sockaddr_in *) pCurrAddresses->FirstUnicastAddress->Address.lpSockaddr;
		free (pAddresses);
	}
return BytesRecv;
} // SktRcvAndGetAddrOfIncomingIf


// /////////////////////
// DHCP Thread
// /////////////////////

void ListenDhcpMessage (void *lpVoid)
{
struct dhcp_packet      sDhcpPkt;
char szHostname [128], *p;
int                     Rc, nSize;
struct S_WorkerParam   *pParam = (struct S_WorkerParam *) lpVoid;
struct sockaddr_in      SockFrom, SockTo;
int                     True=TRUE, nFromLen = sizeof SockFrom;
BOOL                    bUniCast;

    DHCPReadConfig ();
	tThreads[TH_DHCP].bInit = TRUE;		// inits OK

	// let the GUI start
    Sleep (1000);

	 //NJW Prompt to see if we should discover via ping before we start doing DHCP
	 if(scanforleases)
// && IDYES == CMsgBox(pParam->hWnd, "Should I reset the lease file and rediscover devices?", "Discover Devices", MB_YESNO))
	 {
		struct in_addr addr;
		int count;
		ReadKey(TFTPD32_DHCP_KEY, KEY_DHCP_POOL, & addr.s_addr, sizeof(addr.s_addr), REG_DWORD, szTftpd32IniFile);
		ReadKey(TFTPD32_DHCP_KEY, KEY_DHCP_POOLSIZE, &count, sizeof(count), REG_DWORD, szTftpd32IniFile);
		SetNumAllocated(0);
		FreeLeases(FALSE);
		if (sSettings.bPersLeases)  LoadLeases ();
		if (sSettings.bPing)        PingRange (&addr, count);
		// DHCPSaveConfig ();
		SVC_WARNING ("Lease file updated.\nDiscover Devices");
	 }

    // add send broadcast capacity to DHCP listening socket 
    Rc =    setsockopt (tThreads[TH_DHCP].skt, SOL_SOCKET, SO_BROADCAST, (const char *) & True, sizeof True)==0
		&&  setsockopt (tThreads[TH_DHCP].skt, IPPROTO_IP, IP_RECEIVE_BROADCAST, (char *) & True, sizeof True)==0 ;
    if (! Rc)
	{
		SVC_WARNING ("Can add broadcast capability to DHCP socket ! \nError %d", WSAGetLastError());
    }
    // add receive broadcast capacity to DHCP listening socket 
	// if (  LOBYTE(LOWORD(  GetVersion ()  ))  >= 6)
	Rc = setsockopt (tThreads[TH_DHCP].skt, IPPROTO_IP, IP_PKTINFO, (char *) & True, sizeof True) ;		
	if (Rc==-1)
	{
		SVC_WARNING ("Can add PKTINFO capability to DHCP socket ! \nError %d", WSAGetLastError());
		Rc = WSAGetLastError ();
    }

   while ( tThreads[TH_DHCP].gRunning )
   {
        // send leases to GUI
        Dhcp_Send_Leases (tFirstIP, nAllocatedIP);
        memset (& sDhcpPkt, 0, sizeof sDhcpPkt);

		// try to receive request and get incoming interface (need at last XP or windows server 2003)
		Rc = SktRcvAndGetAddrOfIncomingIf (tThreads[TH_DHCP].skt, 
											(char *) & sDhcpPkt,
											sizeof sDhcpPkt,
											& SockFrom,
											& SockTo );
		// 2nd chance for old OS, try recvfrom
		// should succeed if host is NOT multi-homed
		if (Rc==-1  && GetLastError()==WSAVERNOTSUPPORTED)
		{
			Rc = recvfrom ( tThreads[TH_DHCP].skt,
							(char *) & sDhcpPkt,
							sizeof sDhcpPkt,
							0,
							(struct sockaddr *) & SockFrom,
							& nFromLen);
		}
		
		// recv error
      // since Tftpd32 sends broadcasts, it receives its own message, just ignore it
        if (Rc < 0)
        {
             if (GetLastError () != WSAECONNRESET)
              {
                 LOG (1, "Recv error %d", GetLastError ());
                 Sleep (500);
              }
              continue;
        } // recv failed

        // if msg is too short
        // If all bootP fields have been read
        if (Rc < offsetof ( struct dhcp_packet, options ))
        {
           LOG (5, "Message truncated (length was %d)", Rc);
           if ( tThreads[TH_DHCP].gRunning ) Sleep (500);
           continue;
        }

		// receive on a unbound interface
		if (    sSettings.szDHCPLocalIP[0]!=0 
			&&  SockTo.sin_addr.s_addr != inet_addr (sSettings.szDHCPLocalIP))
        {
           LOG (1, "Message received on an unbound interface (IP %s)", inet_ntoa (SockTo.sin_addr));
           Sleep (10);
           continue;
        }

        // if pool is empty and MAC address not statically assigned : ignore request
        if (    sParamDHCP.nPoolSize == 0  
            &&  DHCP_StaticAssignation (& sDhcpPkt)==INADDR_NONE )
        {
           Sleep (10);
           continue;
        }

        // handle only nul-terminated strings
        sDhcpPkt.sname[sizeof sDhcpPkt.sname - 1] = 0;
        sDhcpPkt.file [sizeof sDhcpPkt.file  - 1] = 0;

        // read host name, truncate it
        if (gethostname (szHostname , sizeof szHostname )==SOCKET_ERROR)
              lstrcpy (szHostname, "Tftpd32DchpServer");
        if ((p=strchr (szHostname, '.'))!=NULL)  *p=0;
        szHostname [sizeof sDhcpPkt.sname - 1] = 0;

        if (sDhcpPkt.sname[0]!=0  && lstrcmp (sDhcpPkt.sname, szHostname)!=0)
        {
           LOG (2, "Packet addressed to %s", sDhcpPkt.sname);
            continue;
        }

        // we have only to answer to BOOTREQUEST msg
        if (sDhcpPkt.op != BOOTREQUEST)
        {
            LOG (2, "%d Request %d not processed", GetCurrentThreadId (),sDhcpPkt.op);
            continue ;
        }

        // if request OK and answer ready
        bUniCast =  (     SockFrom.sin_addr.s_addr!=htonl (INADDR_NONE)
                      &&  SockFrom.sin_addr.s_addr!=htonl (INADDR_ANY)
                      // fix 5/02/2006 : 127.0.0.2 should be handle as a broadcast
                      &&  SockFrom.sin_addr.S_un.S_un_b.s_b1 != CLASS_A_LOOPBACK ) ; // class A 127
        if (ProcessDHCPMessage ( & sDhcpPkt, & nSize, &SockTo ) )
        {
//            BinDump ((char *)&sDhcpPkt, sizeof sDhcpPkt, "DHCP");
		   // send reply 
		   // -> if unicast replies, use Windows default behaviour
		   // -> otherwise, send a broadcast on the incoming interface (which is not the default)
		   if (bUniCast)
				DHCPSingleSend (tThreads[TH_DHCP].skt, & SockFrom, & sDhcpPkt, nSize, bUniCast);
		   else
			    DHCPSendFrom (& SockTo, & SockFrom, & sDhcpPkt, nSize);
				
        }  // ProcessDHCPMessage

   } // do it eternally

LogToMonitor ("DHCP thread ends here\n");
    _endthread ();
} // ListenDhcpMessage





