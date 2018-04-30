//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Dec 2008 Ph.jounin
// File DNS.c:  Domain name server
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////

#include "headers.h"
#include "threading.h"

#define IPv6

#define MAX_IPV6_ADDRESS 10
#define DNS_CNAME     5			// canonical name	
#define DNS_A         1			// ipv4 translation
#define DNS_AAAA      2		// ipv6 translation
#define DNS_IPV4_ADDR 1
#define DNS_IPV6_ADDR 2
#define CLASS_IN      1			// internet
#define DNS_NOTIMPLEMENTED 4

#pragma pack(1)

typedef struct
{
	unsigned short qtype;
	unsigned short qclass;
} DNS_QUESTION;

typedef struct
{
	unsigned short type;
	unsigned short _class;
	unsigned int ttl;
	unsigned short data_len;
} DNS_R_DATA;

typedef struct
{
	unsigned char  name[2];
	DNS_R_DATA     resource;
	unsigned char  rdata [0];
} DNS_RES_RECORD;

typedef struct
{
	unsigned char *name;
	DNS_QUESTION *ques;
} DNS_QUERY;

typedef struct
{
   unsigned short id;       // identification number
   unsigned char rd :1;     // recursion desired
   unsigned char tc :1;     // truncated message
   unsigned char aa :1;     // authoritive answer
   unsigned char opcode :4; // purpose of message
   unsigned char qr :1;     // query/response flag
   unsigned char rcode :4;  // response code
   unsigned char cd :1;     // checking disabled
   unsigned char ad :1;     // authenticated data
   unsigned char z :1;      // its z! reserved
   unsigned char ra :1;     // recursion available
   unsigned short q_count;  // number of question entries
   unsigned short ans_count; // number of answer entries
   unsigned short auth_count; // number of authority entries
   unsigned short add_count; // number of resource entries
} DNS_HEADER;

#pragma pack()



// the structure in a more readable format
typedef struct
{
	char           name [NI_MAXHOST];
	unsigned short qtype;
	unsigned short qclass;
} DNS_QUESTION_EXP;


typedef struct
{
	int               nb_ipv6_address;
	char              cname [NI_MAXHOST];
	long              ipv4;
	IN6_ADDR          ipv6 [MAX_IPV6_ADDRESS];
	unsigned short    qtype;
	unsigned short    qclass;
} DNS_ANSWER_EXP;

static int ExplodeQname (DNS_QUESTION_EXP *req, const char *qname, int len);
int AnswerDnsQuery (const DNS_HEADER *header, int len, char *buf, int *anslen);
static int ProcessDnsQuery (const DNS_QUESTION_EXP *req, DNS_ANSWER_EXP *ans);
static int CreateAnswerMsg (const DNS_HEADER *req,
							int len,
							DNS_QUESTION_EXP *reqexp,
							DNS_ANSWER_EXP *ans, 
							DNS_HEADER *msg,
							int *anslen);

/////////////////////////////
// Background window 
//
void ListenDNSMessage (void * param)
{
SOCKET           sDNSSocket=INVALID_SOCKET;
int              Rc;
SOCKADDR_STORAGE sSock;
int              nDummy;
char Buf [DNS_MAXMSG], answer [DNS_MAXMSG];
int anslen=0;

   sDNSSocket  = tThreads[TH_DNS].skt;

   
   tThreads [TH_DNS].bInit = TRUE;  // inits OK
   while ( tThreads[TH_DNS].gRunning )
   {
      nDummy = sizeof sSock;
      // get the message, checks its format and display it
      Rc = recvfrom (sDNSSocket, Buf, DNS_MAXMSG, 0, (struct sockaddr *) & sSock, & nDummy);
	  if (Rc>0)
	  { 
		  if (AnswerDnsQuery ( (DNS_HEADER*)Buf, Rc, answer, & anslen ))
		  {
		    Rc = sendto (sDNSSocket, answer, anslen, 0, (struct sockaddr *) & sSock, sizeof sSock);
			LogToMonitor ("send %d/%d bytes", anslen/Rc);
		  }
	  }
	  if (Rc<0)
	  {
		  LogToMonitor ("erreur %d during socket operation", GetLastError () );
		  Sleep (100);
	  }

   }
   LogToMonitor ("End of DNS thread\n");
_endthread ();
} // ListenDNSMessage



int AnswerDnsQuery (const DNS_HEADER *header, int len, char *buf, int *anslen)
{
DNS_QUESTION_EXP sDNSQuestion;
DNS_ANSWER_EXP  ans;

int Rc;

	memset (& ans, 0, sizeof ans);
   // check that message is a query and only one host is required
	if (htons (header->opcode) == 0 && htons (header->q_count)==1)
	{
		// go to request 
		Rc = ExplodeQname (& sDNSQuestion, 
					 	     (char *) header + sizeof (DNS_HEADER), 
						 	 len - sizeof (DNS_HEADER) );
		if (Rc && ( sDNSQuestion.qtype==DNS_A || sDNSQuestion.qtype==DNS_AAAA || sDNSQuestion.qtype==DNS_CNAME )
			   && sDNSQuestion.qclass==CLASS_IN)
		{
			ProcessDnsQuery (& sDNSQuestion, &ans);
			CreateAnswerMsg (  header, len, & sDNSQuestion, 
				             & ans, (DNS_HEADER *) buf, anslen);
		}
		
	} // complex request ?
	else
	{
		LOG (5, "Complex DNS request : ignored");
		return 0;
	}
return 1;
} // AnswerDnsQuery


// Translate a qname string into a DNS_QUESTION_EXP structure
static int ExplodeQname (DNS_QUESTION_EXP *req, const char *qname, int len)
{
int Ark;
int host_len, n ;
char *p;
const char *q;

  host_len = strnlen (qname, len)+1;

  // drop misformed requests
  if (     len > sizeof *req  
	   ||  host_len> sizeof req->name - 1
	   ||  host_len != len - sizeof (DNS_QUESTION) )  
	   return FALSE;


  // recompose the host name
  for ( Ark=0, q=qname, p=req->name, --host_len ;  host_len>0 &&  *q!=0 ;  )
  {
      n = *q;
	  host_len -= *q++;
	  if (host_len >= 0)
	     while (n--)  *p++ = *q++;
	  if (--host_len>0) *p++ = '.';
  } 
  *p = 0;
  q++;
  req->qclass = htons ( ((DNS_QUESTION *) q)->qclass ) ;
  req->qtype =  htons ( ((DNS_QUESTION *) q)->qtype ) ;
return TRUE;
} // Explode Qname


#ifdef IPv6
static int ProcessDnsQuery (const DNS_QUESTION_EXP *req, DNS_ANSWER_EXP *ans)
{
struct addrinfo *info = NULL, *cur;
int Rc;
int n = 0;
	memset (ans, 0, sizeof *ans);
	ans->ipv4 = INADDR_NONE;

	Rc = getaddrinfo(req->name, NULL, NULL, &info);
    if (Rc==0  && info!=NULL)
	{
       for (cur = info; cur != NULL; cur = cur->ai_next)
       {   
	 	    if (ans->cname[0]==0 && cur->ai_canonname!=NULL) 
				strcpy (ans->cname, cur->ai_canonname);
			switch (cur->ai_family) 
			{
			   case AF_INET:  
				   ans->ipv4 = ((struct sockaddr_in *) cur->ai_addr)->sin_addr.s_addr;
				   break;
			   case AF_INET6 :

				   if (ans->nb_ipv6_address+1  < MAX_IPV6_ADDRESS) 
					   memcpy (& ans->ipv6[ans->nb_ipv6_address++],  cur->ai_addr, cur->ai_addrlen);
				   break;
			}
	   } // search ipv4 and ipv6 records
	   ans->qclass = req->qclass;
	   ans->qtype = req->qtype;
	   freeaddrinfo (info);
	   return TRUE;
	}
	else
	{
		Rc = GetLastError ();
	}
return FALSE;
} // ProcessDnsQuery

#else
static int ProcessDnsQuery (const DNS_QUESTION_EXP *req, DNS_ANSWER_EXP *ans)
{
struct hostent *  lpHostEnt;
int Rc;
   lpHostEnt = gethostbyname (req->name);
   if (lpHostEnt!=NULL)
   {
       memcpy (& ans->ipv4 , lpHostEnt->h_addr, lpHostEnt->h_length);
	   ans->qclass = req->qclass;
	   ans->qtype = req->qtype;

	   return TRUE;
	}
	else
	{
		Rc = GetLastError ();
	}
return FALSE;
} // ProcessDnsQuery
#endif  // IPv6 or IPv4


static int CreateAnswerMsg (const DNS_HEADER *req,
							int len,
							DNS_QUESTION_EXP *reqexp,
							DNS_ANSWER_EXP *ans, 
							DNS_HEADER *msg,
							int *anslen)
{
DNS_RES_RECORD *cur;
int nb_ans = 0;
struct S_DNS_NewEntry guimsg;

   // copy request message
   memcpy (msg, req, len);
   msg->qr = 1;
   // add cname answer
   (char *) cur = (char *) msg + len;
   if (ans->cname[0] != 0)
   {
	   nb_ans++;
	   cur->name [0] = 0xc0;
	   cur->name [1] = 0x0c;
	   cur->resource._class = htons (CLASS_IN);
	   cur->resource.ttl = htonl (300);
	   cur->resource.type = htons (DNS_CNAME);
	   cur->resource.data_len = htons (strlen (ans->cname));
	   strcpy (cur->rdata, ans->cname);
	   (char *) cur += sizeof (*cur) + htons (cur->resource.data_len);
   }
   if (ans->ipv4 != INADDR_NONE  &&  ans->ipv4 != INADDR_ANY )
   {
	   nb_ans++;
	   cur->name [0] = 0xc0;
	   cur->name [1] = 0x0c;
	   cur->resource._class = htons (CLASS_IN);
	   cur->resource.ttl = htonl (300);
	   cur->resource.type = htons (DNS_A);
	   cur->resource.data_len = htons (sizeof (long));
	   * (long *) cur->rdata = (long) ans->ipv4;
	   (char *) cur += sizeof (*cur) + sizeof (long);
   }
   if (! IN6_IS_ADDR_UNSPECIFIED (ans->ipv6))
   {
	   nb_ans++;
	   msg->ans_count = htons ( htons(msg->ans_count) + 1);
	   cur->name [0] = 0xc0;
	   cur->name [1] = 0x0c;
	   cur->resource._class = htons (CLASS_IN);
	   cur->resource.ttl = htonl (300);
	   cur->resource.type = htons (DNS_AAAA);
	   cur->resource.data_len = htons (sizeof (IN6_ADDR));
	   * (IN6_ADDR *) cur->rdata = ans->ipv6[0];
	   (char *) cur += sizeof (*cur) + sizeof (IN6_ADDR);
   }

    msg->ans_count = htons (nb_ans);
    *anslen = (char *) cur - (char *) msg;
	// if no answer send an error code
	if (nb_ans == 0)
	{
		msg->rcode = DNS_NOTIMPLEMENTED;
	}
	else
	{
		// send message to GUI   
	   lstrcpy (guimsg.name, reqexp->name);
	   lstrcpy (guimsg.ipv4, inet_ntoa (* (struct in_addr *) & ans->ipv4));
	   guimsg.ipv6[0] = 0;
	   SendMsgRequest (   C_DNS_NEW_ENTRY, 
  						  & guimsg, 						
						  sizeof guimsg,
						  FALSE,	  	    // don't block thread until msg sent
						  FALSE );		// if no GUI return
	}   
return TRUE;
}

