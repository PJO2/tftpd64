/*
 * From Tcp4u v 3.31         Last Revision 08/12/1997  3.31-01
 *
 *===========================================================================
 *
 * Project: Tcp4u,      Library for tcp protocol
 * File:    tcp4u_exchg.c : a poor man session establishing protocol
 *
 *===========================================================================
 *
 *  TcpExchangeChallenge is called after a successfull connect or accept.
 *  it exchanges a protocol version and a key to ensures that both client
 *  and server will work together
 *
 * Source released under GPL license
 */


#include <winsock2.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include "tcp4u.h"
#include "..\log\logtomonitor.h"

#ifndef INVALID_FILE_HANDLE
#   define INVALID_FILE_HANDLE  (HANDLE) -1
#endif

struct S_Challenge
{
   int  version;
   unsigned char  challenge [12];
   char pad;
};


// a basic but symetric crypto function
// a basic but symetric crypto function
static char *sym_crypt (char *sz, int nLen, const char *key)
{
int Ark;
int nKeyLen = lstrlen (key);
   assert (nKeyLen != 0);
   for ( Ark=0 ;  Ark<nLen ;  Ark++ )
     sz[Ark] ^= key [Ark * 13 % nKeyLen];
return sz;	 
} // crypt

// ok : there was no need to use 2 exchanges.
// however it can not be changed to support legacy 
int TcpExchangeChallenge (SOCKET s, int seed, int nVersion, int *peerVersion, const char *key)
{
FILETIME ft;
struct S_Challenge Out1, In1, Out2, In2;
int Rc;
int Ark;

	GetSystemTimeAsFileTime (&ft);
    srand (ft.dwLowDateTime + seed + (unsigned) s);
	
    for (Ark=0 ; Ark< sizeof Out1.challenge ; Ark++)
        Out1.challenge[Ark] = (unsigned char) (rand () & 0xFF);
	Out1.pad = 0;

	Out1.version = nVersion;
	Rc = TcpPPSend ( s, (char *) & Out1, sizeof Out1, INVALID_FILE_HANDLE );
	if (Rc<0) return Rc;			
	Rc = TcpPPRecv ( s, (char *) & In1, sizeof In1, 10, INVALID_FILE_HANDLE );
	if (Rc<0) 
	{int Ark = WSAGetLastError ();
	    LogToMonitor ("Error %d receiving on socket %d\n", Ark, s);
		return Rc;
	}

	if (peerVersion != NULL) *peerVersion = In1.version;
	/* verify version */
	if (In1.version != nVersion) return TCP4U_VERSION_ERROR;

	/* 'crypt' challenge */
	Out2 = In1;
	sym_crypt ( Out2.challenge, sizeof Out2.challenge, key );
	/* resend frame */
	Rc = TcpPPSend ( s, (char *)& Out2, sizeof Out2, INVALID_FILE_HANDLE );
	if (Rc<0) return Rc;
		
	/* wait for our Challenge (*/
	Rc = TcpPPRecv ( s, (char *)& In2, sizeof In2, 10, INVALID_FILE_HANDLE );
	if (Rc<0) return Rc;
	/* un crypt In2 and compare with Out1 */
	sym_crypt ( In2.challenge, sizeof In2.challenge, key );
	// if 
    if (! memcmp (In2.challenge, Out1.challenge, sizeof Out1.challenge)==0 )
		return TCP4U_BAD_AUTHENT ;

return TCP4U_SUCCESS;
} // TcpExchangeChallenge
	
