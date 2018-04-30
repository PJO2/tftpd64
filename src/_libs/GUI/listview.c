//////////////////////////////////////////////////////
//
// Projet TFTPD32.  June 2006 Ph.jounin
// File listview.c: ListView management
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////



#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include "listview.h"

// A workaround for a quirk bug : if the listview owns the focus, it is not properly refreshed
// --> all ListView windows are subclassed to LVProc.
static WNDPROC wpOrigLVProc ;   
#define TM_REPAINT   WM_USER+10
#define TM_REPAINT2   WM_USER+11


// This proc just hide and show the listview control and display is OK
// All others messages are parsed by the default ListView handler
LRESULT CALLBACK LVProc (HWND hWnd, UINT message, WPARAM wParam, LONG lParam)
{
   switch (message)
   {
      case WM_ERASEBKGND : 	if (GetFocus ()==hWnd) 
                                SetTimer (hWnd, TM_REPAINT, 10, NULL);                            
                            break;
      case TM_REPAINT : // Force full repaint of the list view
			// This will cause a annoying flashing effect
            ShowWindow (hWnd, SW_HIDE);
            SetTimer (hWnd, TM_REPAINT2, 10, NULL);                            
            break;

      case TM_REPAINT2 :
            ShowWindow (hWnd, SW_SHOW);
            break;

       case WM_TIMER :
            KillTimer(hWnd, wParam);
            PostMessage (hWnd, (UINT) wParam, 0, 0);
            break;
   }
return CallWindowProc (wpOrigLVProc, hWnd, message, wParam, lParam);
} // LVProc


// SetViewMode : force a style and ExtStyle for the ListView
void SetViewMode (HWND hwndLV, DWORD dwView, DWORD dwExtStyle)
{ 
	// recuperer le style actuel
	DWORD dwStyle = GetWindowLong(hwndLV, GWL_STYLE);
	if ((dwStyle & LVS_TYPEMASK) != dwView)
		SetWindowLong(hwndLV, GWL_STYLE, (dwStyle & ~LVS_TYPEMASK) | dwView);
   dwStyle = ListView_GetExtendedListViewStyle (hwndLV);
   dwStyle |= dwExtStyle; 
   ListView_SetExtendedListViewStyle (hwndLV, dwStyle);
}  //  SetViewMode


// InitTftpd32ListView : Creates a ListView in the Tftpd32 style : LVS_REPORT 
// An array gives the text headers, the text alignment and the columns width
BOOL InitTftpd32ListView (HWND hListV, const struct S_LVHeader *tCol, int nb, DWORD dwExtStyle)
{
int Ark;
LVCOLUMN lvc;

	SetViewMode (hListV, LVS_REPORT, dwExtStyle);
	// subclass LV Item
    // wpOrigLVProc = (PVOID)SetWindowLong (hListV, GWL_WNDPROC, (LONG) LVProc);
    wpOrigLVProc = (WNDPROC) SetWindowLongPtr(hListV, GWLP_WNDPROC, (LONG_PTR)LVProc);

	for (Ark = 0; Ark < nb; Ark++)
	{
		lvc.mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH | LVCF_SUBITEM;
		lvc.fmt = tCol[Ark].fmt;
		lvc.cx = tCol[Ark].cx;
		lvc.pszText = tCol[Ark].pszText;
		lvc.iSubItem = Ark;
		SendMessage (hListV, LVM_INSERTCOLUMN, Ark, (LPARAM) &lvc);
	}
return Ark;
} // InitTftpd32ListView


// Alternates background 
LRESULT ProcessCustomDraw (LPARAM lParam)
{
LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)lParam;

    switch (lplvcd->nmcd.dwDrawStage) 
    {
            //request notifications for individual listview items
        case CDDS_PREPAINT : //Before the paint cycle begins
                    return CDRF_NOTIFYITEMDRAW;

        case CDDS_ITEMPREPAINT: //Before an item is drawn
                    lplvcd->clrText   = RGB(0, 0, 0);
                    lplvcd->clrTextBk = (lplvcd->nmcd.dwItemSpec & 1) ? RGB(224,255,255) : RGB(255,255,255);
                    return CDRF_NEWFONT;
    }
return CDRF_DODEFAULT;
} // ProcessCustomDraw


// CompareStringFunc not used for DHCP server
int CALLBACK CompareStringFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
LPNM_LISTVIEW pnm = (LPNM_LISTVIEW)lParamSort;
char str1[256], str2[256];
    //Get strings item
    ListView_GetItemText(pnm->hdr.hwndFrom, lParam1, pnm->iSubItem, str1, sizeof str1);
    ListView_GetItemText(pnm->hdr.hwndFrom, lParam2, pnm->iSubItem, str2, sizeof str2);
return strcmp (str1, str2);
}

 