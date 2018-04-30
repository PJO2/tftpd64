/*
 * From Tcp4u v 3.31         Last Revision 08/12/1997  3.31-01
 *
 *===========================================================================
 *
 * Project: Tcp4u,      Library for tcp protocol
 * File:    tcp4.c
 *
 *===========================================================================
 * Source released under GPL license
 */

//#include <windows.h>
// #include <winsock2.h>
#include <ws2tcpip.h>
#include <time.h>

#include "tcp4u.h"
#include "../log/LogToMonitor.h"


// send data using Udp
int UdpSend (int nFromPort, struct sockaddr *sa_to, int sa_len, const char *data, int len)
{
SOCKET              s;
SOCKADDR_STORAGE    sa_from;
int                 Rc;
int                 True=1;
char                szServ[NI_MAXSERV];
ADDRINFO            Hints, *res;

   // populate sa_from
   memset (&sa_from, 0, sizeof sa_from);
	   Hints.ai_family = sa_to->sa_family;
	   Hints.ai_flags = NI_NUMERICSERV;
	   Hints.ai_socktype = SOCK_DGRAM;
	   Hints.ai_protocol = IPPROTO_UDP;
       wsprintf (szServ, "%d", nFromPort);
       Rc = getaddrinfo (NULL, szServ, & Hints, & res);

	   s = socket (res->ai_family, res->ai_socktype, res->ai_protocol);
      if (s == INVALID_SOCKET)  return TCP4U_ERROR;
     // REUSEADDR option in order to allow thread to open 69 port
      Rc = setsockopt (s, SOL_SOCKET, SO_REUSEADDR, (char *) & True, sizeof True);
      LogToMonitor (Rc==0 ? "UdpSend: Port %d may be reused" : "setsockopt error", nFromPort);

	  Rc = bind (s, res->ai_addr, res->ai_addrlen);
	  freeaddrinfo(res);

      LogToMonitor ("UdpSend bind returns %d (error %d)", Rc, GetLastError ());
      if (Rc<0) { closesocket (s); return TCP4U_BINDERROR; }

      Rc = sendto (s, data, len, 0, sa_to, sa_len);
      LogToMonitor ("sendto returns %d", Rc);
      closesocket (s);
return Rc;
} // UdpSend

