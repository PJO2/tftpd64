
// bootpd_util.h
//
// source released under artistic license (see license.txt)
//


#define FREE_DHCP_ADDRESS "FF:FF:FF:FF:FF:FF"

int DHCPSaveConfig ( const struct S_DHCP_Param  *pNewParamDHCP );
int DHCPReadConfig ( void );

int IsMacEmpty (struct LL_IP* pcur);
int QsortCompare (const void *p1, const void *p2);
int MACCompare (const void* p1, const void* p2);
void IncNumAllocated (void);
void DecNumAllocated (void);
void SetNumAllocated(int n);
void SetIP (struct LL_IP* pCur, DWORD newip);
void SetMacAddr (struct LL_IP* pCur, const unsigned char *pMac, int nMacLen);
void ZeroMacAddr (struct LL_IP* pCur);
void SetAllocTime (struct LL_IP* pCur);
void ZeroAllocTime (struct LL_IP* pCur);
void SetRenewTime (struct LL_IP* pCur);
void ForceRenewTime (struct LL_IP* pCur, time_t newtime);
void ZeroRenewTime (struct LL_IP* pCur);
void ReorderLeases (void);
void LoadLeases (void);
void FreeLeases (BOOL freepool);
struct LL_IP *DHCPSearchByIP (const struct in_addr *pAddr, BOOL* wasexpired);
struct LL_IP *DHCPSearchByMacAddress (const unsigned char *pMac, int nMacLen);

void Dhcp_Send_Leases (const struct LL_IP *tIP[], int nbLeases);
void DHCPDestroyItem (struct LL_IP *pCur);

int TranslateParam2Value (void *buffer, int len, const char *opt_val, struct in_addr ip, const char *tMac);
int ArpDeleteHost(struct in_addr addr);