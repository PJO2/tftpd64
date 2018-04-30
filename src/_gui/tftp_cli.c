//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Mai 98 Ph.jounin
// File tftp_cli.c:   TFTP client
//
// released under artistic license
//
//////////////////////////////////////////////////////


#include "headers.h"
#include <commdlg.h>


// some shortcurts
#define ISDLG_CHECKED(hWnd,Ctrl) \
  (SendDlgItemMessage (hWnd, Ctrl, BM_GETCHECK, 0, 0) == BST_CHECKED)

#define TFTP_CLIENT_SEMAPHORE "Tftpd32ClientSemaphore"
#define ID_CLIENT_TRANSFER      0x216       // timer identifier
#define DEFAULT_TFTP_PORT  "69"

static HWND      hTftpClientWnd;
static HANDLE    hTftpClientSemaphore;

////////////////////////////////////////
// something similar to the server structure
////////////////////////////////////////

static struct S_TftpClient
{
    unsigned char opcode;           // TFTP_RRQ ou TFTP_WRQ
    char szFile [256];              // nom du fichier à transférer
    char szDestFile [256];          // nom du fichier en remote
    char szHost [256];              // adresse du serveur
    unsigned char BufSnd [MAXPKTSIZE];     // dernier datagram émis
    unsigned char BufRcv [MAXPKTSIZE];     // dernier datagram reçu
	SOCKADDR_STORAGE  saFrom;      // Server address
    int           nPort;           // port pour la connexion (0 -> default)
    DWORD         nToSend;         // nombre de caractères à envoyer
    DWORD         nRcvd;           // nombre de caractères reçus
    SOCKET        s;                // socket de communication
    BOOL          bConnected;       // socket "connectée"
    HANDLE        hFile;            // Handler du fichier local
    DWORD         nCount;           // nombre de block émis/reçus
    DWORD         nRetransmit;      // nombre de retransmissions
    DWORD         dwTimeout;        // Timeout en 1000e de secondes
    time_t        StartTime;        // Horodatage
    time_t        dLastUpdate;
    DWORD         dwFileSize;
    DWORD         nTotRetrans;       // retransmissions complètes
    BOOL          bBreak;           // break has been selected
    DWORD         nPktSize;
    BOOL          bMultiFile;        // several files are to be transferred
    DWORD         dwMultiFileBlk;    // total count
    DWORD         dwMultiFile;       // total count

    struct S_Trf_MD5 m;
} // struct S_tftpClient
sTC;

enum { TFTP_NOTCONNECTED = FALSE, TFTP_CONNECTED };


// Block size in combo box
static struct S_BlkSize
{
  char  szBlkSize [8];
} 
tBlkSize [] = 
{ 
  {"Default"}, {"128"},   {"512"},   {"1024"}, {"1428"},  
  {"2048"}, {"4096"}, {"8192"}, {"16384"}, {"32768"},
}; 







///////////////////////////////////////////////////////
// End of transfer : 
///////////////////////////////////////////////////////
static void StopTransfer (void)
{
    // stop transfer
    WSAAsyncSelect (sTC.s, hTftpClientWnd, 0, 0);
    KillTimer(hTftpClientWnd, WM_CLIENT_DATA);
    KillTimer(hTftpClientWnd, WM_CLIENT_ACK);
    // free resource
    closesocket (sTC.s);      sTC.s = INVALID_SOCKET;
    CloseHandle (sTC.hFile);  sTC.hFile = INVALID_HANDLE_VALUE;
    sTC.bConnected = TFTP_NOTCONNECTED;
    // reinit buttons
    EnableWindow (GetDlgItem (GetParent(hTftpClientWnd), IDC_CLIENT_SEND_BUTTON), TRUE);
    EnableWindow (GetDlgItem (GetParent(hTftpClientWnd), IDC_CLIENT_GET_BUTTON), TRUE);
    EnableWindow (GetDlgItem (GetParent(hTftpClientWnd), IDC_CLIENT_BREAK_BUTTON), FALSE);
    SetDlgItemText (GetParent(hTftpClientWnd), IDC_CLIENT_BLOCK, "");
    SendDlgItemMessage (GetParent(hTftpClientWnd), IDC_CLIENT_PROGRESS, PBM_SETPOS, 0, 0);

} // StopTransfer



int BadEndOfTransfer (const char *szFmt, ...)
{
char     szBuf [512];
va_list  marker;
struct  tftphdr *tp;

 // send NAK to server
   tp = (struct  tftphdr *) sTC.BufSnd;
   tp->th_opcode = htons((u_short)TFTP_ERROR);
   tp->th_code = htons(EUNDEF);
   send (sTC.s, (char *) tp, TFTP_DATA_HEADERSIZE, 0);

    StopTransfer ();
    // display message
    va_start( marker, szFmt );     /* Initialize variable arguments. */
    wvsprintf (szBuf, szFmt, marker);
    CMsgBox (hTftpClientWnd, szBuf, APPLICATION, MB_OK | MB_ICONERROR);

   // Semaphore released
    ReleaseSemaphore (hTftpClientSemaphore, 1, NULL);

   if (sTC.opcode==TFTP_RRQ  &&  sTC.nRcvd!=0)   DeleteFile (sTC.szFile);
return FALSE;
} // BadEndOfTransfer


int TransferOK (time_t dNow)
{
char     szBuf [256];
    StopTransfer ();    // free resources

     // close MD5 computation
     MD5Final (sTC.m.ident, & sTC.m.ctx);

    // Semaphore released
    ReleaseSemaphore (hTftpClientSemaphore, 1, NULL);

    if (! sTC.bMultiFile)  
    {char szMD5 [33];
     int Ark;
     for (Ark=0 ; Ark<16; Ark++)
         wsprintf (szMD5 + 2*Ark, "%02x", sTC.m.ident[Ark]);
      wsprintf (szBuf, "%d block%s transferred in %d second%s\n %d block%s retransmitted\nMD5: %s",
                       sTC.nCount, // + (sTC.opcode==TFTP_RRQ ? 1 : 0),
                       PLURAL (sTC.nCount), // + (sTC.opcode==TFTP_RRQ ? 1 : 0)),
                       (int) (dNow-sTC.StartTime),
                       PLURAL (dNow-sTC.StartTime),
                       sTC.nTotRetrans,
                       PLURAL (sTC.nTotRetrans),
                       szMD5
            );
        CMsgBox (hTftpClientWnd, szBuf, APPLICATION, MB_OK | MB_ICONINFORMATION);
   } // one transfer --> display stats
 else 
  {
      sTC.dwMultiFileBlk += sTC.nCount;
      sTC.dwMultiFile++;
 }


return TRUE;
} // TransferOK


static void PopulateXRQPacket (char *szBlkSize, BOOL bFullPath)
{
struct  tftphdr *tp;
char *p; // beginning of file

    ///////////////////////////////
    // prepare the data packet
    // option tsize is added at the end of packet - possibly ignored by server
    tp = (struct    tftphdr *) sTC.BufSnd;
    tp->th_opcode = htons (sTC.opcode);
    sTC.nToSend = sizeof (short);

    if (sTC.szDestFile[0] != 0)
           p = sTC.szDestFile;
    else  // get local file
    {
       if (! bFullPath)
       {
          // search for the right '\'
          for (p = sTC.szFile + lstrlen (sTC.szFile) ; p>=sTC.szFile && *p!='\\' ; p--);
          p++;
       }
       else  p = sTC.szFile;
    } 
    lstrcpy (& sTC.BufSnd [sTC.nToSend], p),          sTC.nToSend += lstrlen (p) + 1;
    lstrcpy (& sTC.BufSnd [sTC.nToSend], "octet"),    sTC.nToSend += sizeof "octet";
   // szBlkSize not set to Default..
    if ( isdigit (szBlkSize[0]) )
    {
       lstrcpy (& sTC.BufSnd [sTC.nToSend], "blksize"),    sTC.nToSend += sizeof "blksize";
       lstrcpy (& sTC.BufSnd [sTC.nToSend], szBlkSize);    sTC.nToSend += lstrlen (szBlkSize)+1;
    }
	if ( sGuiSettings.bPortOption )
	{
       lstrcpy (& sTC.BufSnd [sTC.nToSend], TFTP_OPT_PORT),    sTC.nToSend += sizeof TFTP_OPT_PORT;
       lstrcpy (& sTC.BufSnd [sTC.nToSend], "0");              sTC.nToSend += sizeof "0";
	}

	// tsize is always set
   lstrcpy (& sTC.BufSnd [sTC.nToSend], "tsize"),    sTC.nToSend += sizeof "tsize";
   sTC.nToSend += 1+wsprintf(& sTC.BufSnd [sTC.nToSend], "%d", sTC.dwFileSize);
} // PopulateXRQPacket


///////////////////////////////////////////////////////
// Build the send the first message (RRQ or WRQ)
// All Data are in sTC struct
///////////////////////////////////////////////////////
static BOOL TftpSendConnect (char *szBlkSize, BOOL bFullPath)
{
ADDRINFO Hints, *ai;
int              Rc;
char             szPort[NI_MAXSERV];

	// preparte a data packet (either RRQ or WRQ)
	PopulateXRQPacket (szBlkSize, bFullPath);

    // resolve host name
	memset (& Hints, 0, sizeof Hints);
	Hints.ai_family   = AF_UNSPEC;
	Hints.ai_socktype = SOCK_DGRAM;
	Hints.ai_protocol = IPPROTO_UDP;

	// first use port given as parameter, 
	if (sTC.nPort==0)
		  lstrcpy (szPort, "tftp");
	else
	{
		 wsprintf (szPort, "%d", sTC.nPort), 	
		 Hints.ai_flags    = AI_NUMERICSERV;
	}
    Rc = getaddrinfo (sTC.szHost, szPort, & Hints, &ai );
	// last chance : use default tftp port 69
	if (Rc==WSASERVICE_NOT_FOUND)
	{
	    Hints.ai_flags    = AI_NUMERICSERV;
		Rc = getaddrinfo (sTC.szHost, DEFAULT_TFTP_PORT, & Hints, &ai );
	}
	if (Rc!=0)
        return BadEndOfTransfer ("Host is unknown or invalid. Error %d", GetLastError());
    sTC.s = socket( ai->ai_family,ai->ai_socktype, ai->ai_protocol ) ;
	if (sTC.s == INVALID_SOCKET)
	{
		freeaddrinfo (ai);
        return BadEndOfTransfer ( "Can't create client socket.\nError code %d (%s)", 
			                      WSAGetLastError (), LastErrorText());
	}
    
    // since remote port will change do not use either bind or connect 
	Rc = sendto (sTC.s, sTC.BufSnd, sTC.nToSend , 0, ai->ai_addr, ai->ai_addrlen );
	if (Rc == SOCKET_ERROR)
	{
		freeaddrinfo (ai);
        return BadEndOfTransfer  ("can not send data packet.\n%s\nError code %d (%s)", 
                                  "Tftp server may have been stopped",
                                  WSAGetLastError (), LastErrorText());
	}
	freeaddrinfo (ai);
return TRUE;
} // TftpSendConnect


///////////////////////////////////////////////////////
// OACK msg processing
///////////////////////////////////////////////////////

static SOCKET TftpProcessOACK (void)
{
struct  tftphdr *tpr, *tps;
char            *pOpt, *pValue;       // couples Option/Valeur
int              Rc;

    tpr = (struct    tftphdr *) sTC.BufRcv;
    tps = (struct    tftphdr *) sTC.BufSnd;

    // parse options
    pOpt = tpr->th_stuff ;  
    while (pOpt - tpr->th_stuff < TFTP_SEGSIZE  && *pOpt!=0 ) 
    {
    // find value string (next word) do not use strlen since string is possibly not 0 terminated
    for (pValue = pOpt  ; pValue - tpr->th_stuff < TFTP_SEGSIZE   &&   *pValue != 0 ; pValue++);
    pValue++;
    if (pValue - tpr->th_stuff == TFTP_SEGSIZE)  return FALSE;

       if (sTC.opcode==TFTP_RRQ  &&  IS_OPT (pOpt, TFTP_OPT_TSIZE))
       {    // prendre la valeur et envoyer un ACK du block #0
          sTC.dwFileSize = atoi (pValue);
          tps->th_opcode = htons (TFTP_ACK);
          tps->th_block = htons (0);
          SetTimer (hTftpClientWnd, WM_CLIENT_DATA, sTC.dwTimeout, NULL);
          send (sTC.s, (char *) tps, TFTP_DATA_HEADERSIZE, 0);
       } // option Tsize

       if (IS_OPT (pOpt, TFTP_OPT_BLKSIZE))
       {    // prendre la valeur 
         sTC.nPktSize = atoi (pValue);
#ifdef JKKLLKJLKJLKLKJKLLKJ
      // If read -> send a ack of block #0
           if (sTC.opcode==TFTP_RRQ)
      {
          tps->th_opcode = htons (TFTP_ACK);
         tps->th_block = htons (0);
         SetTimer (hTftpClientWnd, WM_CLIENT_DATA, sTC.dwTimeout, NULL);
            send (sTC.s, (char *) tps, TFTP_DATA_HEADERSIZE, 0);
       }
#endif
       } // option BlkSize

       if (IS_OPT (pOpt, TFTP_OPT_PORT))
	   {char szServ[NI_MAXSERV];
		  getnameinfo ( (LPSOCKADDR) & sTC.saFrom, sizeof sTC.saFrom, 
					    NULL, 0, szServ, sizeof szServ, NI_NUMERICSERV);
		 LogToMonitor ("Port value is %s should be changed to %s", szServ, pValue);

         switch (sTC.saFrom.ss_family)
		 {
		     case AF_INET :  ( (struct sockaddr_in *) (&sTC.saFrom) )->sin_port = htons (atoi (pValue));
				              break;
		     case AF_INET6 : ( (struct sockaddr_in6 *) (&sTC.saFrom) )->sin6_port = htons (atoi (pValue));
				              break;
		 }
		 Rc = connect (sTC.s, (struct sockaddr *) & sTC.saFrom, sizeof sTC.saFrom);
		 LogToMonitor ("re-connect returns %d (%d)", Rc, GetLastError ());
	   }

      pOpt = pValue + lstrlen (pValue) + 1; 
      
    }     // next option
return TRUE;
} // TftpProcessOACK



///////////////////////////////////////////////////////
// Recv UDP message
///////////////////////////////////////////////////////
BOOL UdpRecv (void)
{
int Rc, nDummy ;

   sTC.nRcvd = 0;
   nDummy = sizeof  sTC.saFrom;
   // use recvfrom in order to fill the saFrom field
   if ( sTC.bConnected==TFTP_NOTCONNECTED )
            Rc = recvfrom (sTC.s, sTC.BufRcv, sizeof sTC.BufRcv, 0,
						  (struct sockaddr *) & sTC.saFrom, & nDummy);
   else     Rc = recv (sTC.s, sTC.BufRcv, sizeof sTC.BufRcv, 0);
   if (Rc == SOCKET_ERROR)
   {
       return FALSE;
   }
   // now it is safe to memorize the server address into socket structure
   if (sTC.bConnected == TFTP_NOTCONNECTED)
   {
	    connect (sTC.s, (struct sockaddr *) & sTC.saFrom, sizeof sTC.saFrom);
        sTC.bConnected = TFTP_CONNECTED;
   }
  sTC.nRcvd = Rc;
return TRUE;
} // UdRecv


///////////////////////////////////////////////////////
// open the file
///////////////////////////////////////////////////////
BOOL TftpCliOpenFile (void)
{

    // creation fichier ou lecture
    switch (sTC.opcode)
    {
       case TFTP_WRQ :
            sTC.hFile = CreateFile ( sTC.szFile,
                                 GENERIC_READ,
                                 FILE_SHARE_READ,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_SEQUENTIAL_SCAN ,
                                 NULL );
            if (sTC.hFile == INVALID_HANDLE_VALUE)
                 return BadEndOfTransfer ("Error opening file %s for reading\nreturn code is %d (%s)", sTC.szFile, GetLastError (), LastErrorText() );
            sTC.dwFileSize = GetFileSize (sTC.hFile, NULL);
            break;

        case TFTP_RRQ :
           sTC.hFile = CreateFile ( sTC.szFile,
                                 GENERIC_WRITE,
                                 0,
                                 NULL,
                                 CREATE_NEW,
                                 FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_SEQUENTIAL_SCAN ,
                                 NULL );
            if (    sTC.hFile==INVALID_HANDLE_VALUE
                 && GetLastError()==ERROR_FILE_EXISTS
                 && CMsgBox (hTftpClientWnd, "File exists, overwrite it ?", APPLICATION, MB_YESNOCANCEL) == IDYES )
            {
               sTC.hFile = CreateFile ( sTC.szFile,
                                     GENERIC_WRITE,
                                     0,
                                     NULL,
                                     CREATE_ALWAYS,
                                     FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_SEQUENTIAL_SCAN ,
                                     NULL );
               if (sTC.hFile == INVALID_HANDLE_VALUE)
                  return BadEndOfTransfer ("Error opening file %s for writing\nreturn code is %d (%s)", sTC.szFile, GetLastError (), LastErrorText() );
          }
             break;
        } //switch opcode

return sTC.hFile!=INVALID_HANDLE_VALUE;
} // TftpCliOpenFile



///////////////////////////////////////////////////////
// Display statistics
///////////////////////////////////////////////////////
void static Statistics (time_t dNow)
{
char sz [64];
  if (dNow != sTC.dLastUpdate)
  {
       wsprintf (sz, "block #%d", sTC.nCount);
       SetDlgItemText (GetParent(hTftpClientWnd), IDC_CLIENT_BLOCK, sz);
       sTC.dLastUpdate = dNow;
       if (sTC.dwFileSize/100 != 0)
               SendDlgItemMessage (GetParent(hTftpClientWnd), IDC_CLIENT_PROGRESS, PBM_SETPOS, (sTC.nCount*sTC.nPktSize)/(sTC.dwFileSize/100), 0);
  } // time to update
} // Statistics


///////////////////////////////////////////////////////
// WM_Command
///////////////////////////////////////////////////////
static int Handle_VM_Command (HWND hWnd, WPARAM wParam, LPARAM lParam)
{
int     wItem = (int) LOWORD (wParam);
HWND  hParentWnd = GetParent (hWnd);
char  szCBTxt [128];
OPENFILENAME OpenFileName; 
char szCurDir[MAX_PATH];

   switch ( wItem )
   {
      case IDC_CLIENT_SEND_BUTTON :
      case IDC_CLIENT_GET_BUTTON  :          
          // fill sTC structure
          time (& sTC.StartTime);
          sTC.dwTimeout = TFTP_TIMEOUT * 1000;
          sTC.bBreak = FALSE;
          // clear file stats
          sTC.nToSend = sTC.nRcvd = sTC.nCount = sTC.nRetransmit = sTC.nPktSize = 0;
          sTC.s = INVALID_SOCKET;
          sTC.hFile = INVALID_HANDLE_VALUE;
          sTC.dwFileSize = 0;
          if (! sTC.bMultiFile)  sTC.nTotRetrans = 0;
          // init MD5 computation
          MD5Init (& sTC.m.ctx);

          // récupérer les valeurs
              sTC.opcode = wItem==IDC_CLIENT_GET_BUTTON ? TFTP_RRQ : TFTP_WRQ;
          GetDlgItemText (hParentWnd, IDC_CLIENT_HOST, sTC.szHost, sizeof sTC.szHost);
          GetDlgItemText (hParentWnd, IDC_CLIENT_LOCALFILE, sTC.szFile, sizeof sTC.szFile);
          GetDlgItemText (hParentWnd, IDC_CLIENT_REMOTEFILE, sTC.szDestFile, sizeof sTC.szDestFile);
		  if (strchr (sTC.szFile, '%') != NULL  ||  strchr (sTC.szDestFile, '%') != NULL)
		  {
                CMsgBox (hWnd, "Character %% not supported by Tftpd32", APPLICATION, MB_OK | MB_ICONHAND);
				break;
		  }

          ComboBox_GetText (GetDlgItem (hParentWnd, IDC_CB_DATASEG), szCBTxt, sizeof szCBTxt);
 
         // STC.nPort set to 0 on error
         sTC.nPort = GetDlgItemInt (hParentWnd, IDC_CLIENT_PORT, NULL, FALSE);
         // datagram size. May be modified by an OACK message
         sTC.nPktSize = TFTP_SEGSIZE; 

          if (sTC.szHost[0]==0 || sTC.szFile[0]==0)
                CMsgBox (hWnd, "Fields Host and File should be given.", APPLICATION, MB_OK | MB_ICONHAND);
          else
              if (    TftpCliOpenFile ()   
               &&  TftpSendConnect (szCBTxt, ISDLG_CHECKED(hParentWnd,IDC_CLIENT_FULL_PATH))  )
              {int Rc;
                  EnableWindow (GetDlgItem (hParentWnd, IDC_CLIENT_SEND_BUTTON), FALSE);
                  EnableWindow (GetDlgItem (hParentWnd, IDC_CLIENT_GET_BUTTON), FALSE);
                  EnableWindow (GetDlgItem (hParentWnd, IDC_CLIENT_BREAK_BUTTON), TRUE);
                  SetTimer (hWnd, wItem==IDC_CLIENT_GET_BUTTON ? WM_CLIENT_DATA : WM_CLIENT_ACK, sTC.dwTimeout, NULL);
                  Rc = WSAAsyncSelect (sTC.s, hWnd, wItem==IDC_CLIENT_GET_BUTTON ? WM_CLIENT_DATA : WM_CLIENT_ACK, FD_READ);
				  Rc = GetLastError ();
              }
          else   CloseHandle (sTC.hFile);
          break;

      case IDC_CLIENT_BREAK_BUTTON  :
         sTC.bBreak = TRUE;
          WSAAsyncSelect (sTC.s, hWnd, 0, 0);
          BadEndOfTransfer ("Transfer cancelled by user");
          break;


// Code not implemented since OpenFileName dialog box change current directory
      case IDC_CLIENT_BROWSE :
        GetCurrentDirectory (sizeof szCurDir, szCurDir);
        memset (& OpenFileName, 0, sizeof OpenFileName);
        OpenFileName.lStructSize       = sizeof (OPENFILENAME);
        OpenFileName.hwndOwner         = hWnd;
        OpenFileName.hInstance         = NULL;
        OpenFileName.lpstrFilter       = NULL;
        OpenFileName.lpstrCustomFilter = NULL;
        OpenFileName.nMaxCustFilter    = 0L;
        OpenFileName.nFilterIndex      = 0L;
        OpenFileName.lpstrFile         = sTC.szFile;
        OpenFileName.nMaxFile          = sizeof sTC.szFile;
        OpenFileName.lpstrInitialDir   = NULL;
        OpenFileName.lpstrTitle        = "Select file";
        OpenFileName.nFileOffset       = 0;
        OpenFileName.nFileExtension    = 0;
        OpenFileName.lpstrDefExt       = "*.*";
        OpenFileName.lCustData         = 0;
        OpenFileName.Flags             = 0;

       if (GetOpenFileName (&OpenFileName))  
             SetDlgItemText (hParentWnd, IDC_CLIENT_LOCALFILE, sTC.szFile);
       //else CMsgBox (hParentWnd, "GetOpenFailed raison %d", "bad", MB_OK, CommDlgExtendedError());
       SetCurrentDirectory (szCurDir);
       break;

   }
return FALSE;
} // Handle_VM_Command



/////////////////////////////
// Fenetre Background EditText
//
static WNDPROC fEditBoxProc;


// Hook designed to insert the name of the dragged file into the edit control
LRESULT CALLBACK TftpClientFileNameProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
char szFileName [MAX_PATH];
static HANDLE hDrop;
static int    nAmount, Ark; 

    switch (message)
   {
      case WM_TIMER :
                // is previous transfer terminated
             if (WaitForSingleObject (hTftpClientSemaphore, 0) == WAIT_OBJECT_0)
                {       
                   if (! sTC.bBreak && ++Ark<nAmount)
                 {
                      DragQueryFile (hDrop, Ark, szFileName, MAX_PATH-1);
                        SetWindowText (hWnd, szFileName);
                      // ignore directories (wait for next turn)
                     if ( (GetFileAttributes (szFileName) & FILE_ATTRIBUTE_DIRECTORY) == 0)
                            SendMessage (hTftpClientWnd, WM_COMMAND, IDC_CLIENT_SEND_BUTTON, 0);
                        else   ReleaseSemaphore (hTftpClientSemaphore, 1, NULL);
                   }
                  else    // mutiple transfer either stopped or finished
                 {
                      DragFinish (hDrop);
                        KillTimer (hWnd, wParam);
                      CMsgBox (hWnd, "%d file%s fully transferred in %d block%s and %d retransmission%s", "Tftpd32", MB_OK, 
                              sTC.dwMultiFile, PLURAL(sTC.dwMultiFile),
                              sTC.dwMultiFileBlk, PLURAL(sTC.dwMultiFileBlk), 
                               sTC.nTotRetrans, PLURAL(sTC.nTotRetrans)); 
                       sTC.bBreak = sTC.bMultiFile = FALSE;
                       sTC.nTotRetrans = sTC.dwMultiFileBlk = sTC.dwMultiFile = 0 ; 
                      ReleaseSemaphore (hTftpClientSemaphore, 1, NULL);
                      break;
                 }
              }
              break;


       case WM_DROPFILES :
            hDrop = (HANDLE) wParam;
           nAmount = DragQueryFile (hDrop, -1, szFileName, MAX_PATH);
         // multi files mode : only if field Host has been filled
           if (   nAmount>=2 
             && GetDlgItemText (GetParent (hWnd), IDC_CLIENT_HOST, sTC.szHost, sizeof sTC.szHost) > 0)
          {
              if (CMsgBox (hWnd, 
                         "Upload (Put) %d files to host ?", 
                        "Tftpd32", 
                         MB_ICONQUESTION | MB_YESNOCANCEL,
                          nAmount)==IDYES)
             {
                      sTC.bMultiFile = TRUE;
                     sTC.dwMultiFileBlk=sTC.dwMultiFile=0 ; 
                        Ark=-1 ;

                      if (SetTimer (hWnd, ID_CLIENT_TRANSFER, 1000, NULL)==0)
                                CMsgBox (hWnd, "Create timer failed", "Tftpd32", MB_OK);

              } // User do not cancel MessageBox warning
         } // multi files have been dropped
         else
           {
              // only one file selected : just put its name into control 
                DragQueryFile (hDrop, 0, szFileName, MAX_PATH);
                SetWindowText (hWnd, szFileName);
              DragFinish (hDrop);
            }
          break;
     default : break;
           
   }
return CallWindowProc (fEditBoxProc, hWnd, message, wParam, lParam);
} // TftpClientFileNameProc



/////////////////////////////
// Fenetre Background gestion des appels TCP
//
LRESULT CALLBACK TftpClientProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
DWORD          nbWrt;
struct tftphdr *tpr, *tps;
time_t            dNow;
int             Ark;
HWND            hCBWnd, hParentWnd;

  time (&dNow);
  switch (message)
  {

    /////////////////////////
    // Message Windows

    case WM_INITCLIENT :
      // only one client transfer available
      hTftpClientSemaphore = CreateSemaphore(NULL, 1, 1, TFTP_CLIENT_SEMAPHORE);

         hTftpClientWnd = hWnd;
        hParentWnd = GetParent (hWnd);
         EnableWindow (GetDlgItem (hParentWnd, IDC_CLIENT_BREAK_BUTTON), FALSE);
         SetDlgItemText (hParentWnd, IDC_CLIENT_BLOCK, "");

       // populate combo with proposed block sizes
        hCBWnd = GetDlgItem (hParentWnd, IDC_CB_DATASEG);
      for (Ark=0 ;  Ark<SizeOfTab (tBlkSize); Ark++)
                ComboBox_AddString (hCBWnd, tBlkSize[Ark].szBlkSize);
       ComboBox_SetCurSel (hCBWnd, 0);

       // uncheck "Send full path to server"
      SendDlgItemMessage (hParentWnd, IDC_CLIENT_FULL_PATH, BM_SETCHECK, BST_UNCHECKED, 0);   

      // edittext IDC_CLIENT_FILE will accept dropped file (works only because it is on the top in Z-Order
       DragAcceptFiles (GetDlgItem (hParentWnd, IDC_CLIENT_LOCALFILE), TRUE);
          // fEditBoxProc = (WNDPROC) SetWindowLong (GetDlgItem (hParentWnd, IDC_CLIENT_LOCALFILE), GWL_WNDPROC, (LONG) TftpClientFileNameProc);
          fEditBoxProc = (WNDPROC) SetWindowLongPtr (GetDlgItem (hParentWnd, IDC_CLIENT_LOCALFILE), GWLP_WNDPROC, (LONG_PTR) TftpClientFileNameProc);
//    SetWindowLong (GetDlgItem (hParentWnd, IDC_CLIENT_FILE), GWL_USERDATA, (LONG) TftpClientFileNameProc);
    SetWindowLongPtr (GetDlgItem (hParentWnd, IDC_CLIENT_LOCALFILE), GWLP_USERDATA, (LONG_PTR) TftpClientFileNameProc);
         break;

    case WM_CLOSE :
LogToMonitor ("GUI: Closing Tftp Client");
         CloseHandle (hTftpClientSemaphore);
         CloseHandle (sTC.hFile);
         closesocket (sTC.s);
         break;

    case WM_COMMAND :
          Handle_VM_Command (hWnd, wParam, lParam);
          break;


    case WM_TIMER :
         KillTimer(hWnd, wParam);
         PostMessage (hWnd, wParam, 0, (LPARAM) -1);    // pour pas confondre
         break;

      //////////////////////
      // Download : fichier envoyé par le serveur
     case WM_CLIENT_DATA :
        // WSAAsyncSelect (sTC.s, hWnd, 0, 0);  A SUPPRIMER
        KillTimer(hWnd, wParam);
        if (sTC.bBreak) return FALSE;
        sTC.nRcvd = 0;
        // On est reveillé par un message reçu
        tpr = (struct tftphdr *) sTC.BufRcv;
        if (WSAGETSELECTEVENT(lParam) == FD_READ)
        {
            if (! UdpRecv ())
                return BadEndOfTransfer ("Error in Recv.\nError code is %d (%s)", WSAGetLastError (), LastErrorText() );
          // parcours des codes retours
          switch (htons (tpr->th_opcode))
            {
              case TFTP_ERROR :
                   return BadEndOfTransfer ("Server stops the transfer.\nError #%d: %s", htons (tpr->th_code), tpr->th_msg);
             case TFTP_OACK :
                    if (sTC.nCount==0) TftpProcessOACK ();
                     break;
                case TFTP_DATA :
                   // a data packet has been received. Check #block
                   if ( htons (tpr->th_block) == (unsigned short) (sTC.nCount+1) )
                    {
                      if (     !WriteFile (sTC.hFile, tpr->th_data, sTC.nRcvd - TFTP_DATA_HEADERSIZE, & nbWrt, NULL)
                          ||  sTC.nRcvd-TFTP_DATA_HEADERSIZE!=nbWrt)
                            return BadEndOfTransfer ("Error in writing file.\nError code is %d (%s)", GetLastError(), LastErrorText());
                       sTC.nCount++;
                       sTC.nRetransmit = 0;
                       // prepare Ack block
                       tps = (struct tftphdr *) sTC.BufSnd;
                       tps->th_opcode = htons (TFTP_ACK),  tps->th_block = htons ((unsigned short) sTC.nCount);
                       sTC.nToSend = TFTP_DATA_HEADERSIZE;
                       // compute MD5
                       MD5Update (& sTC.m.ctx, tpr->th_data, nbWrt);
                    }
                  break;
             default :
                 return BadEndOfTransfer ("Server sent illegal opcode %d", htons (tpr->th_opcode));
          } // switch opcode
        } // il y a un message à recevoir
        // La comparaison marche si le paquet est le bon ou une répétition du précédent message
        if ( htons (tpr->th_block) == (unsigned short) sTC.nCount)
        {
           if (sTC.nRetransmit)  sTC.nTotRetrans ++;
           if (sTC.nRetransmit++ > TFTP_RETRANSMIT)
                return BadEndOfTransfer ("Timeout waiting block #%d", sTC.nCount+1);
            send (sTC.s, sTC.BufSnd, sTC.nToSend, 0);
            SetTimer (hWnd, WM_CLIENT_DATA, sTC.dwTimeout, NULL);
        }
      // sTC.nRcvd ne peut être inférieur que si on a reçu un block
        if (  htons (tpr->th_opcode)==TFTP_DATA
          &&  sTC.nRcvd!=0
           && sTC.nRcvd < sTC.nPktSize + TFTP_DATA_HEADERSIZE)
                    return TransferOK (dNow);
      Statistics (dNow);
        break;

     ////////////////////////
    // Upload : server sends ACK
     case WM_CLIENT_ACK :
        KillTimer(hWnd, wParam);
       if (sTC.bBreak  ||  sTC.s==INVALID_SOCKET) return FALSE;
       
       // socket not closed by StopTransfer
        tpr = (struct tftphdr *) sTC.BufRcv;
        if ( WSAGETSELECTEVENT(lParam) == FD_READ )
        {
            if (! UdpRecv ())
                return BadEndOfTransfer ("Error in Recv.\nError code is %d (%s)", WSAGetLastError (), LastErrorText() );
            switch (htons (tpr->th_opcode))
            {
              case TFTP_ERROR :
                   return BadEndOfTransfer ("Server stops the transfer.\nError #%d: %s", htons (tpr->th_code), tpr->th_msg);

                case TFTP_OACK :
                    if (sTC.nCount!=0)  break; // ignore message
                   TftpProcessOACK ();
                    tpr->th_block = htons(0);  // pour passer en dessous
                   // Fall through

              case TFTP_ACK :
                    if ( htons (tpr->th_block) == (unsigned short) sTC.nCount )
                    {
                      // prepare Data block
                       tps = (struct tftphdr *) sTC.BufSnd;
                       if ( !ReadFile (sTC.hFile, tps->th_data, sTC.nPktSize, & sTC.nRcvd, NULL) )
                       {
                          if (sTC.nToSend == TFTP_DATA_HEADERSIZE + sTC.nPktSize)  // file was exactly N * PkSize
                                  return TransferOK (dNow);
                            else  return BadEndOfTransfer ("Error in reading file.\nError code is %d (%s)", GetLastError(), LastErrorText());
                       }
                       if (sTC.nRcvd == 0) // EOF 
                        {
                          if (sTC.nToSend < TFTP_DATA_HEADERSIZE + sTC.nPktSize) 
                                return TransferOK (dNow);
                      }
                      sTC.nCount++;
                      sTC.nRetransmit = 0;
                      tps->th_opcode = htons (TFTP_DATA),  tps->th_block = htons ((unsigned short) sTC.nCount);
                      sTC.nToSend = TFTP_DATA_HEADERSIZE + sTC.nRcvd;
                      // compute MD5
                      MD5Update (& sTC.m.ctx, tps->th_data, sTC.nRcvd);
                    }
                  break;
             default :
                   return BadEndOfTransfer ("Server sent illegal opcode %d", htons (tpr->th_opcode));
            } // switch opcode
        } // il y a un message à recevoir

        // Timeout or ack of previous block
        else if ( htons (tpr->th_block) == (unsigned short) (sTC.nCount-1) )
        {
           if (sTC.nRetransmit)  sTC.nTotRetrans ++;
           if (sTC.nRetransmit++ > TFTP_RETRANSMIT)
                return BadEndOfTransfer ("Timeout while waiting ack block #%d", sTC.nCount);
            // une possibilité : on est au dernier message et le serveur TFTP n'a pas acquitté
            // --> on renvoie, mais sur Timeout, on signale un transfert OK
            if (sTC.nToSend < TFTP_DATA_HEADERSIZE + sTC.nPktSize  &&  sTC.nRetransmit<TFTP_RETRANSMIT)
                return TransferOK (dNow);
        }

        send (sTC.s, sTC.BufSnd, sTC.nToSend, 0);
        SetTimer (hWnd, WM_CLIENT_ACK, sTC.dwTimeout, NULL);
      Statistics (dNow);
        break;
   } // switch message

return DefWindowProc (hWnd, message, wParam, lParam);
} // TftpProc


