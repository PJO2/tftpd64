//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Mai 98 Ph.jounin - Jan 2003
// File qui_log.c:  Log management (tab event viewer)
//
//////////////////////////////////////////////////////


#include "headers.h"
#include <process.h>
#include <stdio.h>


// ---------------------------------------------------------------
// GUI  management
// ---------------------------------------------------------------


/* ------------------------------------------------- */
/* Manage a list box as a log window                 */
/* ------------------------------------------------- */
void LB_LOG (HWND hListBox, const char *szTxt)
{
int        Ark, Rc, NbMsg;
int        dwMaxExtent, dwExtent;
SIZE       sTextSize = {0, 0};
HDC        hDC;
DWORD      dwMaxMsg = 250;
char       szBuf[LOGSIZE + 30];
static bInit=FALSE;

#define CROLLBAR
#ifdef CROLLBAR

	if (!bInit) 
	{
		SendMessage (hListBox, LB_INITSTORAGE, dwMaxMsg, 0);
		bInit = TRUE;
	}

   // delete oldest messages
   for ( Rc = SendMessage (hListBox, LB_GETCOUNT, 0, 0);
		 Rc > (long) dwMaxMsg ;
         Rc = SendMessage (hListBox, LB_DELETESTRING, 0, 0) );

    Ark = (int) SendMessage (hListBox, LB_ADDSTRING, 0, (LPARAM) szTxt);
	if (Ark==LB_ERR) 
			return;

	// record extent size
    hDC = GetDC (hListBox);
    GetTextExtentPoint32 (hDC, szTxt, lstrlen (szTxt), & sTextSize);
    LPtoDP (hDC, (POINT *)  & sTextSize, 1);	//  convert to physical units
    dwExtent = (sTextSize.cx *9) / 10;		// experimental !
	SendMessage (hListBox, LB_SETITEMDATA, Ark, (LPARAM) dwExtent);
	ReleaseDC (hListBox, hDC);

    // search the longest text 
    dwMaxExtent = 0;
	NbMsg = (int)SendMessage(hListBox, LB_GETCOUNT, 0, 0);
    for (Ark= NbMsg; Ark>=0 ; Ark --)
    {
        dwExtent = SendMessage (hListBox, LB_GETITEMDATA, Ark-1, (LPARAM) szBuf);
        if (dwExtent > dwMaxExtent)  dwMaxExtent = dwExtent ;
     }
	// and extent horizontal scrollbar accordingly
    SendMessage (hListBox, LB_SETHORIZONTALEXTENT, dwMaxExtent, 0);
	// scroll to the end of the list box
	SendMessage (hListBox, LB_SETTOPINDEX, NbMsg-1, 0);
#endif
} // LB_LOG

//////////////////////////////////////
// Log Thread management
//////////////////////////////////////


