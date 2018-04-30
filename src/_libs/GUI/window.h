//////////////////////////////////////////////////////
//
// Projet TFTPD32.   Mai 98 Ph.jounin
// File window.c:  re usable windows functions
//
// source released under artistic license (see license.txt)
//
//////////////////////////////////////////////////////


int  OpenNewDialogBox ( HWND       hParentWnd,
                        DWORD       dwDlgBox,
                        DLGPROC       lpProc,
                        LPARAM      lParam,
                        HINSTANCE   hInstance);
/* Attach and destroy icon to the TaskTray */
void TrayMessage(HWND hDlg, DWORD dwMessage, HICON hIcon, int TaskTrayId, int uCallBackMsg);

/* Create an hidden background window                */
HWND CreateBckgWindow (HWND hWnd, WORD wMessage, WNDPROC CbkProc, 
                       const char *szName, const char *szAppli);


/* Copy the content of a List Box into ClipBoard     */
void CopyListBoxToClipboard (HWND hListBox);

/* Copy the content of a List View into ClipBoard     */
void CopyListViewToClipboard (HWND hListV, int nbSubItems);


/* Copy a string into Clipboard */
BOOL CopyTextToClipboard (const char *sz);

/* copy a combo box into another one */
int CopyCBContent (HWND hFromCB, HWND hToCB, const char *lpszFind, int family);
