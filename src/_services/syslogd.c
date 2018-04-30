//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Mai 98 Ph.jounin
// File syslogd.c:  Syslog
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////

#include "headers.h"
#include "threading.h"



static HANDLE hSysLogFile = INVALID_HANDLE_VALUE;

const char *sPipeName = "\\\\.\\pipe\\Tftpd32Syslog";


///////////////////////////////////////////////////////
// Open the log file
// uses sSettings, modifies hSysLogFile
///////////////////////////////////////////////////////
void SyslogCreateLogFile (LPSTR szLogFile)
{
   if (hSysLogFile !=INVALID_HANDLE_VALUE)
       CloseHandle (hSysLogFile);

   // if file already exists append at end of file
   if (szLogFile != NULL  && szLogFile[0]!=0)
   {
       hSysLogFile = CreateFile (szLogFile,
                                 GENERIC_WRITE,
                                 FILE_SHARE_READ,
                                 NULL,
                                 OPEN_ALWAYS,
                                 FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_SEQUENTIAL_SCAN ,
                                 NULL);
       // seek end of file
       //if (hSysLogFile==INVALID_HANDLE_VALUE)
       //           MY_WARNING ("can not open Syslog File");
       SetFilePointer (hSysLogFile, 0, 0, FILE_END);
 }
} // SyslogOpenLog




// check that received message match the form '<....?.?>'
// may be this must be an option
BOOL CheckSyslogMsg (char *SyslogTxt, int nSize)
{
int Ark;
   if (nSize < 5)  return FALSE;
   if (    SyslogTxt[0] != '<'
       && (SyslogTxt[4] != '>'  || SyslogTxt[5] != '>' || SyslogTxt[6] != '>')
      )  return FALSE;

   SyslogTxt[nSize] = 0;
   for (Ark=0 ;  Ark<nSize ;  Ark++)
     if (! isascii (SyslogTxt[Ark]))  SyslogTxt[Ark]='.';

return TRUE;
} // CheckSyslogMsg




/////////////////////////////
// Background window 
//
void SyslogProc (void * param)
{
SOCKET   sSyslogListenSocket=INVALID_SOCKET;
char     szSyslogBuf[SYSLOG_MAXMSG+1]; // Buffer 
int             Rc;
SOCKADDR_STORAGE sSock;
int             nDummy;
HANDLE m_hPipe=INVALID_HANDLE_VALUE; 
struct S_SyslogMsg msg;

   if (sSettings.bSyslogPipe)
   {
       m_hPipe = CreateFile (sPipeName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
       //if (m_hPipe == INVALID_HANDLE_VALUE)
       //    MY_WARNING("Named pipe Tftpd32Syslog has not been created\r\nTftpd32 will not forward Syslog messages");
   }
   sSyslogListenSocket  = tThreads[TH_SYSLOG].skt;
   SyslogCreateLogFile (sSettings.szSyslogFile);

   tThreads [TH_SYSLOG].bInit = TRUE;  // inits OK

   while ( tThreads[TH_SYSLOG].gRunning )
   {
      nDummy = sizeof sSock;
      // get the message, checks its format and display it
      Rc = recvfrom (sSyslogListenSocket, szSyslogBuf, SYSLOG_MAXMSG, 
                     0, (struct sockaddr *) & sSock, & nDummy);

      // something received and format OK
      if (Rc>0  && CheckSyslogMsg (szSyslogBuf, Rc) )
      {

		  // who sent the UDP message
		  getnameinfo ( (LPSOCKADDR) & sSock, sizeof sSock, 
		                 msg.from, sizeof msg.from, 
				         NULL, 0,
				         NI_NUMERICHOST );          
		  if ( sSock.ss_family == AF_INET6 
			 && IN6_IS_ADDR_V4MAPPED ( & (* (struct sockaddr_in6 *) & sSock ).sin6_addr ) )
          {
		  	  memmove (msg.from, msg.from + sizeof ("::ffff:") - 1, strlen (msg.from + sizeof ("::ffff:") -1) +1 );        
		  }
		  msg.from[sizeof msg.from - 1] = 0; // probably paranoid


		  if (hSysLogFile != INVALID_HANDLE_VALUE)
		  {
			  struct tm *newtime;
			  time_t tm;
			  int Dummy;
			  char szTxt[SYSLOG_MAXMSG + 1 + 30];
			  time(&tm);
			  newtime = localtime(&tm);
			  // copy in file the string without the new-line
			  wsprintf(szTxt,"%24.24s;%s; %s\r\n",asctime(newtime),msg.from,szSyslogBuf);
			  WriteFile(hSysLogFile, szTxt, lstrlen(szTxt), &Dummy, NULL);
		  } // log in file

		  if (m_hPipe != INVALID_HANDLE_VALUE) WriteFile(m_hPipe, szSyslogBuf, Rc, &nDummy, NULL);

          lstrcpy (msg.txt, szSyslogBuf); 
          SendMsgRequest (   C_SYSLOG, 
			               & msg, 
						     1 + Rc + sizeof msg.from, 
							 FALSE,	  	    // don't block thread until msg sent
							 FALSE );		// if no GUI return
      }
   } // loop

   if (m_hPipe != INVALID_HANDLE_VALUE)     CloseHandle (m_hPipe);
   if (hSysLogFile != INVALID_HANDLE_VALUE) CloseHandle (hSysLogFile);
   LogToMonitor ("End of Syslog thread\n");
_endthread ();
} // SyslogDProc

