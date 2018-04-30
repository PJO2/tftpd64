//////////////////////////////////////////////////////
//
// Projet TFTPD32.       Mai 98 Ph.jounin - June 2006
// File bootpd_util.h:   dhcp module data structure
//
// released under artistic license (see license.txt)
// 
//////////////////////////////////////////////////////



#define DHCP_DEFAULT_LEASE_TIME (2*24*60)    // two days (in minutes)

// DHCP assignations data base
struct LL_IP
{
   struct in_addr     dwIP;            // assignated address
   char               sMacAddr[16];    // MAC Address of the client
   time_t             tAllocated;      // time of assignation 
   time_t             tRenewed;        // time of client ack
   int                dwAllocNum;      // the number this record occupies in the ini file             
};

extern struct LL_IP **tFirstIP;   // sorted array of pointers on struct LL_IP
extern struct LL_IP **tMAC;		  //sorted Pointers to elements in tFirstIP, indexed by MAC addr
extern  int            nAllocatedIP;    // number of item allocated (even if never acked)



// The main settings table :
//    DHCP parameters in memory

struct S_DHCP_Param
{
//   struct in_addr  dwAddr;
   char            szAddr[64];
   int             nPoolSize;
//   struct in_addr  dwMask;
   char            szMask[64];
//   struct in_addr  dwDns;
   char            szDns1[64];
   char            szDns2[64];
//   struct in_addr  dwWins;
   char            szWins[64];
//   struct in_addr  dwGateway;
   char            szGateway[64];
//   struct in_addr  dwOpt42;
   char            szOpt42[64];
//   struct in_addr  dwOpt120;
   char            szOpt120[64];

   char            szBootFile[256];
   char            szDomainName[128];
   int             nLease;
   int             nIgnoreBootp;
   struct
   {
       int      nAddOption;
       char     szAddOption[128];
   }
   t[10];
};

extern struct S_DHCP_Param  sParamDHCP;
extern struct S_DHCP_Param  sGuiParamDHCP;

// written by Cengiz Beytas, it has been rewritten by Jesus Soto
// int FindAdapterIP(char *szIP, DWORD *pdwAdapter, DWORD *pdwFirstAdapter);// 
// struct in_addr DlgItem2Address (HWND hWnd, int nDlgItem, const char *szDescr, BOOL bStrict);


//struct in_addr DlgItem2Address (HWND hWnd, int nDlgItem, const char *szDescr, BOOL bStrict);
int CheckInetAddress (HWND hWnd, char *szAddr, const char *szDescr, BOOL bStrict);
int QsortCompare (const void *p1, const void *p2);
int MACCompare (const void* p1, const void* p2);
void IncNumAllocated (void);
void DecNumAllocated (void);
void DecNumAllocated (void);
void SetNumAllocated(int n);
void SetIP(struct LL_IP* pCur, DWORD newip);
void SetMacAddr(struct LL_IP* pCur, const unsigned char *pMac, int nMacLen);
void ZeroMacAddr(struct LL_IP* pCur);
void SetAllocTime(struct LL_IP* pCur);
void ZeroAllocTime(struct LL_IP* pCur);
void SetRenewTime(struct LL_IP* pCur);
void ForceRenewTime(struct LL_IP* pCur, time_t newtime);
void ZeroRenewTime(struct LL_IP* pCur);
void ReorderLeases (void);
void LoadLeases(void);
void FreeLeases(BOOL freepool);
struct LL_IP *DHCPSearchByIP (const struct in_addr *pAddr, BOOL* wasexpired);
struct LL_IP *DHCPSearchByMacAddress (const unsigned char *pMac, int nMacLen);
char *TranslateExp (const char *exp, char *to, struct in_addr ip, const char *tMac);

//////////////
// From ip_util.h
char * haddrtoa(const unsigned char *haddr, int hlen, char cSep);  //A few decls so we can use these here
void atohaddr(const unsigned char *addrstr, unsigned char* haddr, int haddrlen);
char* timetoa(time_t t);
time_t atotime(char* str);
int AddrFitsPool(struct in_addr* addr);
BOOL bIPSameSubnet (struct in_addr *p1, struct in_addr *p2, struct in_addr *pMask);
struct in_addr *FindNearestServerAddress (struct in_addr *pHost, struct in_addr *pMask, BOOL bExact);
unsigned long AddrInc(struct in_addr in);
