//////////////////////////
// Useful Window proc
// released under european public license (see license.txt)
//////////////////////////


// center a window
#define CCW_VISIBLE 0x0010  // window should be inside the physical screen
#define CCW_INSIDE  0x0020  // fails if child window larger than its parent

BOOL CenterChildWindow (
    HWND hChildWnd,         // Wnd which have to be centerd
    int uType               // options
    );
