//////////////////////////////////////////////////////
//
// Projet TFTPD32.  January 2006 Ph.jounin
// Projet DHCPD32.  January 2006 Ph.jounin
// File threading.h:    Manage threads
//
// source released under artistic license (see license.txt)
//
//////////////////////////////////////////////////////

#include "async_log.h"


// Starts one thread per service (Tftpd, Sntpd, dhcpd)
// Order is the same than tThreadsConfig array 
// pseudo service should be first
enum e_Threads { TH_CONSOLE, 
                 TH_ASYNCSAVEKEY, 
				 TH_SCHEDULER,
				 TH_DHCP, 
				 TH_TFTP, 
				 TH_SNTP, 
				 TH_DNS,
				 TH_SYSLOG, 
				 TH_NUMBER };

// Events created for main threads
struct S_ThreadMonitoring
{
    int     gRunning;    // thread status
    HANDLE  tTh;         // thread handle
    HANDLE  hEv;         // wake up event
    SOCKET  skt;         // Listening SOCKET
	int     bSoftReset;  // Thread will be reset
	BOOL    bInit;		 // inits are terminated
}  
tThreads [TH_NUMBER];

struct S_RestartTable
{
	int oldservices;
	int newservices;
	int flapservices;
};


// threads started by StartAllThreads
void TftpdConsole (void *param);
void ListenDhcpMessage (void *param);
void TftpdMain (void *param);
void SntpdProc (void *param);
void SyslogProc (void *param);
void AsyncSaveKeyBckgProc (void *param);
void Scheduler (void *param);
void ListenDNSMessage (void * param);


// Threads management : birth, life and death
int  StartAllWorkerThreads (void);
int  StartMultiWorkerThreads (BOOL bSoft);
int  WakeUpThread (int Idx);
void TerminateWorkerThreads (BOOL bSoft);
int GetRunningThreads (void);

// Access to console
int SendMsgRequest (int type,				// msg type
					const void *msg_stuff,	// data
					int size,				// size of data
					BOOL bBlocking,			// block thread until msg sent
					BOOL bRetain );			// retain msg if GUI not connected

BOOL Tftpd32ReadSettings (void);
BOOL Tftpd32SaveSettings (void);
BOOL Tftpd32DestroySettings (void);
void Tftpd32UpdateServices (void *lparam);

// Send the IP interfaces
int	AnswerIPList (void);

// Complex actions handled by console thread
void SendDirectoryContent (void);

// interaction between tftp and console (statistics)
DWORD WINAPI StartTftpTransfer (LPVOID pThreadArgs);
int ReportNewTrf (const struct LL_TftpInfo *pTftp);
void ConsoleTftpGetStatistics (void);

// Actions called by Scheduler
int PoolNetworkInterfaces (void);
int GetIPv4Address (const char *szIf, char *szIP);
