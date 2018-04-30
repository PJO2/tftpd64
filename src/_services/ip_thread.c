//////////////////////////////////////////////////////
//
// Projet TFTPD32.  April 2007 Ph.jounin
//                  A free TFTP server for Windows
// File ip_thread.c: periodically checks server interfaces staus
//
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////

#include "headers.h"
#include "threading.h"

static struct S_IPAddressList old_if;

// called by console thread
int AnswerIPList (void)
{
   // change old_if structure ie : force a refresh
   memset (& old_if, 0, sizeof old_if);
   // wake up scheduler thread 
   WakeUpThread (TH_SCHEDULER);
return 0;
} // 

// -----------------------
// Get all interfaces. This function is restricted to IPv4 in order to run under XP
// -----------------------
int PoolNetworkInterfaces (void)
{
ULONG outBufLen;
IP_ADAPTER_ADDRESSES       *pAddresses=NULL, *pCurrAddresses;
IP_ADAPTER_UNICAST_ADDRESS *pUnicast;
int Rc;
struct S_IPAddressList        new_if;

    memset (& new_if, 0, sizeof new_if);

    outBufLen = sizeof (IP_ADAPTER_ADDRESSES);
    pAddresses = (IP_ADAPTER_ADDRESSES *) malloc (outBufLen);

    // Make an initial call to GetAdaptersAddresses to get the 
    // size needed into the outBufLen variable
    if (GetAdaptersAddresses (sSettings.bIPv6 ? AF_UNSPEC : AF_INET, 
							  GAA_FLAG_INCLUDE_PREFIX, 
							  NULL, 
							  pAddresses, 
							 & outBufLen) == ERROR_BUFFER_OVERFLOW) 
	 {
           free(pAddresses);
           pAddresses = (IP_ADAPTER_ADDRESSES *) malloc (outBufLen);
     }

    if (pAddresses == NULL) {
        return FALSE;
    }
    // Make a second call to GetAdapters Addresses to get the
    // actual data we want
	Rc = GetAdaptersAddresses (sSettings.bIPv6 ? AF_UNSPEC : AF_INET, 
							   GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_SKIP_MULTICAST, 
							   NULL, 
							   pAddresses, 
							 & outBufLen);
    if (Rc == NO_ERROR) 
	{
		for ( pCurrAddresses = pAddresses,            new_if.nb_itf = 0,      new_if.nb_addr = 0 ; 
			  pCurrAddresses                      &&  new_if.nb_itf < MAX_ITF ; 
			  pCurrAddresses=pCurrAddresses->Next,    new_if.nb_itf++ )
		{
			// can not use lstrcpy since Description is unicode
			// may give strange results in Japan/China/..
			wsprintf (new_if.itf[new_if.nb_itf].sz, "%ls", pCurrAddresses->Description);
			new_if.itf[new_if.nb_itf].status = pCurrAddresses->OperStatus;

			for ( pUnicast = pCurrAddresses->FirstUnicastAddress; 
				  pUnicast != NULL   &&   new_if.nb_addr<MAX_IP_ADDR ; 
				  pUnicast = pUnicast->Next )
			{
				// convert into string (do not use InetNtoP which is not supported under XP)
				Rc = getnameinfo (pUnicast->Address.lpSockaddr, 
						 	      pUnicast->Length, 
							      new_if.addr[new_if.nb_addr].sz, MAXLEN_IPv6, 
							      NULL, 0,  NI_NUMERICHOST);
				if (Rc==0) // success : link address with name od interface & increment addr number
					new_if.addr[new_if.nb_addr++].idx = new_if.nb_itf;
			} // for each address
		}  // for each interface
	} // GetAdaptersAddresses successfull
	else 
	{
	   free (pAddresses);
	   Rc = WSAGetLastError ();
	   return FALSE;
	}

   // signal a change on request or if interface list has changed
   if (memcmp (&new_if, &old_if, sizeof old_if)!=0)
   {
	   old_if = new_if ;
	   SendMsgRequest (  C_REPLY_GET_INTERFACES, 
						& old_if, 
						  sizeof old_if,
						  FALSE,
						  FALSE );
   }
   free (pAddresses);
return TRUE;
} // PoolNetworkInterfaces


// -----------------------
// Retrieve IPv4 assigned to an interface
// -----------------------
int GetIPv4Address (const char *szIf, char *szIP)
{
ULONG outBufLen;
IP_ADAPTER_ADDRESSES       *pAddresses=NULL, *pCurrAddresses;
IP_ADAPTER_UNICAST_ADDRESS *pUnicast;
int Rc;
char szBuf [MAX_ADAPTER_DESCRIPTION_LENGTH+4];

	szIP[0]=0;
    outBufLen = sizeof (IP_ADAPTER_ADDRESSES);
    pAddresses = (IP_ADAPTER_ADDRESSES *) malloc (outBufLen);

    // Make an initial call to GetAdaptersAddresses to get the 
    // size needed into the outBufLen variable
    if (GetAdaptersAddresses (AF_INET, 
							  GAA_FLAG_INCLUDE_PREFIX, 
							  NULL, 
							  pAddresses, 
							 & outBufLen) == ERROR_BUFFER_OVERFLOW) 
	 {
           free(pAddresses);
           pAddresses = (IP_ADAPTER_ADDRESSES *) malloc (outBufLen);
     }

    if (pAddresses == NULL) {
        return -1;
    }
    // Make a second call to GetAdapters Addresses to get the
    // actual data we want
	Rc = GetAdaptersAddresses (sSettings.bIPv6 ? AF_UNSPEC : AF_INET, 
							   GAA_FLAG_INCLUDE_PREFIX, 
							   NULL, 
							   pAddresses, 
							 & outBufLen);
    if (Rc == NO_ERROR) 
	{
		for ( pCurrAddresses = pAddresses ; pCurrAddresses ; pCurrAddresses=pCurrAddresses->Next )
		{
			wsprintf (szBuf, "%ls", pCurrAddresses->Description);
			if ( 
				   lstrcmp (szBuf, sSettings.szTftpLocalIP) == 0 
				&& pCurrAddresses->FirstUnicastAddress !=0
				)
			{SOCKADDR *sAddr = pCurrAddresses->FirstUnicastAddress->Address.lpSockaddr;
				lstrcpy ( szIP,
						  inet_ntoa (  ((struct sockaddr_in *) sAddr)->sin_addr ) );
				free (pAddresses);
				return 0;	  
			}
		}
	}
	free (pAddresses);
return -1;
} // GetIPv4Address
				


