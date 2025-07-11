//////////////////////////////////////////////////////
//
// Projet cmsgbox.  Oct 2015 Ph.jounin
// File cmsgbox.c:  A replacement for MessageBox
// released under GPLv2 license
//
//////////////////////////////////////////////////////


#include <Windows.h>
#include <Windowsx.h>
#include <strsafe.h>
#include "cmsgbox.h"
#define SizeOfTab(x) (sizeof (x) / sizeof (x[0]))

// 4 button max by dialog box
#define MAXBUTTONS 4


// params passed from myMsgBox API to the Callback function
struct sParam
{
	const char *text;
	const char *title;
	UINT  uStyle;
};


// mapping between style and buttons
// Note: style not used : we used the style retrieved from parameter as index
static const struct sMapping
{
	UINT style_only_for_program_comprehension;	 // unused
	int  nButtons;								 // # of buttons to be displayed
	int  tButtons[MAXBUTTONS];					 // id of the buttons
}
tMapping[] = 
{
	// MB_OK is 0, MB_OKCANCEL is 1 and so on... 
	// Mapping will be done directly using sMapping[style]
	{	MB_OK,	               1, { IDOK,     -1,        -1,        -1 }     }, 
	{	MB_OKCANCEL,           2, { IDOK,     IDCANCEL,  -1,        -1 }     },
	{	MB_ABORTRETRYIGNORE,   3, { IDABORT,  IDRETRY,  IDCANCEL,   -1 }     },
	{	MB_YESNOCANCEL,        3, { IDYES,    IDNO,     IDCANCEL,   -1 }     },
	{	MB_YESNO,              2, { IDYES,    IDNO,     -1,         -1 }     },
	{	MB_RETRYCANCEL,        2, { IDRETRY,  IDCANCEL, -1,         -1 }     },
	{	MB_CANCELTRYCONTINUE,  3, { IDCANCEL, IDRETRY,  IDCONTINUE, -1 }     },
};

// mapping between button ID and its text
// Agin:  should match IDOK, IDCANCEL, ...  since we index directly szButtonText[buttonid] 
static const char *szButtonText[] = { 
								 "undefined", 
								 "OK", 
								 "Cancel", 
								 "Abort", 
								 "Retry", 
								 "Ignore", 
							     "Yes", 
								 "No", 
								 "Close", 
								 "Help", 
								 "Try Again", 
								 "Continue", 
							  };

// mapping between style and icon
static const LPCTSTR tIcon[] = 
{
	NULL,
	IDI_HAND,				// 0x10
	IDI_QUESTION,			// 0x20
	IDI_EXCLAMATION,		// 0x30
	IDI_ASTERISK,			// 0x40
}; 


// A custom messagebox to be displayed at the center of its parent window
// without using SetWindowHook

#define TEXT_X_POSITION  50	    // Place of the edittext control into the dialogbox
#define TEXT_Y_POSITION  10
#define ICON_X_POSITION   5
#define ICON_Y_POSITION   5


#define CX_MARGIN       80		// margin for icon
#define CY_MARGIN       70		// margin for buttons

#define BUTTON_LENGTH   60		// size of a button
#define BUTTON_SPACE    10		// inter button space
#define BUTTON_HEIGHT   25		// size of a button

#define DLGTITLE  L"Debug"				// overwritten by INIT_DIALOG
#define DLGFONT   L"MS Sans Serif"
 


// 
// an empty dialog box template
//
#pragma pack(push, 4)                 
const static struct { // dltt 
    DWORD  style; 
    DWORD  dwExtendedStyle; 
    WORD   ccontrols; 
    short  x; 
    short  y; 
    short  cx; 
    short  cy; 
    WORD   menu;         // name or ordinal of a menu resource
    WORD   windowClass;  // name or ordinal of a window class
    WCHAR  wszTitle[sizeof DLGTITLE]; // title string of the dialog box
    short  pointsize;       // only if DS_SETFONT flag is set
    WCHAR  wszFont[sizeof DLGFONT];   // typeface name, if DS_SETFONT is set
} 
sEmptyDialogBox = 
{
   // only style and font are used
   WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU  | DS_MODALFRAME  | DS_SETFONT ,
   0x0,						// exStyle;
   0,						// # of ccontrols
   0, 0, 10, 10,			// position --> don't care
   0,                       // menu: none
   0,                       // window class: none
   DLGTITLE,                // temporary Window caption
   8,                       // font pointsize
   DLGFONT,					// font 
 };

#pragma pack(pop)



// resources created by OnInitDialog 
// used by WM_PAINT, WM_COMMAND or WM_CLOSE
static struct S_myMsgBoxResources 
{
   HICON hIcon;                 // system icon
   int  style;					// short style, used as an index in tMapping struct
   HWND hwndButton[MAXBUTTONS]; // handler of buttons
   HWND hwndText;               // handler of edittext control
} myMsgBoxResources;



//
// A multi line version of GetTextEXtPoint
//
static int MLGetTextExtentPoint (HWND hWnd, const char *lpText, int nLen, SIZE *lpRect)
{
SIZE        subRect;
const char *lpCur, *lpPrev;
int         n;
BOOL		Rc=TRUE;
HDC 		hDC;


	// retrieve Display Context
	hDC = GetDC (hWnd);

	// iterate for each line,
	//    if it is larger than the previous ones, take its length...
	lpRect->cx = lpRect->cy = 0;
	n = 0;
	lpCur = lpPrev = lpText;
	do
	{
		if (*lpCur!='\n'  &&  *lpCur!=0   &&  n<nLen) 
			continue;   			// next character please

		// we have reached an end of line (or the last char)
		// take its length
		Rc = (BOOL) GetTextExtentPoint32 (hDC, lpPrev, lpCur-lpPrev, & subRect);
		// keep the max
		lpRect->cx = max (lpRect->cx, subRect.cx);
		// add a text line
		lpRect->cy += subRect.cy;
		// save the begin of next line for iteration
		lpPrev = lpCur+1;
	}
	while (Rc &&  n++ < nLen && *lpCur++!=0 );

	// release 
	ReleaseDC (hWnd, hDC);
return Rc;
} // MLGetTextExtentPoint




//
// Ok, now were are in 
// Tasks : determine the size of the window
//         create the text window, 
//         create the buttons
//         assign the default button
//         paste text into the control
//         change window title
//         change window size
//         center window
//
static int OnInitDialog (HWND hwndDlg, const struct sParam *lpData, struct S_myMsgBoxResources *res)
{
SIZE Size;
RECT sParentRect;
int Ark, nIconIdx, nDefButtonIdx;
int x,y, xButtonsWidth;
HICON    hParentIcon;
HWND     hParentWnd;
	
		// Since this procedure is responsible for resources allocation
		// we can safely initialize the resources shared memory
		memset (res, 0, sizeof *res);

		// retrieve the size cx and cy of the text to be displayed
		MLGetTextExtentPoint (hwndDlg, lpData->text, lstrlen (lpData->text), & Size);
		// PJO : 5/5/2018 : GetWindowRect(NULL,..) does not return anymore size of desktop 
		hParentWnd = GetParent(hwndDlg);
		if (hParentWnd == NULL)   
			  hParentWnd = GetDesktopWindow();
		GetWindowRect (hParentWnd, & sParentRect);

		// Create a passive multi line edit control to display the text
		res->hwndText = CreateWindow  ( "Edit",  // Predefined class; Unicode assumed 
								"", 
								WS_VISIBLE | WS_CHILD | ES_READONLY | ES_MULTILINE | WS_DISABLED,  // Styles 
								TEXT_X_POSITION, 
								TEXT_Y_POSITION,
								Size.cx, 
								Size.cy,
								hwndDlg,     // Parent window
								NULL,       // No menu.
								GetWindowInstance (hwndDlg), 
								NULL);      // Pointer not needed.


		// add margins for icon and buttons
		Size.cx += CX_MARGIN;
		Size.cy += CY_MARGIN  
					+ GetSystemMetrics(SM_CYFRAME) 
					+ GetSystemMetrics(SM_CYCAPTION) 
					+ GetSystemMetrics(SM_CXPADDEDBORDER);

		// get the low 4 bits to have the index on tMapping
		// for instance if we want a MB_OKCANCEL window, style will be 1 
		// and the resources to be created are in tMapping[1]
		res->style = lpData->uStyle & 0x0000F;

		// check that all buttons will fit in the window, make it larger if needed
		// no needed for Size.cy since the height has already been take into account
		xButtonsWidth = (BUTTON_LENGTH+BUTTON_SPACE) * tMapping[res->style].nButtons + BUTTON_SPACE;
		Size.cx = max (Size.cx, xButtonsWidth);

		// Create the buttons and center them
		for (Ark=0 ; Ark < tMapping[res->style].nButtons ; Ark++)
		{
			int DefButtonStyle = HIBYTE(lpData->uStyle) == Ark ? BS_DEFPUSHBUTTON : 0;
			res->hwndButton[Ark] = CreateWindow  ( "BUTTON",  // Predefined class 
												szButtonText [tMapping[res->style].tButtons[Ark]],      // Button text 
												WS_TABSTOP | WS_VISIBLE | WS_CHILD | DefButtonStyle,         // Styles 
												Size.cx/2 - (tMapping[res->style].nButtons-2*Ark)*(BUTTON_LENGTH+BUTTON_SPACE)/2, 
												Size.cy - (BUTTON_LENGTH+BUTTON_SPACE), 
												BUTTON_LENGTH, 
												BUTTON_HEIGHT,
												hwndDlg,     // Parent window
												NULL,       // No menu.
												GetWindowInstance (hwndDlg), 
												NULL);      // Pointer not needed.
		} // create all buttons

	    // Set focus on the default push button
		if ( (nDefButtonIdx = HIBYTE(lpData->uStyle)) < tMapping[res->style].nButtons)
			SetFocus (res->hwndButton[nDefButtonIdx]);
		
		// now center the main window 
		x = sParentRect.left + (sParentRect.right - sParentRect.left - Size.cx) / 2 ;
		y = sParentRect.top + (sParentRect.bottom - sParentRect.top - Size.cy) / 2  ;
		SetWindowPos (hwndDlg, NULL, x,  y, Size.cx, Size.cy, SWP_NOZORDER | SWP_NOACTIVATE );

		// send caption and inner text string
		SetWindowText (hwndDlg, lpData->title);
		SetWindowText (res->hwndText, lpData->text);

		// if a system icon has been specified load it and keep its handler until dialog is closed
		if ( (nIconIdx = (lpData->uStyle >> 4) & 0x0F) < SizeOfTab (tIcon))
			// res->hIcon = LoadIcon (myGetWindowInstance (hwndDlg), tIcon[nIconIdx]);
			res->hIcon = LoadIcon (NULL, tIcon[nIconIdx]);

		// inherit parent icon
		// try different methods
		if ( 
			    (hParentIcon = (HICON) SendMessage (GetParent(hwndDlg), WM_GETICON, ICON_SMALL, 0)) != NULL
		     || (hParentIcon = (HICON) SendMessage (GetParent(hwndDlg), WM_GETICON, ICON_BIG, 0)) != NULL
			 || (hParentIcon = (HICON) GetClassLongPtr (GetParent(hwndDlg), GCLP_HICONSM)) != NULL
			 || (hParentIcon = (HICON) GetClassLongPtr (GetParent(hwndDlg), GCLP_HICON)) != NULL 
			)
			SendMessage (hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM) hParentIcon);

return FALSE;
} // OnInitDialog


// releases controls opened in InitDialog
static int ReleaseResources (HWND hwndDlg, HWND hButtonWnd, const struct S_myMsgBoxResources *res)
{
int Ark;
int Rc=-1;
		// release our icon
		if (res->hIcon!=NULL)  DestroyIcon (res->hIcon);
		// destroy edittext control
		if (res->hwndText)     DestroyWindow (res->hwndText);
		// while we destroy the buttons
		//    match the buttons windows handler with lParam to determine the one which was pressed
		//    using tMapping array set the return value
		for (Ark=0 ;   Ark < tMapping[myMsgBoxResources.style].nButtons  ;  Ark++)
		{
			if (res->hwndButton[Ark] == hButtonWnd)  
				Rc = tMapping[res->style].tButtons[Ark];
			DestroyWindow (res->hwndButton[Ark]);
		}
return Rc;
} // ReleaseResources


// --------------------
// The Windows callback
// --------------------
BOOL CALLBACK myMsgBoxCbk (HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
HDC  hDC;
int  Rc;

    switch (message) 
    { 
        case WM_INITDIALOG : 
           return OnInitDialog (hwndDlg, (struct sParam *) lParam, & myMsgBoxResources); 
 
		case WM_PAINT : 		
			// text is already managed by the control, 
			// but icon has to be drawn manually
			hDC = GetDC (hwndDlg);
			Rc = DrawIcon (hDC, ICON_X_POSITION, ICON_Y_POSITION, myMsgBoxResources.hIcon);
			ReleaseDC (hwndDlg, hDC);
			return FALSE;

    	case WM_COMMAND:
			Rc = ReleaseResources (hwndDlg, (HWND) lParam, & myMsgBoxResources);
			EndDialog (hwndDlg, Rc);
			return TRUE;

		case WM_CLOSE :
			ReleaseResources (hwndDlg, NULL, & myMsgBoxResources);
			EndDialog (hwndDlg, IDCANCEL);
			return TRUE;
	} 
return FALSE;
} // myMsgBox


//
// The custom messagebox API
//
int __cdecl CMsgBox(
    HWND hParentWnd,          // handle of owner window
    LPCTSTR lpText,     // address of text in message box
    LPCTSTR lpCaption,  // address of title of message box
    UINT uType,         // style of message box
    ...                 // follow lpText
   )
{
char szBuf [512];
va_list marker;
struct sParam cParam;
int       Rc;
HINSTANCE hInstance = GetWindowInstance (hParentWnd);

	// format the text string
    va_start (marker, uType);     // Initialize variable arguments.
    Rc = StringCbVPrintf (szBuf, sizeof szBuf, lpText, marker);
    va_end ( marker);
	if (Rc != S_OK)  return -1;

	// check some boundaries
	if ( (uType & 0x000F) >SizeOfTab (tMapping)) return -1;

	// save parameters in the stack
	cParam.uStyle = uType;
	cParam.text = szBuf;
	cParam.title = lpCaption;
	
    Rc = DialogBoxIndirectParam ( hInstance, 
								  (LPCDLGTEMPLATEW) & sEmptyDialogBox, 
								  hParentWnd,
                                  (DLGPROC) myMsgBoxCbk, 
								  (LPARAM) & cParam);
return Rc;
} /* OpenNewDialogBox */
