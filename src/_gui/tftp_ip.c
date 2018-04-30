//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Mai 98 Ph.jounin
// File tftp_id.c:   retrieves IP address
//
// released under artistic license
//
//////////////////////////////////////////////////////


#include "headers.h"

/////////////////////////////
// The background window
//
long CALLBACK TftpAddIPProc (HWND hWnd, UINT message, WPARAM wParam, LONG lParam)
{
static char buf[MAXGETHOSTSTRUCT];

  switch (message)
  {

    /////////////////////////
    // This backround window and this quirk AsyncGetHostByName call
    // is here as workaround for a Windows98 bug :
    // If the address has been returned by DHCP and the DNS server is down/unreachable
    // the gethostbyname returns  "Host not found, or SERVERFAIL"
    // This error happens (happened ?) frequently when connected by modem..
    // I have kept this asynchronous background window, since it allow the
    // IP address list to be refreshed every 20 seconds.
    ////////////////////////
    case WM_TFTP_GETIP :
        {char szName [256];          
          if (     gethostname (szName, sizeof szName)==SOCKET_ERROR 
               ||  WSAAsyncGetHostByName (hWnd, WM_IPADDRESS, szName, buf, sizeof buf) ==0 )
                        PostMessage (GetParent (hWnd), WM_ADD_IP_CB, 0, 0);
        }
        break;

    case WM_IPADDRESS :
        // Post now the address to the main window
        PostMessage (GetParent (hWnd), WM_ADD_IP_CB, 0,
                    WSAGETASYNCERROR(lParam)==0 ?  (LPARAM) buf : 0);
        // PostMessage (hWnd, WM_CLOSE, 0, 0);
        // refresh list evry 20 seconds
        SetTimer (hWnd, WM_TFTP_GETIP, 20000, 0);
        break;

       case WM_TIMER :
            KillTimer(hWnd, wParam);
            PostMessage (hWnd, wParam, 0, 0);
            break;
  
  }

return DefWindowProc (hWnd, message, wParam, lParam);

} // TftpAddIPProc



