//////////////////////////////////////////////////////
//
// Projet TFTPD32.  2009 Ph.jounin
// File gui_dns.c:  DNS window
//
//////////////////////////////////////////////////////

#include "headers.h"
#include <commctrl.h>


// number of messages kept
#define MAX_MSG 500




// ---------------------------------------------------------------------
//
// ListView Management 
//
// ---------------------------------------------------------------------



void AddDNSItem (HWND hListV, char *szName, char *szIPv4, char *szIPv6)
{
LVITEM LvItem;
LV_FINDINFO LvInfo;
int    itemPos;

   // is host already in the list
  LvInfo.flags = LVFI_STRING;
  LvInfo.psz = szName;
  itemPos = ListView_FindItem(hListV, -1, &LvInfo);
  if (itemPos != -1) 
  {
	  // already displayed : refresh address
		memset (& LvItem, 0, sizeof LvItem);
		LvItem.iItem = itemPos;
  	    ListView_SetItemText (hListV, itemPos, 1, szIPv4);
	    ListView_SetItemText (hListV, itemPos, 2, szIPv6);
  }
  else
  {
	// a new item should be inserted 	     
	// before that keep only MAX_MSG messages
	   while (ListView_GetItemCount (hListV) >= MAX_MSG) 
			 ListView_DeleteItem (hListV, 0) ;

	  LvItem.mask = LVIF_PARAM | LVIF_STATE;
	  // LvItem.mask = 0;
	  LvItem.state = 0;
	  LvItem.stateMask = 0;
	  LvItem.iItem = ListView_GetItemCount (hListV);    // item pos
	  LvItem.lParam = 0 ;   // sorting params   
	  LvItem.iSubItem = 0; // column index
	  // LvItem.pszText = "";

	  itemPos = ListView_InsertItem(hListV,(LPARAM)&LvItem);
	  ListView_SetItemText (hListV, itemPos, 0, szName);
	  ListView_SetItemText (hListV, itemPos, 1, szIPv4);
	  ListView_SetItemText (hListV, itemPos, 2, szIPv6);
  }  // host not found
} // AddDNSItem


