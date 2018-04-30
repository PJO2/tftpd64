//////////////////////////////////////////////////////
//
// Projet DHCPD32.         January 2006 Ph.jounin
//                         modified Nick Wagner
// PING_API.C -- Ping program using ICMP and RAW Sockets
//
//
// source released under European Union Public License
// 
//////////////////////////////////////////////////////


#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#include <process.h>
#include <iphlpapi.h>

#include "ping_api.h"


#ifdef _MSC_VER
#  pragma pack()
#endif


// Internal Functions
static int      WaitForEchoReply(SOCKET s);
unsigned short  in_cksum(unsigned short *addr, int len);
static int        SendEchoRequest(SOCKET, LPSOCKADDR_IN);
static DWORD    RecvEchoReply(SOCKET, LPSOCKADDR_IN, unsigned char *);


//===========================================================

// PingApi()
// Calls SendEchoRequest() and
// RecvEchoReply() and prints results
int PingApi   (struct in_addr *pAddr, DWORD dwTimeout_msec, int *pTTL)
{
//LPHOSTENT lpHost;
//struct sockaddr_in    saSrc;
//unsigned char    cTTL;
static ECHOREQUEST      echoReq;
static int              nSeq = 1;
SOCKET                  rawSocket;
struct sockaddr_in      saDest;
struct sockaddr_in      saFrom;
DWORD                   dwElapsed, dwStart;
int                     nRet;
DWORD                   dwCurrentTimeout;
ECHOREPLY               echoReply;
int                     nAddrLen = sizeof(struct sockaddr_in);
ICMPHDR                *pIcmpReply = & echoReply.echoRequest.icmpHdr;

    memset (& saDest, 0, sizeof saDest);
    memset (& saFrom, 0, sizeof saFrom);

    // Create a Raw socket
   rawSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
   if (rawSocket == SOCKET_ERROR) 
   {
	   int Rc = WSAGetLastError();
	   return Rc==WSAEACCES ? PINGAPI_PRIVERROR : PINGAPI_INITERROR;
   }

    // init TTL
    if (    pTTL!=NULL
        &&  setsockopt (rawSocket, IPPROTO_IP, IP_TTL, (const char*)pTTL, sizeof *pTTL) == SOCKET_ERROR)
    {
        nRet = WSAGetLastError();
        closesocket (rawSocket) ;
        WSASetLastError (nRet);
        return nRet==WSAEACCES ? PINGAPI_PRIVERROR : PINGAPI_INITERROR;
    }

    // Create the packet
    // Setup destination socket address
    saDest.sin_addr = *pAddr;
    saDest.sin_family = AF_INET;
    saDest.sin_port = 0;
    // Fill in echo request
    echoReq.icmpHdr.Type        = ICMP_ECHO_REQUEST;
    echoReq.icmpHdr.Code        = 0;
    echoReq.icmpHdr.Checksum    = 0;
    echoReq.icmpHdr.ID          = PINGAPI_MYID;
    echoReq.icmpHdr.Seq         = nSeq++;
   // Fill in some data to send
   for (nRet = 0; nRet < REQ_DATASIZE; nRet++)     echoReq.cData[nRet] = ' '+nRet;
   // Save tick count when sent
   echoReq.dwTime              = GetTickCount();
   // Put data in packet and compute checksum
   echoReq.icmpHdr.Checksum = in_cksum((unsigned short *)&echoReq, sizeof(ECHOREQUEST));
   // Send the echo request
   nRet = sendto (rawSocket, (LPSTR) &echoReq,  sizeof echoReq, 0,
                 (LPSOCKADDR) & saDest, sizeof saDest);
    if ( nRet < sizeof(ECHOREQUEST) )
    {
        nRet = WSAGetLastError();
        closesocket (rawSocket) ;
        WSASetLastError (nRet);
        return nRet==WSAEACCES ? PINGAPI_PRIVERROR : PINGAPI_INITERROR;
    }


    dwStart = GetTickCount();
    while ( GetTickCount() < dwStart+dwTimeout_msec )
    {
    struct timeval          Timeout;
    fd_set                  readfds;
        dwCurrentTimeout = dwStart + dwTimeout_msec - GetTickCount() ;
     // Use select() to wait for data to be received
        readfds.fd_count = 1;
        readfds.fd_array[0] = rawSocket;
        Timeout.tv_sec  = dwCurrentTimeout / 1000;
        Timeout.tv_usec = (dwCurrentTimeout - (dwCurrentTimeout / 1000)*1000) * 1000  ;

        nRet = select(1, &readfds, NULL, NULL, &Timeout) ;
        if (nRet == 0)             { closesocket (rawSocket);  return PINGAPI_TIMEOUT; }
        if (nRet == SOCKET_ERROR)
		{  
			nRet = WSAGetLastError();
			closesocket (rawSocket) ;
			WSASetLastError (nRet);
			return PINGAPI_SOCKERROR;
		}
        // Receive reply
        // Receive the echo reply
        nRet = recvfrom(rawSocket,                  // socket
                  (LPSTR)&echoReply,  // buffer
                  sizeof(ECHOREPLY),  // size of buffer
                  0,                  // flags
                  (LPSOCKADDR)&saFrom,    // From address
                  &nAddrLen);         // pointer to address len

        // Check return value
        if (nRet == SOCKET_ERROR)
            { nRet = WSAGetLastError () ; closesocket (rawSocket);
              WSASetLastError (nRet) ;
              return PINGAPI_SOCKERROR; }
        if (nRet < sizeof (IPHDR) + sizeof (ICMPHDR))    continue ;     // ignore packet

        if (pIcmpReply->Type == ICMP_DEST_UNREACH)   { closesocket (rawSocket) ; return PINGAPI_UNREACHABLE; }
        if (pIcmpReply->Type == ICMP_TTL_EXPIRE)     { closesocket (rawSocket) ; return PINGAPI_TTLEXPIRE;   }
        if (pIcmpReply->Type==ICMP_ECHO_REPLY  &&  pIcmpReply->ID==PINGAPI_MYID)         break;
    }
    if ( GetTickCount() > dwStart+dwTimeout_msec)  { closesocket (rawSocket) ; return PINGAPI_TIMEOUT;  }

 // return time sent and IP TTL
    if (pTTL!=NULL)     *pTTL =  echoReply.ipHdr.TTL;
   // Calculate elapsed time
  dwElapsed = GetTickCount() - echoReply.echoRequest.dwTime;
 closesocket(rawSocket);
return dwElapsed==0 ? 1 : (signed) dwElapsed;
} // PingApi





//
// Mike Muuss' in_cksum() function
// and his comments from the original
// ping program
//
// * Author -
// *   Mike Muuss
// * U. S. Army Ballistic Research Laboratory
// *   December, 1983

/*
 *           I N _ C K S U M
 *
 * Checksum routine for Internet Protocol family headers (C Version)
 *
 */
unsigned short in_cksum(unsigned short *addr, int len)
{
    register int nleft = len;
    register unsigned short *w = addr;
    register unsigned short answer;
    register int sum = 0;

 /*
  *  Our algorithm is simple, using a 32 bit accumulator (sum),
  *  we add sequential 16 bit words to it, and at the end, fold
  *  back all the carry bits from the top 16 bits into the lower
     *  16 bits.
    */
    while( nleft > 1 )  {
      sum += *w++;
       nleft -= 2;
    }

 /* mop up an odd byte, if necessary */
 if( nleft == 1 ) {
     unsigned short  u = 0;

        *(unsigned char *)(&u) = *(unsigned char *)w ;
     sum += u;
  }

 /*
  * add back carry outs from top 16 bits to low 16 bits
  */
   sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
   sum += (sum >> 16);         /* add carry */
   answer = ~sum;              /* truncate to 16 bits */
  return (answer);
}





#ifdef TEST_IT

void main (int argc, char *argv[])
{
struct in_addr sAddr;
int           Ark;
char *szErr;
long dw;
WSADATA wsaData ;
int wVersionRequested = MAKEWORD( 1, 1 );
int ttl = 3;


   WSAStartup( wVersionRequested, &wsaData );

   if (argc < 2)   printf ("Usage: ping_api <host>\n");
   else
   {
       sAddr.s_addr = inet_addr (argv[1]);
       if (sAddr.s_addr == INADDR_ANY) printf ("Usage: <host> should be numerical\n");
       printf ("pinging %s\n", argv[1]);
       for (Ark=0 ; Ark<5 ; Ark++)
        {
            dw = PingApi (& sAddr, 2000, &ttl);
            if (dw > 0)  printf ("Success, time %d msec, TTL %d\n", dw, ttl);
            else
            {
                switch (dw)
               {
                    case PINGAPI_SOCKERROR :    szErr = "socket error" ;  break ;
                    case PINGAPI_INITERROR :    szErr = "init error" ;    break ;
                    case PINGAPI_PRIVERROR  :   szErr = "No Privilege";   break;
                    case PINGAPI_TIMEOUT :      szErr = "Timeout";        break;
                    case PINGAPI_UNKNOWNPKT :   szErr = "BUG: Unknown packet"; break;
                    case PINGAPI_UNREACHABLE :  szErr = "unreachable";    break;
                    case PINGAPI_TTLEXPIRE :    szErr = "TTL expire";     break;
                    default : szErr = "Bug";
               } /* switch dw */
               puts (szErr);
            } // erreur
        }   // boucle

   }
    WSACleanup ();
} // main

#endif // TEST_IT

