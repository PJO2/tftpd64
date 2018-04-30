//////////////////////////////////////////////////////
//
// Projet TFTPD32.   Feb 99 By  Ph.jounin
// File start_threads.c:  Thread management
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////


#include "headers.h"
#include <process.h>

#include "threading.h"

#define SNTP_VERSION  2
#define SNTP_SERVER   4
#define SNTP_CLIENT   3
#define SNTP_NO_WARNING  0
#define SNTP_PORT       123


typedef unsigned char  u_int8_t;
typedef unsigned short u_int16_t;
typedef unsigned long  u_int32_t;

#pragma pack(1)
struct sSntpPkt 
{
    struct {
            unsigned char        mode:3;
            unsigned char        version:3;
            unsigned char        leap_indicator:2;
           } l;
    unsigned char    stratum;   /* peer stratum */
    unsigned char    ppoll;     /* peer poll interval */
    signed char      precision; /* peer clock precision */
    u_int32_t        rootdelay; /* distance to primary clock */
    u_int32_t        rootdispersion;    /* clock dispersion */
    u_int32_t        refid;     /* reference clock ID */
    u_int32_t        reftime_sec;   /* time peer clock was last updated */
    u_int32_t        reftime_frac;  /* time peer clock was last updated */
    u_int32_t        org_sec;       /* originate time stamp */
    u_int32_t        org_frac;      /* originate time stamp */
    u_int32_t        rec_sec;       /* receive time stamp */
    u_int32_t        rec_frac;      /* receive time stamp */
    u_int32_t        xmt_sec;       /* transmit time stamp */
    u_int32_t        xmt_frac;      /* transmit time stamp */
   }; // sSntpPkt
#pragma pack()

static struct sSntpPkt sSntpDefaultData =
{
#define TO_BE_FIILED  0
    SNTP_SERVER, 
    SNTP_VERSION,
    SNTP_NO_WARNING,
    10,     // stratum
    10,      // pool each 1024 seconds 
    0,     // precision -> 1 second 
    0x00100000,     // root delay : 0,1 sec (byte order)
    0x00800000,     // root dispersion : 0,5 sec (byte order)
    TO_BE_FIILED,   // reference
    TO_BE_FIILED,   // reference  clock
    TO_BE_FIILED,   // reference  clock
    TO_BE_FIILED,   // origin clock
    TO_BE_FIILED,
    TO_BE_FIILED,   // receive clock
    0x00000080,
    TO_BE_FIILED,   // transmit clock
    TO_BE_FIILED,
 };




///////////////////////////////////////////////////////
// FIll SNTP data
///////////////////////////////////////////////////////
struct sSntpPkt *SntpFillMessage (struct sSntpPkt *pSntpData)
{
#define SECONDS_BETWEEN_1970_AND_1900 ((unsigned long) (365*70 + 17) * 3600 * 24)
#define MILLISECONDS_TO_NTP_TIMESTAMP 0x00418937   // = int (2^32 / 1000)
struct sSntpPkt OldPkt = *pSntpData;
SYSTEMTIME               sSystemTime;
static unsigned          tLastId[] =
     { 0x121, 0x122, 0x123, 0x124, 0x125, 0x126, 0x127, 0x128, 0x129, 0x12A };  // loop detection
static unsigned          nLoopDetector;
int                      Ark;

    for (Ark=0 ; Ark<SizeOfTab(tLastId) ; Ark++) 
        if (OldPkt.xmt_frac==tLastId[Ark])   return NULL ;       // loop

    GetSystemTime (& sSystemTime);
  * pSntpData = sSntpDefaultData;   // copy default data

    pSntpData->l.version   = min (3, OldPkt.l.version);

    pSntpData->org_sec     = OldPkt.xmt_sec;       // origin
    pSntpData->org_frac    = OldPkt.xmt_frac;       // origin
    pSntpData->reftime_sec = 
    pSntpData->rec_sec     = 
    pSntpData->xmt_sec     = htonl ( (unsigned long) time(NULL) + SECONDS_BETWEEN_1970_AND_1900 );  
    pSntpData->rec_frac     = 
    pSntpData->reftime_frac= 
    tLastId [nLoopDetector++ % SizeOfTab(tLastId)]  = 
    pSntpData->xmt_frac    = htonl ( sSystemTime.wMilliseconds * MILLISECONDS_TO_NTP_TIMESTAMP 
									 + rand() & 0xFFFF ) ;

return pSntpData;
}   // SntpFillMessage 




// --------------------------------------
void SntpdProc (void *param)
{
int             Rc;
char     sSntpBuf [1024];    /* space for padding */
struct sSntpPkt *pSntpData;
SOCKADDR_STORAGE SockFrom;
int              nFromLen = sizeof SockFrom;
char             szName [256];
struct hostent  *pHostEntry;

           // give PC address as reference
           if (       gethostname (szName, sizeof szName)!=SOCKET_ERROR
                  && (pHostEntry=gethostbyname (szName)) !=0
                  &&  pHostEntry->h_addr_list!=NULL
              )
                   sSntpDefaultData.refid = * (DWORD *) pHostEntry->h_addr_list[0] ;

  tThreads [TH_SNTP].bInit = TRUE;  // inits OK


   while ( tThreads[TH_SNTP].gRunning )
   {
         Rc = recvfrom (  tThreads[TH_SNTP].skt,
                          sSntpBuf,
                          sizeof sSntpBuf,
                          0,
                         (struct sockaddr *) & SockFrom,
                          & nFromLen);
		 if (Rc < 0)
		 {
   		     LogToMonitor ("erreur %d during socket operation", GetLastError () );
			 Sleep (100);
		 }
         pSntpData = (struct sSntpPkt *) sSntpBuf;

         // ignore message if it is not a client request or if it is has been sent by Tftpd32
         if ( pSntpData->l.mode == SNTP_CLIENT )
         {
             if ( SntpFillMessage (pSntpData) != NULL)
             {
                 // SockFrom.sin_port = htons (SNTP_PORT);
                 Rc = sendto ( tThreads[TH_SNTP].skt,
                               (char *) pSntpData,
                                sizeof (*pSntpData),
                                0,
                               (struct sockaddr *) & SockFrom,
                                sizeof SockFrom);
                if (Rc<sizeof (*pSntpData))
                    LOG (1, "SNTPD: sendto error %d: %s", GetLastError(), LastErrorText ());
            } // message ok
        } // request
   }
   LogToMonitor ("End of Sntp thread\n");
}