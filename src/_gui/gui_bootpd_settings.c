//////////////////////////////////////////////////////
//
// Projet TFTPD32.  January 2006 Ph.jounin
// Projet DHCPD32.  January 2006 Ph.jounin
// File bootp.c:    Manage BOOTP/DHCP protocols
//
// Released under European Union Public License
//
//////////////////////////////////////////////////////

#include "headers.h"
#include <stdio.h>          // sscanf is used


///////////////////////////////////////////
//  DHCP configuration management
///////////////////////////////////////////

// Utilitaire : translate a dialog item text into a in_addr struct
/*
struct in_addr DlgItem2Address (HWND hWnd, int nDlgItem, const char *szDescr, BOOL bStrict)
{
char             szBuf[120];
struct in_addr   dwAddr;
   memset (szBuf, 0, sizeof szBuf);
   GetDlgItemText (hWnd, nDlgItem,  szBuf, sizeof szBuf - 1);
   dwAddr.s_addr = inet_addr (szBuf);
   if (bStrict   &&  dwAddr.s_addr==INADDR_ANY)
   {
    wsprintf (szBuf, "Bad format for field %s", szDescr);
      MY_WARNING (szBuf);
   } // Erreur dans un champ
return dwAddr;
} // DlgItem2Address
*/

// translate text string into in_addr structures
int Text2MultipleInaddr (const char *sz, const char *sep, struct in_addr *pAddr, int *count)
{
char *p;
int Ark;
char s[64];
  lstrcpyn (s, sz, sizeof s - 1);
  s[sizeof s - 1] = 0;

  p=strtok (s, sep);
  for (Ark=0, p=s ;  p!=NULL &&  Ark<*count ; Ark++)
  {
     pAddr[Ark].S_un.S_addr = inet_addr (p);
	 p=strtok (NULL, sep);
  }
return Ark;
} // Text2MultipleInaddr

// Utilitaire : Check a string if a valid inet address (IPv4) 
// by Jesus Soto : probably he was not happy with inet_addr
int CheckInetAddress (HWND hWnd, char *szAddr, const char *szDescr, BOOL bStrict)
{
char   szBuf[120];
int    iErr=0;
int    i,j;
char  * pChar;
struct in_addr   dwAddr;

   memset (szBuf, 0, sizeof szBuf);
   if (strlen(szAddr)==0 && bStrict==FALSE) 
	   return(0);
   if (strlen(szAddr)==0 && bStrict==TRUE) 
   {
      wsprintf (szBuf, "Field %s cannot be empty", szDescr);
      MY_WARNING (szBuf);
	  return(1);
   }

// Check if the string has 3 and only 3 dots
   for (i=j=0,pChar=szAddr;i<6;i++)
   {
		if (pChar!=NULL)
		{
			pChar++;
			pChar=strstr(pChar,".");
			if (pChar!=NULL) j++;
		}
   }
   if (j!=3) iErr=1;
   if (iErr==0) dwAddr.s_addr = inet_addr (szAddr);
   if (bStrict && dwAddr.s_addr==INADDR_ANY) iErr=1;
   if (dwAddr.s_addr==(ULONG)0xFFFFFFFF) iErr=1;

   if (iErr)
   {
      wsprintf (szBuf, "Bad format for field %s", szDescr);
      MY_WARNING (szBuf);
   } // Erreur dans un champ
//      wsprintf (szBuf, "field %lu", dwAddr.s_addr);
//      MY_WARNING (szBuf);

return iErr;
} // CheckInetAddress


int Gui_LoadDHCPConfig (HWND hMainWnd)
{
char szBuf[256];
   // Display values: inet_ntoa and inet_addr help here
   SetDlgItemText (hMainWnd, IDC_DHCP_ADDRESS_POOL,  sGuiParamDHCP.szAddr);
   SetDlgItemText (hMainWnd, IDC_DHCP_MASK,          sGuiParamDHCP.szMask);
   if (sGuiParamDHCP.szDns2[0] !=0)
         wsprintf (szBuf, "%s, %s", sGuiParamDHCP.szDns1, sGuiParamDHCP.szDns2);
   else  wsprintf (szBuf, "%s", sGuiParamDHCP.szDns1);
   SetDlgItemText (hMainWnd, IDC_DHCP_DNS_SERVER,    szBuf);
   SetDlgItemText (hMainWnd, IDC_DHCP_WINS_SERVER,   sGuiParamDHCP.szWins);
   SetDlgItemText (hMainWnd, IDC_DHCP_DEFAULT_ROUTER,sGuiParamDHCP.szGateway);
   SetDlgItemText (hMainWnd, IDC_DHCP_OPTION42,      sGuiParamDHCP.szOpt42);
   SetDlgItemText (hMainWnd, IDC_DHCP_OPTION120,     sGuiParamDHCP.szOpt120);
   SetDlgItemText (hMainWnd, IDC_DHCP_BOOT_FILE,     sGuiParamDHCP.szBootFile);
   SetDlgItemInt  (hMainWnd, IDC_DHCP_POOL_SIZE,     sGuiParamDHCP.nPoolSize, FALSE); 
   SetDlgItemInt  (hMainWnd, IDC_DHCP_LEASE,         sGuiParamDHCP.nLease, FALSE);
   SetDlgItemText (hMainWnd, IDC_DHCP_DOMAINNAME,    sGuiParamDHCP.szDomainName);

   // first value in the GUI
   SetDlgItemInt  (hMainWnd, IDC_DHCP_ADDOPTION_NB,  sGuiParamDHCP.t[0].nAddOption, FALSE);
   SetDlgItemText (hMainWnd, IDC_DHCP_ADDOPTION_VALUE, sGuiParamDHCP.t[0].szAddOption);
return TRUE;   
} //  Gui_LoadDHCPConfig



// Save configuration either in INI file (if it exists) or in registry
int Gui_DHCPSaveConfig (HWND hWnd)
{
struct S_DHCP_Param  sNewParamDHCP;      // New param
// HWND hMainWnd = GetParent (hWnd);     // hWnd is no more a sub-wnd
HWND hMainWnd = hWnd;
INT   Ark;
int	  iErr=0;
char sz [256];	// DNS Server

     memset (& sNewParamDHCP, 0, sizeof sNewParamDHCP);
     // save parameters not assigned by GUI
     for (Ark=1 ; Ark < SizeOfTab (sGuiParamDHCP.t) ; Ark++)    sNewParamDHCP.t[Ark] = sGuiParamDHCP.t[Ark];  

	 sNewParamDHCP.nLease =    GetDlgItemInt (hMainWnd, IDC_DHCP_LEASE, NULL, FALSE);
	 sNewParamDHCP.nPoolSize = GetDlgItemInt (hMainWnd, IDC_DHCP_POOL_SIZE, NULL, FALSE);
//     sNewParamDHCP.dwAddr =    DlgItem2Address (hMainWnd, IDC_DHCP_ADDRESS_POOL, "DHCP Address Pool", sNewParamDHCP.nPoolSize!=0);
     GetDlgItemText (hMainWnd, IDC_DHCP_ADDRESS_POOL,  sNewParamDHCP.szAddr, sizeof sNewParamDHCP.szAddr - 1);
		 if (CheckInetAddress (hMainWnd, sNewParamDHCP.szAddr, "DHCP Pool start address", sNewParamDHCP.nPoolSize!=0)) iErr=1;
     GetDlgItemText (hMainWnd, IDC_DHCP_DEFAULT_ROUTER,  sNewParamDHCP.szGateway, sizeof sNewParamDHCP.szGateway - 1);
		 if (sNewParamDHCP.szGateway[0]!=0  &&  CheckInetAddress (hMainWnd, sNewParamDHCP.szGateway, "DHCP Default Router", TRUE)) iErr=1;
     GetDlgItemText (hMainWnd, IDC_DHCP_MASK,  sNewParamDHCP.szMask, sizeof sNewParamDHCP.szMask - 1);
		 if (CheckInetAddress (hMainWnd, sNewParamDHCP.szMask, "DHCP Mask", sNewParamDHCP.nPoolSize != 0)) iErr=1;
//   GetDlgItemText (hWnd, IDC_DHCP_MASK,  szBuf, sizeof szBuf - 1);
//		 if (CheckInetAddress (hMainWnd, szBuf, "DHCP Mask", TRUE)) iErr=1;
//		 sNewParamDHCP.dwMask.s_addr= inet_addr (szBuf);
     GetDlgItemText (hMainWnd, IDC_DHCP_WINS_SERVER,  sNewParamDHCP.szWins, sizeof sNewParamDHCP.szWins - 1);
		 if (CheckInetAddress (hMainWnd, sNewParamDHCP.szWins, "DHCP WINS Server", FALSE)) iErr=1;
     GetDlgItemText (hMainWnd, IDC_DHCP_OPTION42,  sNewParamDHCP.szOpt42, sizeof sNewParamDHCP.szOpt42 - 1);
		 if (CheckInetAddress (hMainWnd, sNewParamDHCP.szOpt42, "DHCP NTP Server", FALSE)) iErr=1;
     GetDlgItemText (hMainWnd, IDC_DHCP_OPTION120,  sNewParamDHCP.szOpt120, sizeof sNewParamDHCP.szOpt120 - 1);
		 if (CheckInetAddress (hMainWnd, sNewParamDHCP.szOpt120, "DHCP SIP Server", FALSE)) iErr=1;
     GetDlgItemText (hMainWnd, IDC_DHCP_BOOT_FILE,   sNewParamDHCP.szBootFile, sizeof sNewParamDHCP.szBootFile - 1);
     GetDlgItemText (hMainWnd, IDC_DHCP_DOMAINNAME,  sNewParamDHCP.szDomainName, sizeof sNewParamDHCP.szDomainName - 1);

	 // split the DNS server field if a ,; or space is found
     GetDlgItemText (hMainWnd, IDC_DHCP_DNS_SERVER, sz, sizeof sz - 1);
	 sNewParamDHCP.szDns1[0] = sNewParamDHCP.szDns2[0] = 0;
	 if (    sz[0]!=0  
		 &&  sscanf (sz, "%31[0-9.]%*[,; -]%31[0-9.]", sNewParamDHCP.szDns1, sNewParamDHCP.szDns2) < 1 )
	 { 
        MY_WARNING ("Bad DNS format : use <1.2.3.4> or <1.2.3.4 - 4.5.6.7>");
		iErr=1;
	 }

     if ( sNewParamDHCP.nLease < 3 )
	 {
        MY_WARNING ("Lease time must be at least 3 minutes");
		iErr=1;
	 }
	 if (iErr==1) return FALSE;

     // from the GUI only one user option is available
     sNewParamDHCP.t[0].nAddOption = GetDlgItemInt (hMainWnd, IDC_DHCP_ADDOPTION_NB, NULL, FALSE);
     GetDlgItemText (hMainWnd,
                     IDC_DHCP_ADDOPTION_VALUE,
                     sNewParamDHCP.t[0].szAddOption,
                     sizeof sNewParamDHCP.t[0].szAddOption - 1);

     // Change 30 oct 2002 : Gateway can be empty 
//     if ( strlen(sNewParamDHCP.szAddr)==0 || strlen(sNewParamDHCP.szMask)==0)
//		return FALSE;

	 // change: 2019 : do not warn if address is empty
	 if (sNewParamDHCP.szAddr[0] != 0)
	 {
		 // load again (warkaround for a LCC bug)
		 sNewParamDHCP.nPoolSize = GetDlgItemInt(hMainWnd, IDC_DHCP_POOL_SIZE, NULL, FALSE);
		 if (sNewParamDHCP.nPoolSize == 0)   MY_WARNING("DHCP Pool is empty.\nDHCP server will only assign\nstatic leases");

		 // PJO 30/04/2018 : warning if no gateway
		 if (sNewParamDHCP.szGateway[0] == 0)  MY_WARNING("Gateway is empty.\nNo default route will be passed by DHCP server");
	 }

//	 if (memcmp (& sGuiParamDHCP, & sNewParamDHCP, sizeof sNewParamDHCP) !=0)
//	 {
		sGuiParamDHCP = sNewParamDHCP;
		return TRUE;
//	 }
return FALSE;
} // Gui_DHCPSaveConfig




