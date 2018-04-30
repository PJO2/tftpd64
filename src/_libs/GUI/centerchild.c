////////////////////////
// Projet DHCPD32.  January 2006 Ph.jounin
//                  A free DHCP server for Windows
// File   CMsgBox
// Display a centered messagebox
// Code from Microsoft Knowledge base Q180936
//
// Released under European Union Public License
// 
////////////////////////


#include <windows.h>
#include "centerchild.h"


/////////////////////////////////////////////////////////////////////////
// A Window centered on its parent
/////////////////////////////////////////////////////////////////////////
BOOL CenterChildWindow (HWND hChildWnd, int uType)
{
HWND hParentWnd ;
RECT sParentRect, sChildRect, sWorkArea;
int  x, y;

    hParentWnd = GetParent (hChildWnd);
    if (hParentWnd == NULL)  return FALSE;

    if ( (uType & CCW_VISIBLE) && ! IsWindowVisible (hParentWnd) ) return FALSE;

       // compute child position
    GetWindowRect (hParentWnd, & sParentRect);
    GetWindowRect (hChildWnd, & sChildRect);
    SystemParametersInfo (SPI_GETWORKAREA, 0, & sWorkArea, 0);

    x = sParentRect.left + (sParentRect.right - sParentRect.left) / 2 - (sChildRect.right - sChildRect.left) / 2;
    y = sParentRect.top + (sParentRect.bottom - sParentRect.top) / 2  - (sChildRect.bottom - sChildRect.top) / 2;

    // Child larger than its parent ?
    if ( (uType & CCW_INSIDE)  &&
         (    x < sParentRect.left  ||  y < sParentRect.top )
       )
      return FALSE;

    // Child window outside the screen ?
    if (  (uType & CCW_VISIBLE) &&
        (     x<sWorkArea.left  || x+sChildRect.right-sChildRect.left>sWorkArea.right
           || y<sWorkArea.top   || y+sChildRect.bottom-sChildRect.top>sWorkArea.bottom ) )
           return FALSE;

    // OK : change window Pos
    SetWindowPos (hChildWnd, NULL, x,  y, 0, 0,
                  SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );

return TRUE;
} // CenterChildWindow

