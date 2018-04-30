//////////////////////////////////////////////////////
//
// Projet TFTPD32.       Mai 98 Ph.jounin - June 2006
// File tftp_struct.h:   Tftpd32 data structure
//
// released under artistic license (see license.txt)
// 
//////////////////////////////////////////////////////


#define MAX_TFTP_OPTIONS	16


////////////////////////////////////////////////////////////
// The transfer/thread structure
// This whole structure is allocated for each transfer = for each thread
// (since a new thread is started for each new transfer)
////////////////////////////////////////////////////////////

enum e_TftpRetCode { TFTP_TRF_RUNNING, TFTP_TRF_SUCCESS, TFTP_TRF_STOPPED, TFTP_TRF_ERROR };

// settings for the current transfer
struct S_Trf_Settings
{
    DWORD           dwPacketSize;     // Size of a data packet : Note this is a key data
                                      // since if size of received packet != dwPacketSize
                                      // transfer is terminated
    DWORD           dwTimeout;        // Timeout
    unsigned        TftpMode;         // transfer mode, only binary mode is supported
    unsigned        ExtraWinSize;     // Data to sent without waiting for ACK 
    DWORD           dwFileSize;       // -1 if not set
    DWORD           dwMcastAddr;      // Multicast address   
};      // struct S_Trf_Settings

// Buffers
struct S_Trf_Buffers
{
    char           buf[MAXPKTSIZE];     // one piece of file
    char           ackbuf[PKTSIZE];
    SOCKADDR_STORAGE       from ;             // stack of address of remote peers
    char           cnx_frame[PKTSIZE];		  // The 'connexion' datagram, contains file name
	char		   padding[4];				  // set to zero before reading connect datagram
} ;         // struct S_Trf_Buffers

// transfer stats and progress bar : not mandatory 
struct S_Trf_Statistics
{
    DWORD          dwTransfert;     // number of simultaned trf
    DWORD          dwTotalBytes;    // number of transferred bytes
    DWORD          dwTotalTimeOut;  // for stat
    DWORD          dwTransferSize;  // transfer size (read from SIZE option)
    time_t         StartTime;
    time_t         dLastUpdate;     // Last gauge update (seconds)
	DWORD          ret_code;
} ;         // struct S_Trf_Statistics
// control data
struct S_Trf_Control
{
    BOOL            bMCast;            // current transfer is multicast
    DWORD           nCount;            // current packet #
    DWORD           nLastBlockOfFile;  // Last block of file
    // DWORD          dwLastAckPacket; // Sorcerer's Apprentice Syndrome
    DWORD           nLastToSend;       // last sent packet
    DWORD           dwBytes;           // taille de la zone Data (th_stuff)
    unsigned        nTimeOut;          // # of consecutive timeouts
    unsigned        nRetries;          // same datagram resent # times
	int             nOAckPort;         // OAck should be sent on this port
};      // struct S_Trf_Control
// resource allocated for the transfer
struct S_Trf_Resource
{
    SOCKET          skt;                // socket de la connexion
    HANDLE          hFile;              // file handler
    // HWND            hGaugeWnd;          // Handler of Gauge Window
} ;         // struct S_Trf_Resource
// Thread management
struct S_Thread_Management
{
   // The thread which manages the transfer
   BOOL            bActive ;           // act as a semaphore 
                                       // TRUE if thread is busy
   BOOL            bPermanentThread ;  // is thread permanent
   HANDLE          dwThreadHandle; 
   DWORD           dwThreadHandleId;
   HANDLE          hEvent;            // Event used to activate permanent threads
   HWND            hWnd;              // identifiant of main window
   DWORD           dwTransferId;       // transfer id 
   int             N;
} ;         // struct  S_Thread_Management
// MD5 computation data
struct S_Trf_MD5
{
   BOOL           bInit; // contains a copy of bMD5 settings (which may be changed)
   MD5_CTX        ctx;
   unsigned char ident [16];
} ;  // struct S_Trf_MD5


// The super structure and pointer to next block
struct LL_TftpInfo
{
    struct S_Thread_Management  tm;
    struct S_Trf_Resource       r;
    struct S_Trf_Control        c;
    struct S_Trf_Settings       s;
    struct S_Trf_Buffers        b;
    struct S_Trf_Statistics     st;
    struct S_Trf_MD5            m;

    struct LL_TftpInfo *next;
} ;

////////////////////////////////////////////////////////////
// End of transfer/thread structure
////////////////////////////////////////////////////////////
