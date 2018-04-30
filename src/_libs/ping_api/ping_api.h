//////////////////////////////////////////////////////
//
// Projet DHCPD32.         January 2006 Ph.jounin
// PING_API.H -- Ping program using ICMP and RAW Sockets
//
//
// source released under artistic License (see license.txt)
// 
//////////////////////////////////////////////////////

// return codes
enum {  PINGAPI_SOCKERROR = -1000, 
        PINGAPI_INITERROR, 
        PINGAPI_PRIVERROR, 
        PINGAPI_TIMEOUT, 
        PINGAPI_UNKNOWNPKT, 
        PINGAPI_UNREACHABLE, 
        PINGAPI_TTLEXPIRE 
     };

#ifdef _MSC_VER
// The following two structures need to be packed tightly, but unlike
// Borland C++, Microsoft C++ does not do this by default.
#pragma pack(1)
#endif

// PingApi : 
// Params : pAddr       : address of destination
//          dwTimeout  : Timeout in msec
//          pTTL        : init TTL
// return :
//          the time in msec or a negative error code
//          WSAGetLastError() is preserved by the API.
int PingApi   (struct in_addr *pAddr, DWORD dwTimeout_msec, int *pTTL);

//This will attempt to ping and arp the entire range and add them to the lease file
//Don't call this while you are processing DHCP, as the leases are not thread safe.
void PingRange(struct in_addr* pstart, DWORD count);

unsigned short in_cksum(unsigned short *addr, int len);


#define PINGAPI_MYID 216


// ICMP packet types
#define ICMP_ECHO_REPLY     0
#define ICMP_DEST_UNREACH   3
#define ICMP_ECHO_REQUEST   8
#define ICMP_TTL_EXPIRE    11

// Minimum ICMP packet size, in bytes
#define ICMP_MIN 8


// IP Header -- RFC 791
typedef struct tagIPHDR
{
    unsigned char    VIHL;          // Version and IHL
    unsigned char   TOS;            // Type Of Service
    short           TotLen;         // Total Length
    short           ID;             // Identification
    short           FlagOff;        // Flags and Fragment Offset
    unsigned char   TTL;            // Time To Live
    unsigned char   Protocol;       // Protocol
    unsigned short  Checksum;       // Checksum
    struct  in_addr iaSrc;          // Internet Address - Source
    struct  in_addr iaDst;          // Internet Address - Destination
}IPHDR, *PIPHDR;


// ICMP Header - RFC 792
typedef struct tagICMPHDR
{
    unsigned char   Type;           // Type
    unsigned char   Code;           // Code
    unsigned short  Checksum;       // Checksum
    unsigned short  ID;             // Identification
    unsigned short  Seq;            // Sequence
    char    Data;                   // Data
}ICMPHDR, *PICMPHDR;


#define REQ_DATASIZE 32      // Echo Request Data size

// ICMP Echo Request
typedef struct tagECHOREQUEST
{
    ICMPHDR     icmpHdr;
    DWORD       dwTime;
    char        cData[REQ_DATASIZE];
}ECHOREQUEST, *PECHOREQUEST;


// ICMP Echo Reply
typedef struct tagECHOREPLY
{
   IPHDR       ipHdr;
   ECHOREQUEST echoRequest;
   char        cFiller[256];
}ECHOREPLY, *PECHOREPLY;

#ifdef _MSC_VER
// return to default alignment
#pragma pack()
#endif
