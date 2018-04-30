//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Mai 98 Ph.jounin
// File qui_syslogd.c:  Syslog
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



void AddSyslogItem (HWND hListV, const char *szIP, const char *szTxt)
{
LVITEM LvItem;
int    itemPos;
char   szDate [32];
SYSTEMTIME sTime;

   // date of entry
   GetLocalTime (&sTime);
   wsprintf (szDate, "%02d/%02d %02d:%02d:%02d.%03d", 
             sTime.wDay, sTime.wMonth,
             sTime.wHour, sTime.wMinute, sTime.wSecond, sTime.wMilliseconds );

   // keep only MAX_MSG messages
   while (ListView_GetItemCount (hListV) >= MAX_MSG) 
		 ListView_DeleteItem (hListV, 0) ;

  LvItem.mask = LVIF_PARAM | LVIF_STATE;
  // LvItem.mask = 0;
  LvItem.state = 0;
  LvItem.stateMask = 0;
  LvItem.iItem = ListView_GetItemCount (hListV);    // item pos
  LvItem.lParam = 0;   // sorting params   
  LvItem.iSubItem = 0; // column index
  // LvItem.pszText = "";

  itemPos = ListView_InsertItem(hListV,(LPARAM)&LvItem);
  ListView_SetItemText (hListV, itemPos, 0, (char *) szTxt);
  ListView_SetItemText (hListV, itemPos, 1, (char *) szIP);
  ListView_SetItemText (hListV, itemPos, 2, szDate);

  // if no row is selected, scroll to the last item
  if (ListView_GetSelectedCount (hListV) == 0)
     ListView_EnsureVisible (hListV, ListView_GetItemCount (hListV) - 1, TRUE);
} // AddSyslogItem


