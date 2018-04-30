/*
 * From Tcp4u v 3.31         Last Revision 08/12/1997  3.31-01
 *
 *===========================================================================
 *
 * Project: Tcp4u,      Library for tcp protocol
 * File:    tcp4.c
 *
 *===========================================================================
* Source released under under European Union Public License
 */


//#include <windows.h>
#include <ws2tcpip.h>
#include <time.h>

#include "tcp4u.h"
#include "..\log\logtomonitor.h"



// free resources for TcpGetListenSocket
static int BadEnd (SOCKET s, LPADDRINFO res)
{
int Rc;
	  Rc = GetLastError ();
      if (s!=INVALID_SOCKET) closesocket (s);
	  if (res!=NULL)         freeaddrinfo (res);
	  SetLastError (Rc);
      return  INVALID_SOCKET;
} // BadEnd


SOCKET TcpGetListenSocket (int family, LPCSTR szService, unsigned short * pPort)
{
SOCKET               ListenSock = INVALID_SOCKET;
int                  Rc;
ADDRINFO             Hints, *res=NULL;
char                 szServ [NI_MAXSERV];

   memset (&Hints, 0, sizeof Hints);
   Hints.ai_family   = family;
   Hints.ai_socktype = SOCK_STREAM;
   Hints.ai_protocol = IPPROTO_TCP;
   Hints.ai_flags    = AI_PASSIVE;
   
   Rc = getaddrinfo (NULL, szService, & Hints, & res);
   if (Rc==WSASERVICE_NOT_FOUND  ||  Rc==WSATYPE_NOT_FOUND)
   {
	   Hints.ai_flags |= AI_NUMERICSERV;
	   wsprintf (szServ, "%d", pPort==NULL ? 0 : *pPort);
	   Rc = getaddrinfo (NULL, szServ, & Hints, & res);
   }
   if (Rc !=0)                       return BadEnd (ListenSock, res);

  /* create socket */
   ListenSock = socket (res->ai_family, res->ai_socktype, res->ai_protocol);
  if (ListenSock == INVALID_SOCKET)  return BadEnd (ListenSock, res);

  /* Bind name to socket */
  Rc =   bind (ListenSock,(struct sockaddr *) res->ai_addr, res->ai_addrlen);
  if (Rc==SOCKET_ERROR)              return BadEnd (ListenSock, res);
  Rc = listen (ListenSock, 1);
  if (Rc==SOCKET_ERROR)              return BadEnd (ListenSock, res);
  if (Rc==0  && pPort!=NULL)
  {
	  switch (res->ai_family)
	  {
	      case AF_INET  :  *pPort = htons ( (* (struct sockaddr_in *) res->ai_addr).sin_port);
			                break;
	      case AF_INET6 :  *pPort = htons ( (* (struct sockaddr_in6 *) res->ai_addr).sin6_port);
			                break;
	  }
  }
  freeaddrinfo (res);
return ListenSock;
}  /* TcpGetListenSock */



// --------------------
// Recv
// --------------------

int TcpRecv (SOCKET s, LPSTR szBuf, unsigned uBufSize, 
                     unsigned uTimeOut, HANDLE hLogFile)
{
int             Rc, nUpRc;  /* Return Code of select and recv */
struct timeval  TO;         /* Time Out structure             */
struct timeval *pTO;        /* Time Out structure             */
fd_set          ReadMask;   /* select mask                    */
DWORD           dummy;

  if (s==INVALID_SOCKET)  return TCP4U_ERROR;

  FD_ZERO (& ReadMask);     /* mise a zero du masque */
  FD_SET (s, & ReadMask);   /* Attente d'evenement en lecture */

  /* detail des modes */
  switch (uTimeOut)
  {
      case  TCP4U_WAITFOREVER : pTO = NULL; 
                                break;
      case  TCP4U_DONTWAIT    : TO.tv_sec = TO.tv_usec=0 ; 
                                pTO = & TO;
                                break;
      /* Otherwise  uTimeout is really the Timeout */
      default :                 TO.tv_sec = (long) uTimeOut;
                                TO.tv_usec=0;
                                pTO = & TO;
                                break;
  }
  /* s+1 normally unused but better for a lot of bugged TCP Stacks */
  Rc = select (s+1, & ReadMask, NULL, NULL, pTO);
  if (Rc<0) 
  {
	 LogToMonitor ("select returns error %d\n", WSAGetLastError());
     return  TCP4U_ERROR;
  }
  if (Rc==0)
     return  TCP4U_TIMEOUT;  /* timeout en reception           */

  if (szBuf==NULL  ||  uBufSize==0)  
	return TCP4U_SUCCESS;
  
  Rc = recv (s, szBuf, uBufSize, 0);  /* chgt 11/01/95 */
 
  switch (Rc)
  {
       case SOCKET_ERROR : 
		 	  LogToMonitor ("recv returns error %d\n", WSAGetLastError());
              nUpRc = TCP4U_ERROR ; 
              break;
       case 0            : 
              nUpRc = TCP4U_SOCKETCLOSED ; 
              break;
       default :
              if (hLogFile!=INVALID_HANDLE_VALUE)    WriteFile (hLogFile, szBuf, Rc, &dummy, NULL);
              nUpRc = Rc;
              break;
  } /* translation des codes d'erreurs */
return nUpRc;
} /* TcpRecv */


int TcpSend (SOCKET s, LPCSTR szBuf, unsigned uBufSize, HANDLE hLogFile)
{
int      Rc;
unsigned Total;
DWORD    dummy;

  if (s==INVALID_SOCKET)  return TCP4U_ERROR;
  if (hLogFile!=INVALID_HANDLE_VALUE)    WriteFile (hLogFile, szBuf, uBufSize, &dummy, NULL);

  for ( Total = 0, Rc = 1 ;  Total < uBufSize  &&  Rc > 0 ;  Total += Rc)
  {
      Rc = send (s, & szBuf[Total], uBufSize-Total, 0);
  }
  
return Total>=uBufSize ? TCP4U_SUCCESS :  TCP4U_ERROR;
} /* TcpSend */


int TcpPPSend (SOCKET s, LPCSTR szBuf, unsigned uBufSize, HANDLE hLogFile)
{
int      Rc;
unsigned Total;
DWORD    dummy;
unsigned short usSize = htons (uBufSize);

  if (s==INVALID_SOCKET)  return TCP4U_ERROR;
  if (uBufSize > 0x7FFF)  return TCP4U_OVERFLOW;
  if (hLogFile!=INVALID_HANDLE_VALUE)    WriteFile (hLogFile, szBuf, uBufSize, &dummy, NULL);

  // send msg length
  Rc = send (s, (char *) & usSize, sizeof usSize, 0);
  for ( Total = 0 ;  Total < uBufSize  &&  Rc > 0 ;  Total += Rc)
  {
      Rc = send (s, & szBuf[Total], uBufSize-Total, 0);
  }
  if (Rc<0) 
		LogToMonitor ("send returns error %d\n", WSAGetLastError());
 
return Total>=uBufSize ? TCP4U_SUCCESS :  TCP4U_ERROR;
} /* TcpPPSend */



int TcpPPRecv (SOCKET s, LPSTR szBuf, unsigned uBufSize, int uTimeOut, HANDLE hLogFile)
{
unsigned short usToBeReceived=0; 
int            usUpRc, usReceived;
int            Rc;

  usReceived = 0;
  usUpRc=1;
  // get number of byte to be expected
  Rc = TcpRecv (s, (LPSTR) & usToBeReceived, sizeof usToBeReceived, uTimeOut, hLogFile);
  if (Rc!=sizeof usToBeReceived)  return Rc;
  // put in machine ordrer
  usToBeReceived = ntohs (usToBeReceived);
  
  if (usToBeReceived > 0x7FFF)  return TCP4U_OVERFLOW;
  
  // loop while msg is shorter than expected or timeout
  while (usUpRc>0  &&  usReceived < usToBeReceived  )
  {
      usUpRc = TcpRecv (s, szBuf, usToBeReceived - usReceived, uTimeOut, hLogFile);
      usReceived += usUpRc;
      szBuf += usUpRc;
  }

return usUpRc>0 ? usReceived : usUpRc ;
} /* TcpPPRecv */



SOCKET TcpConnect ( LPCSTR  szHost,
                    LPCSTR  szService, 
					int     family,
                    unsigned short nPort)
{
SOCKET               connect_skt = INVALID_SOCKET ;
int                  Rc;
ADDRINFO             Hints, *res=NULL;
char                 szServ [NI_MAXSERV];

   memset (&Hints, 0, sizeof Hints);
   Hints.ai_family = family;
   Hints.ai_socktype = SOCK_STREAM;
   Hints.ai_protocol = IPPROTO_TCP;

   Rc = getaddrinfo (szHost, szService, & Hints, & res);
   if (Rc==WSASERVICE_NOT_FOUND  ||  Rc==WSATYPE_NOT_FOUND)
   {
	   Hints.ai_flags |= AI_NUMERICSERV;
	   wsprintf (szServ, "%d", nPort);
	   Rc = getaddrinfo (szHost, szServ, & Hints, & res);
   }
   if (Rc !=0)                       return BadEnd (connect_skt, res);
   
  /* create socket */
   connect_skt = socket (res->ai_family, res->ai_socktype, res->ai_protocol);
  if (connect_skt == INVALID_SOCKET)   return BadEnd (connect_skt, res);


  /* --- connect retourne INVALID_SOCKET ou numero valide */
  Rc = connect (connect_skt,(struct sockaddr *) res->ai_addr, res->ai_addrlen);
  if (Rc!=0)                         return BadEnd (connect_skt, res);
  freeaddrinfo (res);
  /* --- enregistrement dans notre table */
return connect_skt ;
}  /* TcpConnect */

