///////////////////////////////////////////////////////////////////////////////////////
//
// dialog_socket.h
//
// describes all the messages exchanged between the services and its console
//
// released under artistic license (see license.txt)
// 
///////////////////////////////////////////////////////////////////////////////////////



#define TFTPD32_TCP_PORT    2994
#define LOGSIZE              512

enum e_Types
{
    
    C_LOG               = 1,
	C_WARNING,
	C_ERROR,

    // msg sent from the service to the GUI
    C_TFTP_TRF_NEW      =  100,
    C_TFTP_TRF_END,
    C_TFTP_TRF_STAT,
    C_DHCP_LEASE,
    C_TFTP_RPLY_SETTINGS,
    C_DHCP_RPLY_SETTINGS,       
    C_REPLY_WORKING_DIR,    // working_dir
    C_SYSLOG,               // syslog message available
	C_REPLY_GET_SERVICES,   // get the running services
	C_REPLY_GET_INTERFACES,      // server ip addresses 
	C_SERVICES_STARTED,          // init done
    C_REPLY_DIRECTORY_CONTENT,   // list directory
	C_DNS_NEW_ENTRY,              // DNS request
	C_CHG_SERVICE,				  // Service has been started/stopped

    // msg sent from the GUI to the service 
    C_CONS_KILL_TRF     = 200,
    C_TFTP_TERMINATE,
    C_DHCP_TERMINATE,
	C_SYSLOG_TERMINATE,
	C_SNTP_TERMINATE,
	C_DNS_TERMINATE,
    C_TERMINATE,			// kill threads (terminating)
	C_SUSPEND,              // kill worker services
    C_START,				// start services
    C_DHCP_RRQ_SETTINGS,
    C_TFTP_RRQ_SETTINGS,
    C_DHCP_WRQ_SETTINGS,
    C_TFTP_WRQ_SETTINGS,
    C_TFTP_RESTORE_DEFAULT_SETTINGS,  // remove all settings
    C_TFTP_CHG_WORKING_DIR,			  // working_dir
    C_RRQ_WORKING_DIR,          // empty
    C_DELETE_ASSIGNATION, 
	C_RRQ_GET_SERVICES,		// Request the running services
	C_RRQ_GET_DHCP_ALLOCATION,  // number of allocation
	C_RRQ_GET_INTERFACES,       // IP interfaces
    C_RRQ_DIRECTORY_CONTENT,
	C_TFTP_GET_FULL_STAT,		// request statistics

} ;

// server's interfaces
struct S_ItfEntry
{
	char sz [MAX_ADAPTER_DESCRIPTION_LENGTH+1];
	int  status;
}; // S_IfEntry
// address owned by the server
struct S_IPAddressEntry
{
	int    idx;				// points on the interface entry
	char sz [MAXLEN_IPv6];
};
struct S_IPAddressList
{
#define MAX_IP_ADDR 30
#define MAX_ITF     12
	int                      nb_itf;
	int                      nb_addr;
	struct S_IPAddressEntry  addr [MAX_IP_ADDR];
	struct S_ItfEntry        itf [MAX_ITF];

}; // S_IPAddressList

// A new transfer has begun
struct S_TftpTrfNew
{
   DWORD dwTransferId;
   struct S_Trf_Statistics stat;
   int   opcode;
   SOCKADDR_STORAGE from_addr;
   char  szFile [_MAX_PATH];
};
// A transfer has ended
struct S_TftpTrfEnd
{
   DWORD dwTransferId;
   struct S_Trf_Statistics stat;
};

// Stat of current trf
struct subStats
{
   DWORD   dwTransferId;
   /* struct S_Trf_Statistics read from tfpt_struct */
   struct S_Trf_Statistics stat;
}; 
// transfer statistics
struct S_TftpTrfStat
{
    int nbTrf;      // nb de tranferts
    time_t  dNow;   // current time
    struct subStats t[101];
};

// abort a TFTP transfer
struct S_TftpKill
{
   DWORD dwTransferId;
} ; 

// DHCP report --> leases
struct S_Lease
{
   char  szIP [sizeof "255.255.255.255"];
   char  szMAC [sizeof "aa.BB.CC.DD.EE.ff"]; // MAC Address of the client
   time_t             tAllocated;      // time of assignation 
   time_t             tRenewed;        // time of client ack
}; 
// the first DHCP leases
struct S_DhcpLeases
{
    int nb;
    struct S_Lease l [50];
}; // struct S_Lease
// kill a lease
struct S_DhcpSuppressLease
{
  unsigned ip; 
};
// syslog msg transferred from service to GUI
struct S_SyslogMsg
{
    char from [MAXLEN_IPv6];
    char txt [SYSLOG_MAXMSG + 1];
}; 

// send directory content
struct S_DirEntry
{
    char   file_descr [64];
};
struct S_DirectoryContent
{
    int nb;
    struct S_DirEntry  ent[200];
}; // S_DirectoryEntry


// DNS request
struct S_DNS_NewEntry
{
    char   name [NI_MAXHOST];
	char   ipv4 [sizeof "255.255.255.255"];
	char   ipv6 [MAXLEN_IPv6];
}; // S_DNS_NewEntry

// beginning or end of a service
struct S_Chg_Service
{
	int service;   // id of the service TFTPD32_TFTP_SERVER, TFTPD32_DHCP_SERVER, ...
	int status;    // either SERVICE_RUNNING or SERVICE_STOPPED
}; // struct S_Chg_Service



struct S_ConsoleMsg
{
    int     type;
    union
    {
        // struct S_ServiceVersion ver;
		struct S_IPAddressList address;

        struct S_TftpTrfNew  trf_new;
        struct S_TftpTrfEnd  trf_end;
        struct S_TftpTrfStat trf_stat;
        struct S_TftpKill    kill;
        struct S_Tftpd32Settings tftp_settings;
        struct S_DHCP_Param  dhcp_settings;
        

        struct S_DhcpLeases  dhcp_lease;
        struct S_SyslogMsg   syslog_msg;
        struct S_DhcpSuppressLease del_lease;

        struct S_DirectoryContent  dir;

		struct S_DNS_NewEntry      dns;
		struct S_Chg_Service       chg;

        char                 log [LOGSIZE];
		char                 error [LOGSIZE];
		char                 warning [LOGSIZE];
        char                 working_dir [_MAX_PATH];
		int                  uServices;				// reply running services
    } u;
} ;  // the main structure


