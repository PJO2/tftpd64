//////////////////////////////////////////////////////
//
// Projet TFTPD32.   Feb 99 By  Ph.jounin
// File tftp_trf.c:  Gauge Window management
//
//
//////////////////////////////////////////////////////


#include "headers.h"


#define TFTP_MIN_VERT_GAUGE  3
#define TFTP_MIN_HORZ_GAUGE  1

// descripteur du menu système
enum { IDM_TFTP_ABORTTRF = 0x1100 };     // should be under 0xF000


//--------------------------------------------------------
static int NewGaugePos (HWND hWnd, int n);
static void Gui_FillGaugeWnd (HWND hNW, struct S_TftpGui *pTftpGui);
//--------------------------------------------------------


int CALLBACK Gui_TftpGaugeProc (HWND hWnd, UINT message, WPARAM wParam, LONG lParam)
{
HMENU hMenu;
static int nGaugeWindow;
  switch (message)
  {
        case WM_INITDIALOG :
           PostMessage (hWnd, WM_NEWPOS, 0, 0);
           break;

        case WM_NEWPOS :
            NewGaugePos (hWnd, nGaugeWindow++);
            // force redraw of main window
             if (IsWindowVisible (GetTopWindow(hWnd)))
                 PostMessage ( GetTopWindow(hWnd), 
				               WM_SHOWWINDOW, 
							   TRUE, 
							   SW_OTHERUNZOOM );
             // Add an "abort transfer" item
             hMenu = GetSystemMenu (hWnd, FALSE);
             if (hMenu != NULL)
             {
                AppendMenu (hMenu, MF_SEPARATOR, 0, NULL);
                AppendMenu (hMenu, MF_STRING, IDM_TFTP_ABORTTRF, "Abort Transfer");
             }
            break;

        case WM_FILLWND :
            Gui_FillGaugeWnd (hWnd, (struct S_TftpGui *) lParam); 
            break;

        case WM_SYSCOMMAND :
            if (wParam == IDM_TFTP_ABORTTRF)
            {const struct S_TftpGui *pTftpGui;
                // search who owns the gauge window
                for ( pTftpGui=Gui_GetFirstGuiItem () ; 
                      pTftpGui!=NULL && pTftpGui->hGaugeWnd!=hWnd ; 
                      pTftpGui=pTftpGui->next );
                // kill associated transfer
                if (pTftpGui!=NULL) 
                    PostMessage ( GetParent (GetParent (hWnd)), 
					              WM_TFTP_TRANFSER_TO_KILL, 
								  0, 
								  pTftpGui->dwTransferId );
            }
           break;

        case WM_CLOSE :
            DestroyWindow (hWnd);
            return FALSE;
        case WM_DESTROY:
           return TRUE;
  }/*endSwitch*/
return FALSE;
}/* end TftpGaugeProc */

///////////////////////////////////
// positionne la Gauge Window dans la fenêtre TFTP
///////////////////////////////////
static int NewGaugePos (HWND hWnd, int n)
{
HWND hTftpWnd = GetParent (hWnd);
RECT sTftpRect, sGaugeRect, sTftpClientRect, sGaugeClientRect;
int  NbVert, NbHoriz;

   GetWindowRect (hTftpWnd, & sTftpRect);
   GetClientRect (hTftpWnd, & sTftpClientRect);
   GetWindowRect (hWnd,     & sGaugeRect);
   GetClientRect (hWnd,     & sGaugeClientRect);
   // Ajustement des paramètres (barres de tâches et non recouvrement)
   sGaugeClientRect.bottom += 10;
   sGaugeClientRect.right  += 0;
   sTftpRect.top += sTftpRect.bottom - sTftpRect.top - sTftpClientRect.bottom;
   sTftpRect.left += 10;

// Nombre de fenêtres en vertical et Horizontal.
// Attention, on peut avoir sTftpClientRect = {0,0}.
// Pour éviter la division par 0, on force NbVert=3, NbHoriz=1.
   NbVert  =  max (TFTP_MIN_VERT_GAUGE, sTftpClientRect.bottom  / sGaugeClientRect.bottom);
   NbHoriz =  max (TFTP_MIN_HORZ_GAUGE, sTftpClientRect.right   / sGaugeClientRect.right);

  SetWindowPos (hWnd, NULL,
                sTftpRect.left +  sGaugeClientRect.right  * ( (n / NbVert) % NbHoriz )  ,
              sTftpRect.top  +  sGaugeClientRect.bottom * ( n % NbVert ) ,
               0, 0,
              SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOSIZE );
return TRUE;
} // NewGaugePos

/* ------------------------------------------------- */
/* Create the Gauge Window                           */
/* ------------------------------------------------- */
HWND Gui_CreateGaugeWindow (HWND hParentWnd, const struct S_TftpGui *pTftpGui)
{
HWND hNW;
    hNW = CreateDialog (GetWindowInstance(hParentWnd), // hInstance
                        MAKEINTRESOURCE (IDD_DIALOG_GAUGE),
                        hParentWnd,           // Parent
                        (DLGPROC) Gui_TftpGaugeProc);  // CallBack

    if (hNW == NULL)
    {
            CMsgBox  (hParentWnd, "Error : Can't create temporary window", APPLICATION, MB_OK);
            return NULL;
    }

    // start background task
    PostMessage (hNW, WM_FILLWND, 0, (LPARAM) pTftpGui);
return hNW;
} //CreateGaugeWindow



/////////////////////
// UpdateGaugeWindow : Design of gauge window
/////////////////////
static void Gui_FillGaugeWnd (HWND hNW, struct S_TftpGui *pTftpGui)
{
char  szTitle [_MAX_PATH + sizeof " from " + MAXLEN_IPv6];
int   len;

    assert (pTftpGui!=NULL);

    // retrieve file name from GUI datagram
    len = wsprintf ( szTitle, "%s %s ",
                     pTftpGui->filename,
                     pTftpGui->opcode == TFTP_RRQ ? "to" : "from" );
	getnameinfo ( (LPSOCKADDR) & pTftpGui->stg_addr, sizeof (pTftpGui->stg_addr), 
		           szTitle+len, sizeof szTitle - len, 
				   NULL, 0,
				   NI_NUMERICHOST );

    SetWindowText (hNW, szTitle);
    if (pTftpGui->stat.dwTransferSize != 0)
           wsprintf (szTitle, "File size : %d", pTftpGui->stat.dwTransferSize);
    else
    {
          wsprintf (szTitle, "File size : Unknown");
          ShowWindow (GetDlgItem (hNW, IDC_TRF_PROGRESS), SW_HIDE);
         // suppress gauge from window
          SetWindowPos (hNW, 0, 0, 0,
                        X_LOG2PHYS (130), Y_LOG2PHYS (35),
                        SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER );
    }
    SetWindowText (GetDlgItem (hNW, IDC_FILE_SIZE), szTitle);

    ShowWindow (hNW, SW_SHOW);
} // FillGaugeWnd


void Gui_UpdateGaugeWindow (const struct S_TftpGui *pTftpGui, time_t dNow)
{
HWND hGaugeWnd = pTftpGui->hGaugeWnd;
HWND hGWnd;
char            szTitle [_MAX_PATH+sizeof " from 255.255.255.255 "];

   if (hGaugeWnd == NULL)  return;

   // do not update gauge window if last update has been done in the current second
   // NB: another feature is to avoid division by 0
   if (pTftpGui->stat.dLastUpdate == dNow)  return;

   // update progress bar
   hGWnd = GetDlgItem (hGaugeWnd, IDC_TRF_PROGRESS);
   if (pTftpGui->stat.dwTransferSize>100)
        SendMessage (hGWnd, PBM_SETPOS, 
                     pTftpGui->stat.dwTotalBytes/(pTftpGui->stat.dwTransferSize/100), 
                     0);

   // Update stat text
   wsprintf (szTitle, "%d Bytes %s \t %d Bytes/sec",
             pTftpGui->stat.dwTotalBytes,
             (pTftpGui->opcode == TFTP_RRQ) ? "sent" : "rcvd",
             pTftpGui->stat.dwTotalBytes / (dNow-pTftpGui->stat.StartTime) );

   SetWindowText (GetDlgItem (hGaugeWnd, IDC_FILE_STATS), szTitle);
   
} // UpdateGaugeWindow


