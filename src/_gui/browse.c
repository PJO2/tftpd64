//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Mars 2000 Ph.jounin
// File browse.c:   Browse window management
//                  just a wrapper for SHBrowseForFolder API
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////


#include "headers.h"
#include <shlobj.h>

//////////////////////////////////////////////
// Select the current directory into the ListView
//////////////////////////////////////////////
static int CALLBACK BrowseCallbackProc (HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
  switch (uMsg)
  {
      case BFFM_INITIALIZED  : SendMessage (hWnd, BFFM_SETSELECTION, TRUE, lpData);
                               return FALSE;
  }
return FALSE;
} // BrowseCallbackProc 



//////////////////////////////////////////////
// Displays Browse window
//////////////////////////////////////////////

BOOL MyBrowseWindow (HWND hWnd, LPSTR szBrowsePath, BOOL bOpenCurDir)
{
BROWSEINFO BrowseInfo;
LPITEMIDLIST  lpItem;

   memset (& BrowseInfo, 0, sizeof BrowseInfo);
   // GetCurrentDirectory (MAX_PATH, szBrowsePath);
   BrowseInfo.hwndOwner  = hWnd;
   // 2010-08-13 : Change proposed by Nathan Alderson BIF_USENEWUI
   BrowseInfo.ulFlags    = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
   BrowseInfo.lParam     = (LPARAM) szBrowsePath;
   if (bOpenCurDir)   BrowseInfo.lpfn = BrowseCallbackProc;
   lpItem = SHBrowseForFolder (& BrowseInfo);
   // Do not change dir, wait for OK button
return (lpItem!= NULL  && SHGetPathFromIDList (lpItem, szBrowsePath));
} // MyBrowseWindow
