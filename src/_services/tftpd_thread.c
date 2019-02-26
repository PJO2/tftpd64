//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Mai 98 Ph.jounin - June 2006
// File tftp.c:   TFTP protocol management
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////



#undef min
#define min(a,b)  (((a) < (b)) ? (a) : (b))


// #define DEB_TEST

#include "headers.h"
#include <process.h>
#include <stdio.h>

#include "threading.h"
#include "tftpd_functions.h"


enum e_TftpMode { TFTP_BINARY, TFTP_NETASCII, TFTP_MAIL };
enum e_TftpCnxDecod {  CNX_OACKTOSENT_RRQ = 1000,
                       CNX_OACKTOSENT_WRQ,
                       CNX_SENDFILE ,
                       CNX_ACKTOSEND ,
                       CNX_FAILED
                    };




BOOL AllowSecurity (char *szFile, int op_code);

// debug functions (thrown into tftp_dbg.c)
struct LL_TftpInfo *DoDebugSendBlock (struct LL_TftpInfo *pTftp);
struct LL_TftpInfo *DoDebugRcvAck (struct LL_TftpInfo *pTftp);
struct LL_TftpInfo *DoDebugRcvData (struct LL_TftpInfo *pTftp);
struct LL_TftpInfo *DoDebugSendAck (struct LL_TftpInfo *pTftp);


/////////////////////////////////////////////////////////
// Security checks
/////////////////////////////////////////////////////////
BOOL SecAllowSecurity (const char *szFile, int op_code)
{
struct stat  sStat;
BOOL          bForward = (    strstr (szFile, "..")==NULL
                          &&  strstr (szFile, "\\\\")==NULL);

    switch (sSettings.SecurityLvl)
    {
        // SECURITY_NONE : do not verify
        case SECURITY_NONE :  return TRUE;
        // SECURITY_STD  : check that file path doe snot contain ..
        case SECURITY_STD  :
            if (! bForward) SetLastError (ERROR_DIRECTORY);
            return bForward;
        // SECURITY_HIGH : if write request file shoul exist and be empty
        case SECURITY_HIGH :
            if (bForward)
            {
                sStat.st_size = 0;
                if (op_code == TFTP_WRQ   &&  stat (szFile, & sStat) == -1)
                    return FALSE;
                // pour les logs -> positionner l'erreur
                if (sStat.st_size != 0)  SetLastError (ERROR_BAD_LENGTH);
                return (sStat.st_size == 0);
            }
            else
            {
                SetLastError (ERROR_DIRECTORY);
                return FALSE;
            }
        case SECURITY_READONLY :
            if (bForward  && op_code==TFTP_RRQ) return TRUE;
           SetLastError (ERROR_DIRECTORY);
            return FALSE;
    } // type de sécurité
return FALSE;
} // SecAllowSecurity

////////////////////////////////////////////////////////
struct errmsg {
    int e_code;
    const char *e_msg;
} errmsgs[] = {
    { EUNDEF,   "Undefined error code" },
    { ENOTFOUND, "File not found" },
    { EACCESS,  "Access violation" },
    { ENOSPACE, "Disk full or allocation exceeded" },
    { EBADOP,   "Illegal TFTP operation" },
    { EBADID,   "Unknown transfer ID" },
    { EEXISTS,  "File already exists" },
    { ENOUSER,  "No such user" },
    { ECANCELLED, "Cancelled by administrator" },
    { -1,       0 }
};

/*
 * Send a nak packet (error message).
 * Error code passed in is one of the
 * standard TFTP codes, or a UNIX errno
 * offset by 100.
 */
int nak (struct LL_TftpInfo *pTftp, int error)
{
struct tftphdr *tp;
int length;
struct errmsg *pe;

    if (pTftp->r.skt == INVALID_SOCKET) return 0;

    tp = (struct tftphdr *)pTftp->b.buf;
    tp->th_opcode = htons((u_short)TFTP_ERROR);
    tp->th_code = htons((u_short)error);
    for (pe = errmsgs; pe->e_code >= 0; pe++)
        if (pe->e_code == error)
            break;
    if (pe->e_code < 0) {
        pe->e_msg = strerror(error - 100);
        tp->th_code = EUNDEF;   /* set 'undef' errorcode */
    }
    lstrcpy(tp->th_msg, pe->e_msg);
    length = lstrlen(pe->e_msg);
	// padd with 2 null char
    * (short *) & tp->th_msg[length] = 0;
    length += 2 + offsetof (struct tftphdr, th_msg) ;
#if (defined DEBUG || defined DEB_TEST)
        BinDump (pTftp->b.buf, length, "NAK:");
#endif
    return send(pTftp->r.skt, pTftp->b.buf, length, 0) != length ? -1 : 0;
} // nak



///////////////////////////////////////
// Alter file name
///////////////////////////////////////
static void SecFileName (char *szFile)
{
char *p;
	// translate // in / : not efficient but simple
	if (sSettings.bReduceTFTPPath)
	{
		p = szFile;
		while ((p = strchr(p, '/')) != NULL)
		{
			if (*(p + 1) == '/')
				memmove(p, p + 1, lstrlen(p));
			else p++;
		}
	}

// Translate '/' in '\'
    if (sSettings.bUnixStrings)
    {
        for (p=szFile ; *p!=0 ; p++ )
            if (*p=='/')  *p='\\';

    }
    if (szFile[1] == ':')   szFile[0] = toupper (szFile[0]);
    // Si option Virtual Root : Suppression de '\\'
    // sera traité à partir du répertoire courant
  if (sSettings.bVirtualRoot  && szFile[0] == '\\')
      memmove (szFile, szFile+1, lstrlen (szFile));

} // SecFileName



/////////////////////
// TftpExtendFileName
//       add current directory to the file name
//       current directory is saved into the combobox
/////////////////////
static char *TftpExtendFileName (struct LL_TftpInfo *pTftp,
                                 const char         *szShortName,
                                 char               *szExtName,
                                 int                 nSize)
{
int  nLength;
    lstrcpy (szExtName, sSettings.szWorkingDirectory);
    nLength = lstrlen(szExtName);
    if (nLength>0   &&   szExtName[nLength-1] != '\\')  szExtName [nLength++] = '\\';
	// virtual root has already been processed 
	lstrcpy (szExtName + nLength, szShortName);
return szExtName;
} // TftpExtendFileName



/////////////////////
// TftpCreateMD5File
/////////////////////
static int TftpCreateMD5File (struct LL_TftpInfo *pTftp)
{
HANDLE          hFile;
struct tftphdr *tp;
char            szMD5File [2 * _MAX_PATH];
char            szidentBuf[35];
int             Ark;

     // close MD5 computation
     MD5Final (pTftp->m.ident, & pTftp->m.ctx);

     // retrieve connect frame which contains the file name
     tp = (struct tftphdr *)pTftp->b.cnx_frame;
     TftpExtendFileName (pTftp, tp->th_stuff, szMD5File, sizeof szMD5File - 5);
     lstrcat (szMD5File, ".md5");

     hFile = CreateFile (szMD5File,
                         GENERIC_WRITE,
                         FILE_SHARE_READ,
                         NULL,
                         CREATE_ALWAYS,
                         FILE_ATTRIBUTE_ARCHIVE,
                        NULL);
     // if (hFile == INVALID_HANDLE_VALUE)  return -1;

     for (Ark=0 ; Ark<16; Ark++)
         wsprintf (szidentBuf + 2*Ark, "%02x", pTftp->m.ident[Ark]);

     WriteFile (hFile, szidentBuf, 32, &Ark, NULL);
     CloseHandle (hFile);

    LOG (2, "MD5 written into <%s>", szMD5File);

return 0;
}  // TftpCreateMD5File


/////////////////////
// TftpBind : a bind function which choose ports in a range
//            nLow (nHigh) is lowest (highest)allowed ports
/////////////////////
static int TftpBind (SOCKET s, struct LL_TftpInfo *pTftp, unsigned nLow, unsigned nHigh)
{
unsigned nPort;
SOCKADDR_STORAGE in ;
int       Rc;


   // if settings unset or unconsistent, use defaults
   nPort =  ( nLow==69 ||  (nLow>=1024  &&  nLow<=nHigh) )  ?   nLow : 0;

   memset (&in, 0, sizeof in);
   in.ss_family = pTftp->b.from.ss_family;
   do
   {
      switch (in.ss_family)
      {
	      case AF_INET :  (* (struct sockaddr_in *) & in).sin_port = htons ((short) nPort);
					  break;

	      case AF_INET6 : (* (struct sockaddr_in6 *) & in).sin6_port = htons ((short) nPort);
					break;
	  } // switch
	  Rc = bind ( s, (struct sockaddr *)  & in, sizeof (in) );
	}
	//     range set     highest not reach       error  is address in use
	while (nPort!=0  &&  nPort++<nHigh    &&    Rc!=0 && GetLastError()==WSAEADDRINUSE);

return Rc;
} // TftpBind



/////////////////////
// TftpSysError : report errror in a system call
/////////////////////
static int TftpSysError (struct LL_TftpInfo *pTftp, int nTftpErr, char *szText)
{
// tp points on the connect frame --> file name
struct tftphdr *tp = (struct tftphdr *) pTftp->b.cnx_frame;

   LOG (0, "File <%s> : error %d in system call %s %s", tp->th_stuff, GetLastError (), szText, LastErrorText() );
   nak (pTftp, nTftpErr);
return FALSE;
} // TftpSysError


/////////////////////
// TftpSelect : wait for incoming data
//          with a pseudo exponential timeout
/////////////////////
static int TftpSelect (struct LL_TftpInfo *pTftp)
{
int          Rc;
fd_set          readfds;
struct timeval sTimeout;

       FD_ZERO (& readfds);
       FD_SET (pTftp->r.skt, &readfds);

       sTimeout.tv_usec = 0 ;
       switch (pTftp->c.nTimeOut)
       {
           case 0 :  sTimeout.tv_sec = (pTftp->s.dwTimeout+3) / 4;
                      break ;
           case 1 :  sTimeout.tv_sec = (pTftp->s.dwTimeout+1) / 2;
                      break;
           default : sTimeout.tv_sec = pTftp->s.dwTimeout ;
       }
       Rc = select(1, &readfds, NULL, NULL, &sTimeout) ;
       if (Rc == SOCKET_ERROR) return TftpSysError (pTftp, EUNDEF, "select");
return Rc; // TRUE if something is ready
} // TftpSelect



/////////////////////////////
// Parse connect datagram :
//      deep into the protocol
/////////////////////////////
static int DecodConnectData (struct LL_TftpInfo *pTftp)
{

struct tftphdr *tp, *tpack;
char *p, *pValue, *pAck;
int   opcode;
size_t   Ark, Len;
int   Rc;
BOOL  bOptionsAccepted = FALSE;
char  szExtendedName [2 * _MAX_PATH];

#if (defined DEB_TEST || defined DEBUG)
  sSettings.LogLvl = 10;
#endif

  // pad end of struct with 0
  memset (pTftp->b.padding, 0, sizeof pTftp->b.padding);
  // map the datagram on a Tftp structure
  tp = (struct tftphdr *)pTftp->b.cnx_frame;


    //-------------------------   Verify Frame validity  -------------------------
    // terminate the strings
	// PJO: 01 january 2017
    //tp->th_stuff [TFTP_SEGSIZE - TFTP_DATA_HEADERSIZE - 1] = 0;	// suppress done above

    // read or write request
    opcode = ntohs (tp->th_opcode);
    if (opcode!=TFTP_RRQ  && opcode!=TFTP_WRQ)
    {
        LOG (0, "Unexpected request %d from peer", opcode);
        LOG (0, "Returning EBADOP to Peer");
        nak (pTftp, EBADOP);
        return CNX_FAILED;
    }

    // ensure file name is strictly under _MAX_PATH (strnlen will terminates)
    if ( (Ark=strnlen (tp->th_stuff, _MAX_PATH)) >= _MAX_PATH)
    {
        LOG (0, "File name too long, return EBADOP to peer");
        nak (pTftp, EBADOP);
        return CNX_FAILED;
    }

    // Tftpd32 does not support file names with percent sign : it avoids buffer overflows vulnerabilities
    if (strchr (tp->th_stuff, '%') != NULL)
    {
        LOG (1, "Error: Tftpd32 does not handle filenames with a percent sign");
        nak (pTftp, EACCESS);
        return  CNX_FAILED;
    }
    LOG (9, "FileName is <%s>", tp->th_stuff);

    // create file index
    if (opcode==TFTP_RRQ  && sSettings.bDirText)
        CreateIndexFile ();

    // OK now parse the frame
    // it should have the following format   <FILE>\0<MODE>\0EXTENSION\0....

	// next word : Mode
    p = & tp->th_stuff[++Ark];		// ++Ark to point after the null char
	// ensure file name is strictly under the longest mode (0 included)
	Len = strnlen (p, sizeof("netascii"));
    if ( Len >= sizeof("netascii"))
    {
        LOG (0, "mode is too long, return EBADOP to peer");
        nak (pTftp, EBADOP);
        return CNX_FAILED;
    }
	Ark += Len;

    if (IS_OPT (p, "netascii")  ||  IS_OPT (p, "ascii"))
                        pTftp->s.TftpMode = TFTP_NETASCII;
    else if (IS_OPT (p, "mail"))
                        pTftp->s.TftpMode = TFTP_MAIL;
    else if (IS_OPT (p, "octet")  ||  IS_OPT (p, "binary") )
                        pTftp->s.TftpMode = TFTP_BINARY;
    else
    {
        LOG (0, "Uncorrect message");
        nak (pTftp, EBADOP);
        return CNX_FAILED;
    }
    LOG (12, "Mode is <%s>", p);
    
    Ark++;	// p[Ark] points on beginning of next word

    LOG (1, "%s request for file <%s>. Mode %s",
                opcode==TFTP_RRQ ? "Read" : "Write", tp->th_stuff, p);

    // input file parsing
    //   --> change / to \, modify directory if VirtualRoot is on
    SecFileName (tp->th_stuff);
	// Check if it passes security settings 
    if (! SecAllowSecurity (tp->th_stuff, opcode) )
    {
        LOG (1, "Error EACCESS on file %s. Ext error %s", tp->th_stuff, LastErrorText());
        nak (pTftp, EACCESS);
        return  CNX_FAILED;
    }
	// get full name
    TftpExtendFileName (pTftp, tp->th_stuff, szExtendedName, sizeof szExtendedName);
    LOG (10, "final name : <%s>", szExtendedName);
    // ensure again extended file name is under _MAX_PATH (strlen will terminates)
    // NB: we also may call CreateFileW instaed of CreateFileA, but Windows95/Me will not support
    if (strnlen (szExtendedName, _MAX_PATH) >= _MAX_PATH)
    {
        LOG (0, "File name too long, return EBADOP to peer");
        nak (pTftp, EBADOP);
        return CNX_FAILED;
    }

    // OK now we have the complete file name
    // will Windows be able to open/create it ?
    pTftp->r.hFile = CreateFile (szExtendedName,
                                 opcode==TFTP_RRQ ? GENERIC_READ : GENERIC_WRITE,
                                 FILE_SHARE_READ,
                                 NULL,
                                 opcode==TFTP_RRQ ? OPEN_EXISTING : CREATE_ALWAYS,
		// Feb 2019 : ISSUE #11 
		// FILE_FLAG_SEQUENTIAL_SCAN suppressed from flags to optimize caching behaviour
        // it seems that read is sequential 
		// however due to retransmissions, it is not
                                 FILE_ATTRIBUTE_ARCHIVE,
                                 NULL);
    if (pTftp->r.hFile == INVALID_HANDLE_VALUE)
    {
        TftpSysError (pTftp, opcode==TFTP_RRQ ? ENOTFOUND : EACCESS, "CreateFile");
        return  CNX_FAILED;
    }
    pTftp->st.dwTransferSize = GetFileSize (pTftp->r.hFile, NULL);

    ///////////////////////////////////
    // File has been correctly created/opened
    // --> Transfer will be accepted
    // but we still have to check the negotiated options
    // This procedure build the OACK packet
    ///////////////////////////////////////////////////
    // next words : Options (RFC 1782)

    memset (pTftp->b.ackbuf, 0, sizeof pTftp->b.ackbuf);
    tpack = (struct tftphdr *) pTftp->b.ackbuf;
    tpack->th_opcode = htons (TFTP_OACK);
    pAck = tpack->th_stuff;

    // ---------------------------------
    // loop to handle options
    while ( (sSettings.bNegociate || sSettings.bPXECompatibility)  &&  Ark<TFTP_SEGSIZE  &&  tp->th_stuff[Ark]!=0)
    {
        // if (tp->th_stuff[Ark] == 0) { Ark++; continue; }  // points on next word
        p = & tp->th_stuff[Ark];
        LOG (12, "Option is <%s>", p);

        // ----------
        // get 2 words (RFC 1782) : KeyWord + Value
		// p points already on Keyword, search for Value
        for ( ; Ark<TFTP_SEGSIZE && tp->th_stuff[Ark]!=0 ; Ark++);		// pass keyword
        pValue = & tp->th_stuff[Ark+1];
        for (Ark++ ; Ark<TFTP_SEGSIZE && tp->th_stuff[Ark]!=0 ; Ark++); // pass value
		// keyword or value beyond limits : do not use
		if (Ark>=TFTP_SEGSIZE || tp->th_stuff[Ark]!=0)
			break;
		// PJO: Fix 04/30/2018, only one option is handled
		Ark++; // points now on next option (safe since PopulateTftpdStruct set b.cnx frame to 0)


        LOG (12, "Option <%s>: value <%s>", p, pValue);

        if (!sSettings.bPXECompatibility && IS_OPT (p, TFTP_OPT_BLKSIZE))
        {unsigned dwDataSize;
            dwDataSize = atoi (pValue);
            if (dwDataSize<TFTP_MINSEGSIZE)            // BLKSIZE < 8 --> refus
                    LOG (9, "<%s> proposed by client %d, refused by Tftpd32", p, dwDataSize);
            if (dwDataSize>TFTP_MAXSEGSIZE)            // BLKSIZE > 16384 --> 16384
            {
                    LOG (2, "<%s> proposed by client %d, reply with %d", p, dwDataSize, TFTP_MAXSEGSIZE);
                    pTftp->s.dwPacketSize = TFTP_MAXSEGSIZE;
            }
            if (dwDataSize>=TFTP_MINSEGSIZE   && dwDataSize<=TFTP_MAXSEGSIZE)
            {
                    LOG (9, "<%s> changed to <%s>", p, pValue);
                    pTftp->s.dwPacketSize = dwDataSize;
            }
            if (dwDataSize>=TFTP_MINSEGSIZE)
            {
                lstrcpy (pAck, p),                        pAck += lstrlen (pAck)+1;
                wsprintf (pAck, "%d", pTftp->s.dwPacketSize), pAck += lstrlen (pAck)+1;
                bOptionsAccepted = TRUE;
            }
       }  // blksize options

       if (!sSettings.bPXECompatibility  &&  IS_OPT (p, TFTP_OPT_TIMEOUT))
       {unsigned dwTimeout;
            dwTimeout = atoi (pValue);
            if (dwTimeout>=1   && dwTimeout<=255)  // RFCs values
            {
                LOG (9, "<%s> changed to <%s>", p, pValue);
                lstrcpy (pAck, p),      pAck += lstrlen (pAck)+1;
                lstrcpy (pAck, pValue), pAck += lstrlen (pAck)+1;
                bOptionsAccepted = TRUE;
                pTftp->s.dwTimeout = dwTimeout;
            }
        }  // timeout options

        if (IS_OPT (p, TFTP_OPT_TSIZE))
        {
            lstrcpy (pAck, p), pAck += lstrlen (p)+1;
            // vérue si read request -> on envoie la taille du fichier
            if (opcode==TFTP_RRQ)
            {
                wsprintf (pAck, "%d", pTftp->st.dwTransferSize);
                pAck += lstrlen (pAck)+1;
                pTftp->s.dwFileSize = pTftp->st.dwTransferSize;
            }
            else
            {
                lstrcpy (pAck, pValue),     pAck += lstrlen (pAck)+1;
                pTftp->s.dwFileSize = pTftp->st.dwTransferSize = atoi (pValue); // Trust client
            }

            LOG (10, "<%s> changed to <%u>", p, pTftp->st.dwTransferSize);
            bOptionsAccepted = TRUE;
        }  // file size options

		// experimental port option
		if (sSettings.bPortOption  &&  IS_OPT (p, TFTP_OPT_PORT))
		{SOCKADDR_STORAGE   sa;
			int dummy = sizeof sa;
				// get chosen port for transfer
			    if (getsockname (pTftp->r.skt, (struct sockaddr *) & sa, & dummy)>=0)
				{char szServ[NI_MAXSERV];
			        getnameinfo ( (struct sockaddr *) & sa, sizeof sa, NULL, 0, szServ, sizeof szServ, NI_NUMERICSERV);
					LogToMonitor ("using udpport option --> %s", szServ);
                    lstrcpy (pAck, p), pAck += lstrlen (p)+1;
					wsprintf ( pAck, "%d", htons (atoi (szServ)) );
					pAck += lstrlen (pAck)+1;
					pTftp->c.nOAckPort = sSettings.Port;
                    bOptionsAccepted = TRUE;
				}

		 } // port option

    } // for all otptions

    // bOptionsAccepted is TRUE if at least one option is accepted -> an OACK is to be sent
    // else the OACK packet is dropped and the ACK or DATA packet is to be sent
    // Another annoying protocol requirement is to begin numbering to 1 for an upload
    // and to 0 for a dwonload.
    pTftp->c.dwBytes  = (DWORD) (pAck - tpack->th_stuff);

    pTftp->c.nCount = pTftp->c.nLastToSend = opcode==TFTP_RRQ && !bOptionsAccepted ? 1 : 0;
    pTftp->c.nLastBlockOfFile = pTftp->st.dwTransferSize / pTftp->s.dwPacketSize + 1 ;
    pTftp->s.ExtraWinSize = sSettings.WinSize / pTftp->s.dwPacketSize;

     if (bOptionsAccepted)
     {char szLog[TFTP_SEGSIZE];
      int  bKey;
         // build log from OACK segment
         memset (szLog, 0, sizeof szLog);
         memcpy (szLog, tpack->th_stuff, (int) (pAck - tpack->th_stuff));
         for (Ark=0, bKey=0  ; Ark<(int)(pAck-tpack->th_stuff) ; Ark++)
             if (szLog[Ark]==0)
                 szLog[Ark] = (bKey++) & 1 ? ',' : '=';
         LOG (3, "OACK: <%s>", szLog);
         LOG (13, "Size of OACK string : <%d>", pTftp->c.dwBytes);
         Rc =  opcode==TFTP_RRQ ? CNX_OACKTOSENT_RRQ : CNX_OACKTOSENT_WRQ ;
     }
     else    Rc =  opcode==TFTP_RRQ ? CNX_SENDFILE : CNX_ACKTOSEND ;

return Rc;
} // DecodConnectData



/////////////////////////////
// send the OACK packet
/////////////////////////////
static int TftpSendOack (struct LL_TftpInfo *pTftp)
{
int Rc;

        assert (pTftp!=NULL);

        // OACK packet is in ackbuf
        pTftp->c.dwBytes += sizeof (short);

        LOG (10, "send OACK %d bytes", pTftp->c.dwBytes);
		// should OACk be sent on a specifi port (option udpport) ?
		if (pTftp->c.nOAckPort != 0)
 		     Rc = UdpSend (  pTftp->c.nOAckPort, 
							(struct sockaddr *) & pTftp->b.from, sizeof pTftp->b.from, 
							pTftp->b.ackbuf, pTftp->c.dwBytes);
		else // use file transfer socket 
			 Rc = send (pTftp->r.skt, pTftp->b.ackbuf, pTftp->c.dwBytes , 0);
        if (Rc<0   || (unsigned) Rc != pTftp->c.dwBytes )
        {
			Rc = GetLastError ();
            LOG (0, "send : Error %d", WSAGetLastError ());
            return FALSE;
        }
#if (defined DEBUG || defined DEB_TEST)
        BinDump (pTftp->b.ackbuf, pTftp->c.dwBytes, "OACK:");
#endif
//struct tftphdr *tp;
//     tp = (struct tftphdr *)pTftp->b.cnx_frame;
//     if ( ntohs (tp->th_opcode) == TFTP_RRQ ) // must wait for ACK block #0
//            XXXXXXXXXXXXXXXXXXXXXX

return TRUE;        // job done
} // TftpSendOack


/////////////////////////////
// display report after successfull end of transfer
/////////////////////////////
static void TftpEndOfTransfer (struct LL_TftpInfo *pTftp)
{
struct tftphdr *tp;
int nBlock;

    tp = (struct tftphdr *)pTftp->b.cnx_frame;
    // calcul du nb de block avec correction pour l'envoi
    nBlock = ntohs (tp->th_opcode)!=TFTP_RRQ ? pTftp->c.nCount : pTftp->c.nCount - 1;
    LOG (2, "<%s>: %s %d blk%s, %d bytes in %d s. %d blk%s resent",
                tp->th_stuff,
                ntohs (tp->th_opcode)==TFTP_RRQ ? "sent" : "rcvd",
                nBlock,
                PLURAL (nBlock),
                pTftp->st.dwTotalBytes,
                (int) (time(NULL) - pTftp->st.StartTime),
                pTftp->st.dwTotalTimeOut,
                PLURAL (pTftp->st.dwTotalTimeOut));
        if (    sSettings.bBeep
            &&  time(NULL) - pTftp->st.StartTime  > TIME_FOR_LONG_TRANSFER )
                    MessageBeep (MB_ICONASTERISK);
} // TftpEndOfTransfer


// ReportNewTrf can be called from tftp_main.
int ReportNewTrf (const struct LL_TftpInfo *pTftp)
{
struct S_TftpTrfNew gui_msg;
struct tftphdr *tp;
int Rc;

LogToMonitor ("starting transfer %d\n", pTftp->tm.dwTransferId);
     tp = (struct tftphdr *)pTftp->b.cnx_frame;

    // Creates msg for the GUI
    gui_msg.dwTransferId = pTftp->tm.dwTransferId;
    gui_msg.opcode       = ntohs (tp->th_opcode);
    gui_msg.stat         = pTftp->st;
    gui_msg.from_addr    = pTftp->b.from;
    lstrcpy (gui_msg.szFile, tp->th_stuff); 
    Rc = SendMsgRequest (   C_TFTP_TRF_NEW, 
		                  & gui_msg, 
						    sizeof gui_msg, 
						    TRUE,		// block thread until msg sent
					        FALSE );		// if no GUI return
return Rc;    
} // ReportNewTrf 

static int ReportEndTrf (const struct LL_TftpInfo *pTftp)
{
struct S_TftpTrfEnd gui_msg;
int  Rc;
LogToMonitor ("end of transfer %d\n", pTftp->tm.dwTransferId);
    gui_msg.dwTransferId  = pTftp->tm.dwTransferId;
	gui_msg.stat          = pTftp->st;
    Rc = SendMsgRequest (  C_TFTP_TRF_END, 
						  (void *) & gui_msg, 
						   sizeof gui_msg, 
						   TRUE,		// block thread until msg sent
					       FALSE );		// if no GUI return	
return Rc;
} // ReportEndTrf

 //////////////////////////////////////////////////////////////////////////
 //                                                                      //
 // TFTP protocol : file transfer                                        //
 //                                                                      //
 //////////////////////////////////////////////////////////////////////////



    ////////////////////////
    //   DOWNLOAD (From Server to client)
    ////////////////////////
static int TftpSendFile (struct LL_TftpInfo *pTftp)
{
int Rc;
struct tftphdr *tp;
DWORD dwPos; // pos of file pointer

    assert (pTftp!=NULL);
    pTftp->c.nLastToSend = 1;

    if (pTftp->m.bInit)  MD5Init (& pTftp->m.ctx);

    // loop on send/recv for current packet until
    //       dwRetries set to 0 (correct ack received)
    //        dwRetries > MAXRETRIES  (incorrect dialog)
    //       to many Timeout
    // Note : nTimeout counts only the timeout, nRetries count the nb of times
    // the same block has been sent
 // pTftp->c.nCount is the last block sent

    pTftp->c.nTimeOut = pTftp->c.nRetries = 0 ;
	// for the stats, early acknowledgements are already sent
	pTftp->st.dwTotalBytes = pTftp->s.ExtraWinSize * pTftp->s.dwPacketSize;
    do
    {
         // On Timeout:  cancel anticipation window
         if (pTftp->c.nTimeOut > 0)  pTftp->c.nLastToSend = pTftp->c.nCount;

         ////////////////////////
         //   Send blocks #Count to #Count+window
         ////////////////////////
        if (pTftp->c.nCount > 0)      // if pTftp->c.nLastToSend is 0 wait for ack#0 first
        for (   ;
                pTftp->c.nLastToSend <= min (pTftp->c.nCount+pTftp->s.ExtraWinSize, pTftp->c.nLastBlockOfFile) ;
                pTftp->c.nLastToSend ++  )
         {
              tp = (struct tftphdr *)pTftp->b.buf;
			  // need to go back due to retransmission ?
			  // optimize cache beahviour
			  dwPos = SetFilePointer(pTftp->r.hFile, 0, NULL, FILE_CURRENT);
			  if (dwPos != pTftp->s.dwPacketSize * (pTftp->c.nLastToSend - 1))
			  {
				  Rc = (SetFilePointer(pTftp->r.hFile, pTftp->s.dwPacketSize * (pTftp->c.nLastToSend - 1), NULL, FILE_BEGIN) != (unsigned)-1);
				  if (!Rc)
					  return TftpSysError(pTftp, EUNDEF, "fseek");
			  }
			  Rc = ReadFile (pTftp->r.hFile, tp->th_data, pTftp->s.dwPacketSize,  & pTftp->c.dwBytes, NULL) ;
              if (! Rc)
                  return TftpSysError (pTftp, EUNDEF, "ReadFile");

              if (pTftp->m.bInit && pTftp->c.nRetries==0)  MD5Update (& pTftp->m.ctx, tp->th_data, pTftp->c.dwBytes);

              tp->th_opcode = htons (TFTP_DATA);
              tp->th_block  = htons ( (unsigned short) pTftp->c.nLastToSend );
              DoDebugSendBlock (pTftp); // empty ifndef DEBUG
#ifdef TEST_DROPP
    if (pTftp->c.nRetries<3  &&  pTftp->c.nCount==4)
        { LOG (1, "dropping 4th packet"), Rc=pTftp->c.dwBytes + TFTP_DATA_HEADERSIZE; }
    else
#endif
              Rc = send (pTftp->r.skt, pTftp->b.buf, pTftp->c.dwBytes + TFTP_DATA_HEADERSIZE, 0);

              if (Rc<0 || (unsigned) Rc != pTftp->c.dwBytes + TFTP_DATA_HEADERSIZE )
                    return TftpSysError (pTftp, EUNDEF, "send");
              pTftp->c.nRetries++ ;
        } // send block Count to Count+windowSize

        ////////////////////////
        //   receive ACK from first peer
        ////////////////////////
        if (TftpSelect (pTftp))       // something has been received
        {
             // retrieve the message (will not block since select has returned)
             Rc = recv (pTftp->r.skt, pTftp->b.ackbuf, sizeof pTftp->b.ackbuf, 0);
             if (Rc<=0)     return TftpSysError (pTftp, EUNDEF, "recv");

             tp = (struct tftphdr *) pTftp->b.ackbuf;

             //////////////////////////////////////////////
             // read the message
             //////////////////////////////////////////////
             if (Rc<TFTP_ACK_HEADERSIZE)
             {
                  LOG (1, "rcvd packet too short");
             }
             else if (ntohs (tp->th_opcode) == TFTP_ERROR)
             {
                  LOG (1, "Peer returns ERROR <%s> -> aborting transfer", tp->th_msg);
                  return FALSE;
             }
             else if (ntohs (tp->th_opcode) == TFTP_ACK)
             {
                  DoDebugRcvAck (pTftp);    // empty ifndef DEBUG
                  // the right ack has been received
                  if (ntohs (tp->th_block) == (unsigned short) pTftp->c.nCount)
                  {
                        pTftp->st.dwTotalTimeOut += pTftp->c.nTimeOut;
                        pTftp->c.nTimeOut = 0;
                        pTftp->c.nRetries = 0 ;
                        if (pTftp->c.nCount!=0)  // do not count OACK data block
                                pTftp->st.dwTotalBytes += pTftp->c.dwBytes;  // set to 0 on error
                        pTftp->c.nCount++;        // next block
                  } // message was the ack of the last sent block
                  else
                  {
                      // fixes the Sorcerer's Apprentice Syndrome
                      // if an this is an ACK of an already acked packet
                      // the message is silently dropped
                      if (    pTftp->c.nRetries<3
                          &&  pTftp->c.nCount != 1        // Do not pass the test for the 1st block
                          &&  ntohs (tp->th_block) == (unsigned short) (pTftp->c.nCount-1) )
                      {
                            LOG (1, "Ack block %d ignored (received twice)",
                                 (unsigned short) (pTftp->c.nCount - 1), NULL);
                      }
                      // Added 29 June 2006: discard an ack of a block which has still not been sent
                      // only for unicast transfers
                      // works only for the 65535 first blocks
                      else if (!pTftp->c.bMCast && (unsigned) ntohs (tp->th_block) > (DWORD) pTftp->c.nCount )
                      {
                           LOG (1, "Ack of block #%d received (last block sent #%d !)",
                                 ntohs (tp->th_block), pTftp->c.nCount);
                      }

                  }   // bad ack received
            } // ack received
            else
            {
                  LOG (2, "ignore unknown opcode %d received", ntohs (tp->th_opcode) );
            } // unknown packet rcvd
        } //something received
        else
        {
              LOG (9, "timeout while waiting for ack blk #%d", pTftp->c.nCount );
              pTftp->c.nTimeOut ++;
        }   // nothing received
    } // for loop
    while (      ( pTftp->c.nLastToSend<=pTftp->c.nLastBlockOfFile	 // not eof or eof but not acked
						|| (! sSettings.bIgnoreLastBlockAck && pTftp->c.nRetries!=0) ) 
              &&  pTftp->c.nRetries<TFTP_MAXRETRIES                 // same block sent N times (dog guard)
              &&  pTftp->c.nTimeOut<sSettings.Retransmit ) ;        // N timeout without answer

    // reason of exiting the loop
    if (pTftp->c.nRetries >= TFTP_MAXRETRIES)	// watch dog
    {
         LOG (0, "MAX RETRIES while waiting for Ack block %d. file <%s>",
              (unsigned short) pTftp->c.nCount, ((struct tftphdr *) pTftp->b.cnx_frame)->th_stuff  );
        nak (pTftp, EUNDEF);  // return standard
       return FALSE;
    }
    else if ( pTftp->c.nTimeOut >= sSettings.Retransmit  &&  pTftp->c.nLastToSend>pTftp->c.nLastBlockOfFile )
	{
		LOG (1, "WARNING : Last block #%d not acked for file <%s>", 
              (unsigned short) pTftp->c.nCount, ((struct tftphdr *) pTftp->b.cnx_frame)->th_stuff  );
		pTftp->st.dwTotalTimeOut += pTftp->c.nTimeOut;
	}
    else if ( pTftp->c.nTimeOut >= sSettings.Retransmit )
    {
         LOG (1, "TIMEOUT waiting for Ack block #%d ", pTftp->c.nCount);
         nak (pTftp, EUNDEF);  // return standard
       return FALSE;
    }
    LOG (10, "Count %d, Last pkt %d ", pTftp->c.nCount, pTftp->c.nLastToSend);

    // do not create MD5 file for mcast transfer
    if (pTftp->m.bInit  && !pTftp->c.bMCast)  TftpCreateMD5File (pTftp);

    TftpEndOfTransfer (pTftp);
return TRUE;
} // TftpSendFile




    ////////////////////////
    //   UPLOAD (From client to Server)
    ////////////////////////
static int TftpRecvFile (struct LL_TftpInfo *pTftp, BOOL bOACK)
{
int Rc;
struct tftphdr *tp;

     assert (pTftp!=NULL);

    // if no OACK ready, we have to send an ACK to acknowledge the transfer
     tp = (struct tftphdr *) pTftp->b.ackbuf;
  if (! bOACK)
   {
      tp->th_opcode = htons (TFTP_ACK);
      tp->th_block  = 0;
         Rc = send (pTftp->r.skt, pTftp->b.ackbuf, TFTP_ACK_HEADERSIZE, 0);
         if (Rc != (int) TFTP_ACK_HEADERSIZE)
                return TftpSysError (pTftp, EUNDEF, "send");
    } // ACK to send

  pTftp->c.nRetries = 0;
     pTftp->c.nTimeOut = 0;

     // MD5 auto computation
   if (pTftp->m.bInit)  MD5Init (& pTftp->m.ctx);


  do     // stop if max retries, max timeout or received less than dwPacketSize bytes
    {
     // recv data block
        if (TftpSelect (pTftp))      // something has been received
     {
          Rc = recv (pTftp->r.skt, pTftp->b.buf, sizeof pTftp->b.buf, 0);
            if (Rc<=0)      return TftpSysError (pTftp, EUNDEF, "recv");

            tp = (struct tftphdr *) pTftp->b.buf;
            DoDebugRcvData (pTftp);

           if (Rc<TFTP_DATA_HEADERSIZE)
           {
                LOG (1, "rcvd packet too short");
               nak (pTftp, EBADOP);
               return FALSE;
          }
          // it should be a data block
           if (ntohs (tp->th_opcode) != TFTP_DATA)
            {
              LOG (1, "Peer sent unexpected message %d", ntohs (tp->th_opcode));
                nak (pTftp, EBADOP);
               return FALSE;
          }
          // some client have a bug and sent data block #0 --> accept it anyway after one resend
#ifdef YYYYY
LOG (1, "block %d, previous acked %d, rcvd bytes %d, retries %d",
ntohs(tp->th_block), pTftp->c.nCount, pTftp->st.dwTotalBytes, pTftp->c.nRetries);
#endif
           if (ntohs(tp->th_block)==0 && pTftp->c.nCount==0)
          {
            pTftp->c.dwBytes = Rc-TFTP_DATA_HEADERSIZE;
            if (pTftp->st.dwTotalBytes==0 && pTftp->c.nRetries==0)
             {
                 LOG (1, "WARNING: First block sent by client is #0, should be #1, fixed by Tftpd32");
                 pTftp->c.nCount--;  // this will fixed the pb
           }
              else continue;  // wait for next block
           } // ACK of block #0

          // pTftp->c.nCount is the last block acked
         if (ntohs (tp->th_block) == (unsigned short) (pTftp->c.nCount+1 ) )
            {
              pTftp->c.nCount++;
              pTftp->st.dwTotalTimeOut += pTftp->c.nTimeOut;
              pTftp->c.nTimeOut = 0;
              pTftp->c.nRetries = 0;
                pTftp->c.dwBytes = Rc-TFTP_DATA_HEADERSIZE;
                Rc = WriteFile (pTftp->r.hFile, tp->th_data, pTftp->c.dwBytes, & pTftp->c.dwBytes, NULL);
              if (! Rc)
                  return TftpSysError (pTftp, ENOSPACE, "write");

               if (pTftp->m.bInit)  MD5Update (& pTftp->m.ctx, tp->th_data, pTftp->c.dwBytes);

              // Stats
               pTftp->st.dwTotalBytes += pTftp->c.dwBytes;    // set to 0 on error
         } // # block received OK
       } // Something received
        else
        {
               LOG (9, "timeout while waiting for data blk #%d", pTftp->c.nCount+1 );
             pTftp->c.nTimeOut ++;
        }  // nothing received

       // Send ACK of current block
       tp = (struct tftphdr *) pTftp->b.ackbuf;
       tp->th_opcode  = htons ((unsigned short) TFTP_ACK);
        tp->th_block   = htons ((unsigned short) pTftp->c.nCount);
     send (pTftp->r.skt, pTftp->b.ackbuf, TFTP_ACK_HEADERSIZE, 0);

 } // do
    while (     pTftp->c.dwBytes==pTftp->s.dwPacketSize
            &&  pTftp->c.nRetries  < TFTP_MAXRETRIES
           &&  pTftp->c.nTimeOut < sSettings.Retransmit) ;

   // MAX RETRIES -> synchro error
   if (pTftp->c.nRetries >= TFTP_MAXRETRIES)
   {
         LOG (0, "MAX RETRIES while waiting for Data block %d. file <%s>",
                 (unsigned short) (pTftp->c.nCount+1), ((struct tftphdr *) pTftp->b.cnx_frame)->th_stuff  );
         nak (pTftp, EUNDEF);  // return standard
        return FALSE;
   }
   if (pTftp->c.nTimeOut >= sSettings.Retransmit)
   {
        LOG (0, "TIMEOUT while waiting for Data block %d, file <%s>",
                 (unsigned short) (pTftp->c.nCount+1), ((struct tftphdr *) pTftp->b.cnx_frame)->th_stuff );
         nak (pTftp, EUNDEF);  // return standard
         return FALSE;
   }

    // do not create MD5 file for mcast transfer
   if (pTftp->m.bInit && !pTftp->c.bMCast)  TftpCreateMD5File (pTftp);
   TftpEndOfTransfer (pTftp);
return TRUE;
}   // TftpRecvFile


////////////////////////////////////////////////////////////
// _USERENTRY StartTftpTransfer (void *pThreadArgs)
// The beginning of the thread
// ie. the beginning of a transfer
////////////////////////////////////////////////////////////
DWORD WINAPI StartTftpTransfer (LPVOID pThreadArgs)
{
struct LL_TftpInfo *pTftp = (struct LL_TftpInfo *) pThreadArgs;
int              Rc;
BOOL             bSuccess;

   // do not let die the first thread (avoid the memory leak)
   do
   {

     if (pTftp->tm.bPermanentThread)   
     {
         Rc = WaitForSingleObject(pTftp->tm.hEvent,INFINITE);
         LogToMonitor ("permanent thread signalled %d (%d)\n", Rc, GetLastError());
     }
     if ( ! tThreads [TH_TFTP].gRunning ) break;

     //////////////////////////////////
     // Read the Request and prepare socket for Reply
     // Common socket sTftpListenSocket has to be used
     //////////////////////////////////

     // let the GUI run
     SetThreadPriority (pTftp->tm.dwThreadHandle, THREAD_PRIORITY_BELOW_NORMAL);

     // create a socket for the dialog
     // pTftp->b.from.sin_family = AF_INET;
	 if (  pTftp->b.from.ss_family == AF_INET6  &&  IN6_IS_ADDR_V4MAPPED ( & (* (struct sockaddr_in6 *) & pTftp->b.from ).sin6_addr )  )
	 {
		 // If ingress conection is IPv4 mapped use IPv4 sockets for answer
		 // Hack copied from Apache
		 struct sockaddr_in in ;
		 memset (& in, 0, sizeof in);
		 in.sin_family = AF_INET;
		 in.sin_addr.s_addr =  ((DWORD *) (* (struct sockaddr_in6 *) & pTftp->b.from).sin6_addr.s6_bytes) [3];
		 in.sin_port        = (* (struct sockaddr_in6 *) & pTftp->b.from).sin6_port;
		 * (struct sockaddr_in *) & pTftp->b.from = in;
	 }


	 if ( (pTftp->r.skt = socket(pTftp->b.from.ss_family, SOCK_DGRAM, 0) ) == INVALID_SOCKET)
     {
		  Rc = GetLastError ();
          LOG (0, "Error : socket returns %d: <%s>", Rc, LastErrorText());
     }
     else if ( TftpBind (pTftp->r.skt, pTftp,  sSettings.nTftpLowPort, sSettings.nTftpHighPort) != 0 )
     {
		  Rc = GetLastError ();
          LOG (0, "Error : bind returns %d: <%s>", Rc, LastErrorText());
     }
     else if ( (Rc=connect(pTftp->r.skt, (struct sockaddr *) & pTftp->b.from, sizeof pTftp->b.from)) != 0 )
     {
		  Rc = GetLastError ();
          LOG (0, "Error : connect returns %d: <%s>", Rc, LastErrorText());
          Sleep (1000);
     }
         //////////////////////////////////
         // Parse the RRQ/WRQ request
         /////////////////////////////////
     else if ( (Rc = DecodConnectData (pTftp)) !=  CNX_FAILED )
     {struct sockaddr_storage s_in;
      unsigned s_len = sizeof s_in;
	  char szServ[NI_MAXSERV];
         getsockname( pTftp->r.skt, (struct sockaddr *) & s_in, & s_len);
		 getnameinfo ( (struct sockaddr *) & s_in, sizeof s_in, NULL, 0, szServ, sizeof szServ, NI_NUMERICSERV);
         LOG (1, "Using local port %s", szServ);

		 pTftp->st.ret_code =  TFTP_TRF_RUNNING;
         ReportNewTrf (pTftp);

		 bSuccess = FALSE;
         // everything OK so far
         switch (Rc)
         {
          // download RRQ
             case CNX_OACKTOSENT_RRQ :
                       if (TftpSendOack (pTftp))  bSuccess = TftpSendFile (pTftp);
						break;
             case CNX_SENDFILE :
                       bSuccess = TftpSendFile (pTftp);
                       break;
             // upload WRQ
             case CNX_OACKTOSENT_WRQ :
                       if (TftpSendOack (pTftp))  bSuccess = TftpRecvFile (pTftp, TRUE);
                       break;
             case CNX_ACKTOSEND :
                       bSuccess = TftpRecvFile (pTftp, FALSE);
                       break;
            default :  MessageBox (NULL, "Humm", "Tftpd32", MB_OK);
         }

LogToMonitor ("return from thread\n");
		 pTftp->st.ret_code =  bSuccess ? TFTP_TRF_SUCCESS :  TFTP_TRF_ERROR;
         if ( tThreads [TH_TFTP].gRunning )  ReportEndTrf (pTftp);

     } // no error in init


       if ( pTftp->r.skt!=INVALID_SOCKET && 0==closesocket (pTftp->r.skt)==0 )
                    pTftp->r.skt=INVALID_SOCKET ;
       if ( pTftp->r.hFile!=INVALID_HANDLE_VALUE   &&   CloseHandle(pTftp->r.hFile) )
                  pTftp->r.hFile=INVALID_HANDLE_VALUE ;

        // pause before deleting entry into List View
        if (pTftp->tm.bPermanentThread)  { pTftp->tm.bActive = FALSE;   }
        //Sleep (5000);
    }  // loop if this is thread is permanent
    while (   pTftp->tm.bPermanentThread  &&  tThreads[TH_TFTP].gRunning  ) ;

LogToMonitor ("worker thread leaving %d\n", GetCurrentThreadId () );

  pTftp->tm.bActive = FALSE;
  SetEvent ( tThreads[TH_TFTP].hEv ); 
  Sleep (250);
 _endthread ();
return 0;
} /* StartTftpTransfer */

