//////////////////////////////////////////////////////
//
// Projet TFTPD32.  December 2006 Ph.jounin
// Projet DHCPD32. December 2006 Ph.jounin
// File bootp.c:    Bootp/Dchp utilities
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////

#include <windows.h>
#include <winsock.h>
#include <time.h>
#include <stdio.h>
#include "bootpd_util.h"

#ifndef MSVC
#  define sscanf_s sscanf
#endif



//quick smart increment function to avoid addresses ending in 0 or 255.
//in is in network order, but what is returned is in host order
unsigned long AddrInc(struct in_addr in)
{
   unsigned long addr = ntohl(in.s_addr);
   if((addr & 0xff) == 254)
      return addr + 3;  //Skip 0 and 1
   if((addr & 0xff) == 255)
      return addr + 2;  //At least skip 0...
   return addr + 1;
} // AddrInc


//Returns true if the address would fit in the pool
int AddrFitsPool(struct in_addr* addr)
{
   return ntohl(addr->s_addr) >= ntohl (inet_addr(sParamDHCP.szAddr))
          &&  ntohl (addr->s_addr) <  ntohl (inet_addr(sParamDHCP.szAddr)) + sParamDHCP.nPoolSize;
}

// Return TRUE if 2 IP addresses are in the same subnet
BOOL bIPSameSubnet (struct in_addr *p1, struct in_addr *p2, struct in_addr *pMask)
{ return (p1->s_addr & pMask->s_addr) == (p2->s_addr & pMask->s_addr); } // bIPSameSubnet

// Return TRUE if given address belongs to current host
BOOL IsThisOurAddress (struct in_addr *pHost)
{
int             Ark;
char            szName [256];
struct hostent *pHostEntry;

    // search for an address which fit the pool
   if (       gethostname (szName, sizeof szName)!=SOCKET_ERROR
          && (pHostEntry=gethostbyname (szName)) !=0
          &&  pHostEntry->h_addr_list!=NULL
      )
   {
        for (Ark=0 ;  pHostEntry->h_addr_list[Ark]!=NULL ; Ark++ )
            if (* (DWORD *) pHostEntry->h_addr_list[Ark] == pHost->s_addr)
             return TRUE;
   }
return FALSE ;
} // IsThisOurAddress

/////
// scan the addresses of the server, returns the first which has the same subnet than the host
// if bExact is FALSE and the search was not successfull, the first address is returned
struct in_addr *FindNearestServerAddress (struct in_addr *pHost, struct in_addr *pMask, BOOL bExact)
{
int             Ark;
char            szName [256];
struct hostent *pHostEntry;
static DWORD    dwLoopback = 0x7F000001;  // loopback
char           *pLoopback = (char *) &dwLoopback;

    // search for an address which fit the pool
   if (       gethostname (szName, sizeof szName)!=SOCKET_ERROR
          && (pHostEntry=gethostbyname (szName)) !=0
          &&  pHostEntry->h_addr_list!=NULL
      )
   {
        for (Ark=0 ;
           pHostEntry->h_addr_list[Ark]!=NULL
             &&   ! bIPSameSubnet ((struct in_addr *)pHostEntry->h_addr_list[Ark], pHost, pMask);
           Ark++) ;
      if (pHostEntry->h_addr_list[Ark]!=NULL)
                return (struct in_addr *) pHostEntry->h_addr_list[Ark];
   }
return (struct in_addr *) ( bExact ? NULL : (Ark>0 ? pHostEntry->h_addr_list[0] : pLoopback) );
} //FindNearestServerAddress



#define MAXHADDRLEN 16
char * haddrtoa(const unsigned char *haddr, int hlen, char cSep)
{
static char haddrbuf[3 * MAXHADDRLEN + 1];
char *bufptr;

  if (hlen > MAXHADDRLEN)
        hlen = MAXHADDRLEN;

   bufptr = haddrbuf;
   while (hlen > 0) {
       wsprintf(bufptr, "%02X%c", (unsigned) (*haddr++ & 0xFF), cSep);
       bufptr += 3;
       hlen--;
   }
   bufptr[-1] = 0;
   return (haddrbuf);
} // haddrtoa

//Converts an ASCII string to a macaddr, assuming it was formed with haddrtoa
void atohaddr(const unsigned char *addrstr, unsigned char* haddr, int haddrlen)
{
   if(haddrlen == 6)
   {
      int b0, b1, b2, b3, b4, b5;
      b0=b1=b2=b3=b4=b5=0;
      sscanf_s (addrstr, "%x:%x:%x:%x:%x:%x", &b0, &b1, &b2, &b3, &b4, &b5);
      haddr[0] = (unsigned char) b0;
      haddr[1] = (unsigned char) b1;
      haddr[2] = (unsigned char) b2;
      haddr[3] = (unsigned char) b3;
      haddr[4] = (unsigned char) b4;
      haddr[5] = (unsigned char) b5;
   }
}

//Converts a time value to the "standard" date/time string -- works like haddrtoa
#define MAXTIMESTR  20  //"mm/dd/yyyy/00:00:00"/0
char* timetoa(time_t t)
{
   static char timebuf[MAXTIMESTR];
   struct tm *tt = localtime(&t);
   if (tt==NULL)  timebuf[0]=0;
   else           sprintf(timebuf, "%02d/%02d/%04d/%02d:%02d:%02d", 
                                    tt->tm_mon + 1, tt->tm_mday, tt->tm_year + 1900, 
                                    tt->tm_hour, tt->tm_min, tt->tm_sec);
   return timebuf;
}

//Converts our "standard" date/time string to a time_t
time_t atotime(char* str)
{
   struct tm t;
   memset(&t, 0, sizeof(t));
   t.tm_isdst = -1;  //For daylight savings calculation
#ifdef MSVC
   sscanf_s (str, "%d/%d/%d/%d:%d:%d", &t.tm_mon, &t.tm_mday, &t.tm_year, &t.tm_hour, &t.tm_min, &t.tm_sec);
#else
   sscanf(str, "%d/%d/%d/%d:%d:%d", &t.tm_mon, &t.tm_mday, &t.tm_year, &t.tm_hour, &t.tm_min, &t.tm_sec);
#endif
   t.tm_mon -= 1;
   t.tm_year -= 1900;
   return mktime(&t);
}


