//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Nov 2002 Ph.jounin
// File tftp_dir.c:   Display contents of current directory
//
// released under artistic license
//
//////////////////////////////////////////////////////


#include "headers.h"


#ifdef _M_X64
#  define TFTPD_DIR_TITLE  "Tftpd64: directory"
#else
#  define TFTPD_DIR_TITLE  "Tftpd32: directory"
#endif


// Start an explorer window, directory is Tftpd32's default directory
int StartExplorer (void)
{
// A fix from Colin to authorize directories including commas
#define EXPLORER "explorer.exe \""
STARTUPINFO sInfo;
PROCESS_INFORMATION pInfo;
int Rc;
char szCmdString [sizeof EXPLORER + _MAX_PATH];

   lstrcpy (szCmdString, EXPLORER);
   GetActiveDirectory (szCmdString + (sizeof EXPLORER - 1), _MAX_PATH-1);
   lstrcat (szCmdString, "\"");

   memset (& sInfo, 0, sizeof sInfo);
   sInfo.cb = sizeof sInfo;
   
   Rc = CreateProcess(NULL, szCmdString, NULL, NULL, FALSE,
  				     NORMAL_PRIORITY_CLASS, NULL, NULL, &sInfo, &pInfo) ;
   CloseHandle (pInfo.hProcess);
   CloseHandle (pInfo.hThread);
return Rc;
} // StartExplorer 


////////////////////////
// The change directory command :
// manages the CB_DIR combo box
int TftpDir_AddEntry (HWND hCBWnd, const char *szPath)
{
int Rc=-1;

     Rc = ComboBox_FindStringExact (hCBWnd, -1, szPath);
     if (Rc==CB_ERR)  Rc = ComboBox_AddString (hCBWnd, szPath);
     ComboBox_SetCurSel (hCBWnd, Rc);
return Rc;
} // TftpDir_AddEntry



int TftpDir_SelectEntry (HWND hCBWnd)
{
char szPath[_MAX_PATH];

    int n = ComboBox_GetCurSel (hCBWnd);
    if (     n!=CB_ERR  
        &&  ComboBox_GetLBTextLen (hCBWnd, n) < sizeof szPath  )
    {
        ComboBox_GetLBText (hCBWnd, n, sGuiSettings.szWorkingDirectory);
    }
return TRUE;
}  // TftpDir_SelectEntry



// list the entries (called ONCE at the end in order to save them)
int TftpDir_GetNextEntry (HWND hCBWnd)
{
static int n;
static char szPath [_MAX_PATH];
return (ComboBox_GetLBText (hCBWnd, n++, szPath) != CB_ERR);
} // TftpDir_GetNextEntry

////////////////////////
// Callback for the Show Dir function
static int CbkDisplay (char *szLine, DWORD dw)
{
      SendMessage ((HWND) dw, LB_ADDSTRING, 0, (LPARAM) szLine);
      return 0;
}



// The commands : Select, OK & Cancel
static int Handle_VM_Command (HWND hWnd, WPARAM wParam, LPARAM lParam)
{
int wItem = (int) LOWORD (wParam);
     switch (wItem)
     {
     // Copy filename into clipboard
        case IDC_LB_SHDIR :
           if (HIWORD(wParam) == LBN_SELCHANGE)
           {char szLine [256], *p ;
            int n = (int) SendMessage ((HWND) lParam, LB_GETCURSEL, 0, 0);
             SendMessage ((HWND) lParam, LB_GETTEXT, n, (LPARAM) szLine);
             if (SendMessage ((HWND) lParam, LB_GETTEXTLEN, n, 0) >= sizeof szLine)
                       break;
             // cut string before tab
             p = strchr (szLine, '\t');
             if (p!=NULL)  *p=0;
             CopyTextToClipboard (szLine);
           }
          break;
         case IDOK :
             // fallthrough
         case IDCANCEL :
             EndDialog (hWnd, 0);
             break;
		case IDC_SD_COPY : 
             // do nothing since the file name has already been copied
             break; 
		case IDC_SD_EXPLORER : // start an explorer and close window
			 StartExplorer ();
             PostMessage (hWnd, WM_CLOSE, 0, 0);
			 break;
     } // switch wItem
return FALSE;
} // Handle_VM_Command



/////////////////////////////////////////
// Dir window callback
LRESULT CALLBACK ShDirProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
static const int tTabs[] = { 90, 125 };
struct S_DirectoryContent *pDir = (struct S_DirectoryContent *) lParam;
HWND hLBWnd = GetDlgItem (hWnd, IDC_LB_SHDIR);
int Ark;

  switch (message)
  {
       case WM_INITDIALOG :
	   	   // Set the window name to either tftpd32 or tftpd64
	       SetWindowText (hWnd, TFTPD_DIR_TITLE);

           ListBox_SetTabStops ( hLBWnd, SizeOfTab(tTabs), tTabs );
           ListBox_ResetContent ( hLBWnd );
           for ( Ark=0 ;  Ark < pDir->nb ;  Ark++ )
                ListBox_AddString ( hLBWnd, pDir->ent[Ark].file_descr );
           CenterChildWindow (hWnd, CCW_INSIDE | CCW_VISIBLE);
           // If GUI is in remote mode, deactivate Explorer Button
           if ( IsGuiConnectedToRemoteService () )
                Button_Enable (GetDlgItem (hWnd, IDC_SD_EXPLORER), FALSE);
           break;

       case WM_COMMAND :
            Handle_VM_Command (hWnd, wParam, lParam);
           break;
       case WM_CLOSE :
       case WM_DESTROY :
            EndDialog (hWnd, 0);
            break;

  } // switch

return FALSE;
} // ShDirProc

