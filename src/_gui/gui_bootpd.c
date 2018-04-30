//////////////////////////////////////////////////////
//
// Projet TFTPD32.  January 2006 Ph.jounin
// Projet DHCPD32.  January 2006 Ph.jounin
// File bootp.c:    Manage BOOTP/DHCP protocols
//
// Released under European Union Public License
//
//////////////////////////////////////////////////////

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
#  pragma message ("                  Tftpd32 compilation")
#  include "headers.h"
#endif

#include <stdio.h>          // sscanf is used




extern HWND hDbgMainWnd  ;

// ------------------------------------------------------------------------------------
// GUI
// ------------------------------------------------------------------------------------

//////////////////////////////////////////////////
// manage drop down menu
int DhcpServerListPopup (HWND hListV)
{
POINT pt;
HMENU hMenu;
int iSelected;

    // retrieve selected item
    iSelected = ListView_GetNextItem (hListV, -1, LVNI_FOCUSED);
    if (iSelected == -1) return TRUE;

    hMenu = LoadMenu(GetWindowInstance (hListV), MAKEINTRESOURCE (IDM_DHCP_LIST));
    GetCursorPos (&pt);
    TrackPopupMenu (GetSubMenu (hMenu, 0), TPM_LEFTALIGN | TPM_LEFTBUTTON,
                    pt.x, pt.y, 0, GetParent (hListV), NULL);
    DestroyMenu (hMenu);
return TRUE;
} // DhcpServerListPopup




// -------------------------------------
// ListView management
// --------------------------------------
void DhcpRefresh_ListView ( int nbLeases, struct S_Lease tLeases[] )
{
struct tm ltime;
int Ark;
char szDate[64];
LVITEM LvItem;
int itemPos;
HWND hListV;
HWND hWnd = hDbgMainWnd;

    memset (&LvItem, 0, sizeof LvItem);

   while (GetParent (hWnd)!=NULL)  hWnd=GetParent(hWnd);
   hListV = GetDlgItem (hWnd, IDC_LV_DHCP);
   
   ListView_DeleteAllItems (hListV);
   for (Ark = 0 ;   Ark < nbLeases  ;  Ark++ )
   {
  	  LvItem.mask = LVIF_PARAM | LVIF_STATE;
      LvItem.state = 0;
      LvItem.stateMask = 0;
      LvItem.iItem = Ark;      // numéro de l'item
      LvItem.iSubItem = 0;     // index dans la ligne
      LvItem.lParam = inet_addr (tLeases[Ark].szIP);  // keep track of assignation
	  itemPos = ListView_InsertItem(hListV,(LPARAM)&LvItem);

#ifdef _MSC_VER
            localtime_s (&ltime, & tLeases[Ark].tAllocated);
#else
            memcpy (& ltime, localtime (& tLeases[Ark].tAllocated), sizeof ltime);
#endif
	wsprintf (szDate, "%02d/%02d %02d:%02d:%02d", 
					  ltime.tm_mon+1, ltime.tm_mday,ltime.tm_hour, ltime.tm_min, ltime.tm_sec);	
	ListView_SetItemText (hListV, itemPos, 0, szDate);
	ListView_SetItemText (hListV, itemPos, 1, tLeases[Ark].szIP);
	ListView_SetItemText (hListV, itemPos, 2, tLeases[Ark].szMAC);
	
	if (tLeases[Ark].tRenewed != 0)
	{
#ifdef _MSC_VER
            localtime_s (&ltime, & tLeases[Ark].tRenewed);
#else
            memcpy (& ltime, localtime (& tLeases[Ark].tRenewed), sizeof ltime);
#endif
	     wsprintf (szDate, "%02d/%02d %02d:%02d:%02d", 
					  ltime.tm_mon+1, ltime.tm_mday,ltime.tm_hour, ltime.tm_min, ltime.tm_sec);	
	     ListView_SetItemText (hListV, itemPos, 3, szDate);
	 }	// renew ! =0	   
   } // parse all alocations
} // DhcpRefresh_ListView 




