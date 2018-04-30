//////////////////////////////////////////////////////
//
// Projet TFTPD32.  January 2006 Ph.jounin
// Projet DHCPD32.  January 2006 Ph.jounin
// File bootpd_console.c:    DHCP Reporting
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////

#ifndef TFTPD32
#  pragma message ("                  Bootp_file : Dhcpd32 compilation")
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
#  pragma message ("                  Bootp_file : Tftpd32 compilation")
#  include "headers.h"
#endif

#include <stdio.h>          // sscanf is used
#include <process.h>        // endthread + beginthread

#include "threading.h"
#include "bootpd_functions.h"



void Dhcp_Send_Leases (const struct LL_IP *tIP[], int nbLeases)
{
struct S_DhcpLeases dhcp;
int Ark;

    for ( Ark=0, dhcp.nb=0 ; 
          Ark<nbLeases && Ark<SizeOfTab(dhcp.l) && Ark <  sParamDHCP.nPoolSize ; 
          Ark++)
    {
		if (memcmp (tIP[Ark]->sMacAddr, FREE_DHCP_ADDRESS, 6) == 0)
				strcpy (dhcp.l[dhcp.nb].szMAC, "-");
		else
				strcpy ( dhcp.l[dhcp.nb].szMAC, haddrtoa(tIP[Ark]->sMacAddr, 6,':') );
		dhcp.l[dhcp.nb].tAllocated = tIP[Ark]->tAllocated;
		dhcp.l[dhcp.nb].tRenewed   = tIP[Ark]->tRenewed;
		strcpy ( dhcp.l[dhcp.nb].szIP, inet_ntoa (tIP[Ark]->dwIP) );
		dhcp.nb++;
    }
    SendMsgRequest (   C_DHCP_LEASE, 
					 & dhcp, 
					   dhcp.nb * (sizeof dhcp.l[0]) + sizeof dhcp.nb, 
					   TRUE,		// block thread until msg sent
					   FALSE );		// if no GUI return
} // Dhcp_Send_NewLease


