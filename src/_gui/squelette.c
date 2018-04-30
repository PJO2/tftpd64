//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Aout 03 Ph.jounin
// File squelette.c:   Gestion du protocole SNTP
//
// released under artistic license
//
//////////////////////////////////////////////////////


#include "headers.h"

#define SNTP_PORT  119


///////////////////////////////////////////////////////
// Création de la socket SNTP
///////////////////////////////////////////////////////
SOCKET SntpBindSocket (HWND hWnd)
{
struct sockaddr_in SockAddr;
SOCKET             sSntpListenSocket;
int    Rc;
struct servent *lpServEnt;


   sSntpListenSocket = socket (AF_INET, SOCK_DGRAM, 0);
   if (sSntpListenSocket == INVALID_SOCKET)
   {
      MY_ERROR ("Error : Can't create socket");
      return INVALID_SOCKET;
   }
   memset (& SockAddr, 0, sizeof SockAddr);
   SockAddr.sin_family = AF_INET;
   lpServEnt = getservbyname ("sntp", "udp") ;
   SockAddr.sin_port =  (lpServEnt != NULL) ?  lpServEnt->s_port : htons (SNTP_PORT);
   // bind the socket to all active interfaces
   SockAddr.sin_addr.s_addr = INADDR_ANY ;
   Rc = bind (sSntpListenSocket, (struct sockaddr *) & SockAddr, sizeof SockAddr);
   if (Rc == INVALID_SOCKET)
   {char szTxt[256];
       wsprintf (szTxt, "Error Can't bind the SNTP port!\nEither you do not have necessary privilege or a SYSLOG daemon is already started\n\nbind returns error %d, GetLastError %d",
                 Rc, GetLastError());
       MY_ERROR (szTxt);
       return INVALID_SOCKET;
   }
return sSntpListenSocket;
} // SntpBindSocket 


///////////////////////////////////////////////////////
// Gestion des boutons
///////////////////////////////////////////////////////
static int Handle_VM_Command (HWND hWnd, WPARAM wParam, LPARAM lParam)
{
int     wItem = (int) LOWORD (wParam);
HWND    hListBox = GetDlgItem (GetParent (hWnd), IDC_LB_SYSLOG);

   switch ( wItem )
   {
 case 0 :
       break;
   }

return FALSE;
} // Handle_VM_Command




/////////////////////////////
// Fenetre Background gestion des appels TCP
//
long CALLBACK SntpDProc (HWND hWnd, UINT message, WPARAM wParam, LONG lParam)
{
static SOCKET   sSntpListenSocket=INVALID_SOCKET;
static char     szSyslogBuf[SYSLOG_MAXMSG+1]; // Buffer
int             Rc, Ark;
HWND            hListBox = GetDlgItem (GetParent (hWnd), IDC_LB_SYSLOG);

  switch (message)
  {
     case WM_SYSLOG_INIT :
        sSntpListenSocket  = SntpBindSocket (hWnd);
           WSAAsyncSelect (sSntpListenSocket, hWnd, WM_SYSLOG_MSG, FD_READ);
           break;

    /////////////////////////
    // Arrivée message

    case WM_SYSLOG_MSG :
        Rc = recv (sSntpListenSocket, szSyslogBuf, sizeof szSyslogBuf, 0);
       break;

    /////////////////////////
    // Message Windows
    case WM_CLOSE :
         break;
    case WM_COMMAND :
          Handle_VM_Command (hWnd, wParam, lParam);
          break;
    case WM_TIMER :
         KillTimer(hWnd, wParam);
         PostMessage (hWnd, wParam, 0, (LPARAM) -1);    // pour pas confondre
         break;
  }
return DefWindowProc (hWnd, message, wParam, lParam);
} // SntpDProc
