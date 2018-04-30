//////////////////////////////////////////////////////
//
// Projet TFTPD32.   Feb 99 By  Ph.jounin
// File qui_tftpd.c:  TFTP sub-window management
//
// released under artistic license
//
//////////////////////////////////////////////////////


#include "headers.h"


enum E_fields { FD_PEER, FD_FILE, FD_START, FD_PROGRESS, FD_BYTES, FD_TOTAL, FD_TIMEOUT };

// a transfer terminated but still displayed
#define ZOMBIE_STATUS '.'

///////////////////////////////
// verify that all transfer are terminated
// by listing list view
///////////////////////////////
BOOL GuiIsActiveTFTPTransfer ( HWND hMainWnd )
{
long lLen = 128;
LVITEM lvItem;
LRESULT lResult = 0;
int     Ark;
HWND    hLV = GetDlgItem (hMainWnd, IDC_LV_TFTP);
char    szName [512];
  
   // scan the TFTP list view, if an item does not begin with '.' 
   // a transfer is in progress
   for ( Ark = ListView_GetItemCount (hLV) - 1 ;  Ark>=0 ;  Ark-- )
   {
	  // retrieve the field file
      memset(&lvItem, 0, sizeof(LVITEM));
	  ListView_GetItemText (hLV, Ark, FD_FILE, szName, sizeof szName); 
      if (szName[0]!=ZOMBIE_STATUS) return TRUE;
   }
return FALSE;
} // GuiIsActiveTFTPTransfer


////////////////////////////////////////////////////////////////////////////////////////
//
// Reporting into LV_LOG list View
//
////////////////////////////////////////////////////////////////////////////////////////


static int AddNewTftpItem (HWND hListV, const struct S_TftpGui *pTftpGui, int Pos)
{
LVITEM      LvItem;
char        szTxt [512], szAddr[MAXLEN_IPv6], szServ[NI_MAXSERV] ;
int         itemPos;
struct tm   ltime;
char		cDel;

    //LvItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
    LvItem.mask = LVIF_PARAM | LVIF_STATE;
    LvItem.state = 0;
    LvItem.stateMask = 0;
    LvItem.iItem = Pos;      // numéro de l'item
    LvItem.lParam = (LPARAM) pTftpGui->dwTransferId;    // for Right-Click actions
    LvItem.iSubItem = 0;     // index dans la ligne
    // LvItem.pszText = "";
    itemPos = ListView_InsertItem (hListV, & LvItem);

	getnameinfo ( (LPSOCKADDR) & pTftpGui->stg_addr, sizeof (pTftpGui->stg_addr), 
		           szAddr, sizeof szAddr, 
				   szServ, sizeof szServ,
				   NI_NUMERICHOST | AI_NUMERICSERV );

    wsprintf (szTxt, "%s:%s", szAddr, szServ);
LogToMonitor ("CREATING item <%s>\n", szTxt);
    ListView_SetItemText (hListV, itemPos, FD_PEER, szTxt);

#ifdef _MSC_VER
    localtime_s (&ltime, & pTftpGui->stat.StartTime);
#else
    memcpy (& ltime, localtime (& pTftpGui->stat.StartTime), sizeof ltime);
#endif

    wsprintf (szTxt, "%02d:%02d:%02d", ltime.tm_hour, ltime.tm_min, ltime.tm_sec);
    ListView_SetItemText (hListV, itemPos, FD_START, szTxt);

    cDel = pTftpGui->opcode == TFTP_RRQ ? '<' : '>';
    wsprintf (szTxt, "%c%s%c", cDel,pTftpGui->filename, cDel );
    ListView_SetItemText (hListV, itemPos, FD_FILE, szTxt );
	return   itemPos;
} // static int AddNewTftpItem 


static int UpdateTftpItem (HWND hListV, const struct S_TftpGui *pTftpGui, int itemPos)
{
char szTxt [512];

  lstrcpy (szTxt, "N/A");
  switch (pTftpGui->stat.ret_code) 
  {
	case TFTP_TRF_RUNNING :
	  if (pTftpGui->stat.dwTransferSize > 100)
          wsprintf (szTxt, "%d%%", pTftpGui->stat.dwTotalBytes/(pTftpGui->stat.dwTransferSize/100));
	  break;
	case TFTP_TRF_SUCCESS : 
		lstrcpy (szTxt, "100%");
		break;
	case TFTP_TRF_STOPPED : 
		lstrcpy (szTxt, "STPD");
		break;
	case TFTP_TRF_ERROR : 
		lstrcpy (szTxt, "ERR");
		break;
  }
  ListView_SetItemText (hListV, itemPos, FD_PROGRESS, szTxt);

  wsprintf (szTxt, "%d", pTftpGui->stat.dwTotalBytes);
  ListView_SetItemText (hListV, itemPos, FD_BYTES, szTxt);

  wsprintf (szTxt, "%d", pTftpGui->stat.dwTransferSize);
  ListView_SetItemText (hListV, itemPos, FD_TOTAL, 
                         pTftpGui->stat.dwTransferSize==0 ? "unknown" : szTxt);

  wsprintf (szTxt, "%d", pTftpGui->stat.dwTotalTimeOut);
  ListView_SetItemText (hListV, itemPos, FD_TIMEOUT, szTxt);
  return TRUE;
} // UpdateTftpItem


static int ManageTerminatedTransfers (HWND hListV, int itemPos)
{ 
char szTxt [512];
LVITEM      LvItem;
int  tNow  = (int) time(NULL);

   szTxt[sizeof szTxt - 1]=0;
   ListView_GetItemText (hListV, itemPos, 1, szTxt, sizeof szTxt -1 );
   // The '.' is added for terminated transfer
   if (szTxt [0] != ZOMBIE_STATUS)
   {
	   // update target name
      szTxt[0] = ZOMBIE_STATUS;
	  ListView_SetItemText (hListV, itemPos, FD_FILE, szTxt);
	  // Put in param the times before deletion
      LvItem.iSubItem =FD_PEER;
      LvItem.mask = LVIF_PARAM;
	  LvItem.iItem = itemPos;
      if (ListView_GetItem (hListV, & LvItem) )
	  {
		   LvItem.lParam = sGuiSettings.nGuiRemanence + tNow;
		   ListView_SetItem (hListV, & LvItem) ;
		   // SetTimer (hListV, 
      }
   } // transfer not already marked as termnated
   else
   {
      LvItem.iSubItem =FD_PEER;
      LvItem.mask = LVIF_PARAM;
	  LvItem.iItem = itemPos;
      if (ListView_GetItem (hListV, & LvItem) && LvItem.lParam < tNow)
	  {
		  ListView_DeleteItem (hListV, itemPos);
	  }
			  
   }
return TRUE;
} // ManageTerminatedTransfers


///////////////////////////////////////////////
// populate listview
// called each new transfer or each end of transfer
int Gui_TftpReporting (HWND hListV, const struct S_TftpGui *pTftpGuiFirst)
{
LVFINDINFO  LvInfo;
int itemPos;
const struct S_TftpGui *pTftpGui;
   // date of entry
int    Ark;
short  tPos [512];

  // ListView_DeleteAllItems (hListV);
  memset (tPos, 0, sizeof tPos);

  for (Ark=0, pTftpGui = pTftpGuiFirst ; pTftpGui != NULL ; pTftpGui = pTftpGui->next, Ark++)
  {
     // search peer field (key)
      LvInfo.flags = LVFI_PARAM;
      LvInfo.lParam = pTftpGui->dwTransferId;
      itemPos = ListView_FindItem (hListV, -1, & LvInfo);
       
      // item has not been found --> should be inserted
      if (itemPos==-1)
      {
		  itemPos = AddNewTftpItem (hListV, pTftpGui, Ark);
      } // create transfers
     // actualize fields
	  UpdateTftpItem (hListV, pTftpGui, itemPos);
	  // flag : ths item has been processed
      if (itemPos < SizeOfTab(tPos)) tPos [itemPos] = 1 ;	// flag item
  }

  // manage item that are not on the stat record --> they are terminated
  for (Ark=ListView_GetItemCount (hListV) - 1 ; Ark>=0 ;  Ark-- )
      if (Ark<SizeOfTab(tPos) &&  tPos[Ark]==0)
         ManageTerminatedTransfers (hListV, Ark) ;
		 // ListView_DeleteItem (hListV, Ark);
return Ark;
} // Reporting



/////////////////////////////////////////////////
// manage drop down menu
int TftpServerListPopup (HWND hListV)
{
POINT pt;
HMENU hMenu;
int iSelected;

    // retrieve selected item
    iSelected = ListView_GetNextItem (hListV, -1, LVNI_FOCUSED);
    if (iSelected == -1) return TRUE;

    hMenu = LoadMenu(GetWindowInstance (hListV), MAKEINTRESOURCE (IDM_SERVER_LIST));
    GetCursorPos (&pt);
    TrackPopupMenu (GetSubMenu (hMenu, 0), TPM_LEFTALIGN | TPM_LEFTBUTTON,
                    pt.x, pt.y, 0, GetParent (hListV), NULL);
    DestroyMenu (hMenu);
return TRUE;
} // TftpServerListPopup

