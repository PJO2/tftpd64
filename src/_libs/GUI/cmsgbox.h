
// --------------------------------------------
// A plain-C replacement for MessageBox command
//
// myMsgBox opens a messagebox-like modal window at the center of the parent window
//
// by Ph. Jounin Oct 2015
// release under GPLv2 license
// --------------------------------------------


int __cdecl CMsgBox(
    HWND hParentWnd,          // handle of owner window
    LPCTSTR lpText,     // address of text in message box
    LPCTSTR lpCaption,  // address of title of message box
    UINT uType,         // style of message box
    ...                 // follow lpText
   );
