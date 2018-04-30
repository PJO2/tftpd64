//////////////////////////////////////////////////////
//
// Projet TFTPD32.   Mai 98 Ph.jounin
// File windo.c:  re usable window functions
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////


#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
// Macro redefinition. Removing
// #define _WIN32_IE 0x500
#include <shellapi.h>
#include <stdio.h>

#include "../log/LogToMonitor.h"
#include "../lasterr/lasterr.h"
#include "window.h"



/* ------------------------------------------------- */
/* Open a New Dialog Box                             */
/* ------------------------------------------------- */
int  OpenNewDialogBox ( HWND        hParentWnd,
                        DWORD       dwDlgBox,
                        DLGPROC     lpProc,
                        LPARAM      lParam,
                        HINSTANCE   hInstance)
{
int       Rc;
  if (hInstance == NULL)  hInstance = GetWindowInstance (hParentWnd);
     Rc = (int) DialogBoxParam (hInstance,
								MAKEINTRESOURCE (dwDlgBox),
								hParentWnd,
								lpProc,
								lParam);
return Rc;
} /* OpenNewDialogBox */


////////////////////////////
// Attach and destroy icon to the TaskTray
void TrayMessage(HWND hDlg, DWORD dwMessage, HICON hIcon, int TaskTrayId, int uCallBckMsg)
{
NOTIFYICONDATA IconData;

LogToMonitor ("TrayMessage Call : msg %d <%s>, wnd %d, id %d\n", dwMessage,
			  dwMessage==NIM_ADD ? "ADD" : dwMessage==NIM_DELETE ? "DELETE" : "OTHER",
			  hDlg, TaskTrayId );
   memset (& IconData, 0, sizeof IconData);
   // IconData.cbSize = NOTIFYICONDATA_V1_SIZE;

   IconData.cbSize = sizeof IconData;

   IconData.hWnd = hDlg;
   IconData.uID = TaskTrayId ;
   if (dwMessage != NIM_DELETE)
   {
	   IconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	   IconData.uCallbackMessage = uCallBckMsg;
	   IconData.hIcon = hIcon;
	   GetWindowText (hDlg, IconData.szTip, sizeof IconData.szTip - 1);
   }
		
   if ( ! Shell_NotifyIcon (dwMessage, &IconData) )
	   LogToMonitor ("error %d in Shell_NotifyIcon <%s>\n", 
					 GetLastError (), LastErrorText () );
} // TrayMessage



/* ------------------------------------------------- */
/* Create an hidden background window                */
/* ------------------------------------------------- */
HWND CreateBckgWindow (HWND hWnd, 
                       WORD wMessage, 
                       WNDPROC CbkProc, 
                       const char *szName, 
                       const char *szAppli)
{
HWND    hNW;
int      Rc;
WNDCLASS WndClass;
RECT     sParentRect;
    if ( ! GetClassInfo (GetWindowInstance (hWnd), szName, & WndClass) )
   {
      // Create a new class
        memset (& WndClass, 0, sizeof WndClass);
        WndClass.lpfnWndProc   = CbkProc;
        WndClass.hInstance     = GetWindowInstance (hWnd);
        WndClass.lpszClassName = szName;
        Rc = RegisterClass (& WndClass);
       if (Rc==0)
       {
            MessageBox (NULL, "Can not register class", szAppli, MB_OK | MB_ICONSTOP);
            return NULL;
       }
    }
    // Get parent's dimension and create the window with the same dimension
    GetWindowRect (hWnd, & sParentRect);
    hNW = CreateWindow (szName,
                        NULL,          // window name
                        WS_CHILD,        // dwStyle
                        sParentRect.left,
                        sParentRect.top,
                        sParentRect.right - sParentRect.left,
                        sParentRect.bottom - sParentRect.top,
                        hWnd,     // parent
                        NULL,         // Menu
                        GetWindowInstance(hWnd), // hInstance
                        NULL);            // creation param
    if (hNW == NULL)
    {
            MessageBox (NULL, "Error : Can't create temporary window", szAppli, MB_OK | MB_ICONSTOP);
            return NULL;
    }
    // start background task
    PostMessage (hNW, wMessage, 0, 0);
return hNW;
} //CreateBckgWindow


/* ------------------------------------------------- */
/* Copy the content of a List Box into ClipBoard     */
/* ------------------------------------------------- */
void CopyListBoxToClipboard (HWND hListBox)
{
char *p, *pCur;
int NbMsg, Ark, NbChar;

   NbMsg = (int) SendMessage (hListBox, LB_GETCOUNT, 0, 0);
   // Nb chars in list box 
   for (NbChar=0, Ark=0 ;  Ark < NbMsg ;  Ark++)
      NbChar += (int) SendMessage (hListBox, LB_GETTEXTLEN, Ark, 0);

   // allocat enough space for char + end of lines + NULL
   NbChar += 2 * NbMsg + 1;
   p = malloc (NbChar);
   if (p==NULL) { MessageBeep (-1); return; }

   for (pCur=p, Ark=0 ;  Ark < NbMsg ;  Ark++)
   {
      // verify that we do not overflow
      // A thread may have put some text
     if (pCur + SendMessage (hListBox, LB_GETTEXTLEN, Ark, 0)  >=  p + NbChar)
          break;
     pCur += SendMessage (hListBox, LB_GETTEXT, Ark, (LPARAM) pCur);
    *pCur++ = '\r';
    *pCur++ = '\n';
   }
   *pCur = 0;
   CopyTextToClipboard (p);
   free (p);
} // CopyListBoxToClipboard


/* -------------------------------------------------- */
/* Copy the content of a List View into ClipBoard     */
/* -------------------------------------------------- */
void CopyListViewToClipboard (HWND hListV, int nbSubItems)
{
char *p, *pCur;
int NbMsg, Ark, Evan, NbChar, buf_len;
char buf[256];
     
   NbMsg = ListView_GetItemCount (hListV);
   // Nb chars in list box 
   for (NbChar=0, Ark=0 ;  Ark < NbMsg ;  Ark++)
       for (Evan=0 ; Evan<nbSubItems ; Evan++)
       {
           ListView_GetItemText (hListV, Ark, Evan, buf, sizeof buf);
           NbChar += lstrlen (buf);
       }

   // allocate enough space for char + tabs + end of lines + NULL
   NbChar += 2 * NbMsg + NbMsg * nbSubItems + 1;
   p = malloc (NbChar);
   if (p==NULL) { MessageBeep (-1); return; }

   for (pCur=p, Ark=0 ;  Ark < NbMsg ;  Ark++)
   {
       for (Evan=0 ; Evan<nbSubItems ; Evan++)
       {
           ListView_GetItemText (hListV, Ark, Evan, buf, sizeof buf);
           buf_len = lstrlen (buf);
            // verify that we do not overflow
            // since a thread may have put some text
           if (pCur + buf_len > p +NbChar) break;
           memcpy (pCur, buf, buf_len);
           pCur += buf_len;
           *pCur++ = '\t';
       }
      *pCur++ = '\r';
      *pCur++ = '\n';
   }
   *pCur = 0;
   CopyTextToClipboard (p);
   free (p);
} // CopyListViewToClipboard


// Copy a string into Clipboard
// used to copy file into Dir window, an IP address from IP combobox 
// and copy buttons (log viewer and syslog)
BOOL CopyTextToClipboard (const char *sz)
{
int    nLength;
HANDLE hMemory;
void  *pMemory;

  nLength = lstrlen (sz);
  hMemory = GlobalAlloc(GHND, nLength + 1);
  // hMemory = GlobalAlloc(GMEM_DDESHARE, nLength + 1);
  pMemory = GlobalLock(hMemory);
  lstrcpy (pMemory, sz);
  GlobalUnlock(hMemory);

  if (OpenClipboard (NULL))
  {
      EmptyClipboard();
      SetClipboardData (CF_TEXT,hMemory);
      CloseClipboard();
  }
return TRUE;
} // CopyStringIntoClipboard


// copy a ComboBox and select a given string
// Note : Since CB_IP has changed from text to a S_If structure
// this is not exactly a Copy function...
int CopyCBContent (HWND hFromCB, HWND hToCB, const char *lpszFind, int family)
{
int Rc=-1;
int Ark, n;
struct S_If 
{
	char  *descr;
	char  *addr;
} // Struct S_If 
*pif;
   // erase content
   ComboBox_ResetContent (hToCB);
   // get n of source items
   n = ComboBox_GetCount (hFromCB);
   // copy original combo
   for (Ark=0 ; Ark<n ; Ark++)
   {
	   pif = (struct S_If *) ComboBox_GetItemData (hFromCB, Ark);
	   // if AF_INET specified do not copy IPv6 address (the ones with : in the address)
	   if (! (family==AF_INET && strchr (pif->addr, ':')!=NULL)) ComboBox_AddString (hToCB, pif->addr);
   }
	  // locate string and select if found
   if (lpszFind!=NULL  && lpszFind[0]!=0)
   {
		Rc = ComboBox_FindStringExact (hToCB, 0, lpszFind);
		if (Rc==-1) 
		{
			// DHCP bound address is not present into the interface list
			// (because tftpd32.ini has been edited or interface address has changed...)
			char sz[128];
			sscanf (lpszFind, "%[0-9.:]", sz);
			lstrcat (sz, " [Bad Addr]");
			Rc = ComboBox_InsertString (hToCB, 0, sz);
		}
   }
   ComboBox_SetCurSel (hToCB, Rc==-1 ? 0 : Rc);
return Ark;
} // CopyCBContent
