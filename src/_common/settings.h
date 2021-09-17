
//////////////////////////////////////////////////////
// Projet TFTPD32.  Mars 2000 Ph.jounin
//
// File settings.h 
// settings structure declaration 
//
// released under artistic license (see license.txt)
// 
//////////////////////////////////////////////////////

#define DFLT_CONSOLE_PWD "tftpd32"

enum e_SecurityLevels { SECURITY_NONE, SECURITY_STD, SECURITY_HIGH, SECURITY_READONLY };

#define MAXLEN_IPv6 40

struct S_Tftpd32Settings
{
    char                  szBaseDirectory [_MAX_PATH];
    int                   LogLvl;
    unsigned              Timeout;
    unsigned              Retransmit;
    unsigned              WinSize;
    enum e_SecurityLevels SecurityLvl;
    unsigned              Port;
    BOOL                  bHide;
    BOOL                  bNegociate;
    BOOL                  bPXECompatibility;
    BOOL                  bProgressBar;
    BOOL                  bDirText;
    BOOL                  bMD5;
    BOOL                  bResumeOption;
    BOOL                  bUnixStrings;
    BOOL                  bBeep;
    BOOL                  bVirtualRoot;
	// changed in release 4 : szTftpLocalIP is either an IP address or an interface descriptor
    char                  szTftpLocalIP [max (MAXLEN_IPv6, MAX_ADAPTER_DESCRIPTION_LENGTH+4)];
    unsigned              uServices;
    unsigned              nTftpLowPort;
    unsigned              nTftpHighPort;
    char                  szTftpLogFile[_MAX_PATH];
    char                  szSyslogFile [_MAX_PATH];
    BOOL                  bSyslogPipe;
    char                  szMCastAddr[MAXLEN_IPv6];
    DWORD                 dwMCastPort;
    DWORD                 bPersLeases;
    DWORD                 bUnicastBOOTP;
	DWORD                 bPing;
	DWORD                 bDoubleAnswer;
    char                  szDHCPLocalIP [MAXLEN_IPv6];
	BOOL				  bEventLog;
    char                  szConsolePwd [12];    // password for GUI
	BOOL                  bPortOption;			// experimental port option
	DWORD				  nGuiRemanence;
	BOOL                  bIgnoreLastBlockAck;
	BOOL                  bIPv4;
	BOOL                  bIPv6;
	BOOL                  bReduceTFTPPath;

	char				  szHttpDirectory [_MAX_PATH];
	
	// unsaved settings
	DWORD				  dwMaxTftpTransfers;
    char                  szWorkingDirectory [_MAX_PATH];
    DWORD                 dwRefreshInterval;
	unsigned short		  uConsolePort;
	BOOL                  bTftpOnPhysicalIf;

	// should be last
	unsigned              uRunningServices;
};

extern struct S_Tftpd32Settings sSettings;          // The settings,used anywhere in the code
extern struct S_Tftpd32Settings sGuiSettings;

BOOL Tftpd32ReadSettings (void);
BOOL Tftpd32SaveSettings (void);
BOOL Tftpd32DestroySettings (void);

#define TFTPD32_BEFORE_MAIN_KEY "SOFTWARE"
#define TFTPD32_MAIN_KEY        "SOFTWARE\\TFTPD32"
// wish to create a executable which has a different registry key
#ifdef  TFTPD33
#  define TFTPD32_MAIN_KEY      "SOFTWARE\\TFTPD33"
#endif

#define KEY_WINDOW_POS      "LastWindowPos"


#define KEY_BASEDIR                "BaseDirectory"
#define KEY_DEFSECURITY            "SecurityLevel"
#define KEY_TIMEOUT                "Timeout"
#define KEY_MAXRETRANSMIT          "MaxRetransmit"
#define KEY_PORT                   "TftpPort"
#define KEY_HIDE                   "Hide"
#define KEY_WINSIZE                "WinSize"
#define KEY_NEGOCIATE              "Negociate"
#define KEY_PXE                    "PXECompatibility"
#define KEY_PROGRESS               "ShowProgressBar"
#define KEY_DIRTEXT                "DirText"
#define KEY_MD5                    "MD5"
#define KEY_UNIX                   "UnixStrings"
#define KEY_BEEP                   "Beep"
#define KEY_TFTPLOCALIP            "LocalIP"
#define KEY_SERVICES               "Services"
#define KEY_TFTPLOGFILE            "TftpLogFile"
#define KEY_SYSLOGFILE             "SaveSyslogFile"
#define KEY_SYSLOGPIPE             "PipeSyslogMsg"
#define KEY_VROOT                  "VirtualRoot"
#define KEY_LOWEST_PORT            "LowestUDPPort"
#define KEY_HIGHEST_PORT           "HighestUDPPort"
#define KEY_MCAST_PORT             "MulticastPort"
#define KEY_MCAST_ADDR             "MulticastAddress"
#define KEY_PERS_LEASES            "PersistantLeases"
#define KEY_UNICAST_BOOTP          "UnicastBOOTP"
#define KEY_DHCP_LOCIP             "DHCP LocalIP"
#define KEY_PING                   "DHCP Ping"
#define KEY_DOUBLE_ANSWER          "DHCP Double Answer"
#define KEY_MAX_TRANSFERS          "Max Simultaneous Transfers"
#define KEY_USE_EVENTLOG           "UseEventLog"
#define KEY_CONSOLE_PWD            "Console Password"
#define KEY_PORT_OPTION            "Support for port Option"
#define KEY_GUI_REMANENCE          "Keep transfer Gui"
#define KEY_IGNORE_LASTBLOCK_ACK   "Ignore ack for last TFTP packet"
#define KEY_IPv4                   "Enable IPv4"
#define KEY_IPv6                   "Enable IPv6"
#define KEY_REDUCE_PATH			   "Reduce TFTP Path"
#define KEY_DONOTCHECK_FIREWALL    "Donot verify firewall"

#define KEY_HTTP_DIR                "HttpBaseDirectory"


#define TFTPD32_DHCP_KEY              "SOFTWARE\\TFTPD32\\DHCP"
#define KEY_DHCP_POOL                 "IP_Pool"
#define KEY_DHCP_POOLSIZE             "PoolSize"
#define KEY_DHCP_BOOTFILE             "BootFile"
#define KEY_DHCP_DNS                  "DNS"
#define KEY_DHCP_DNS2                 "DNS2"
#define KEY_DHCP_WINS                 "WINS"
#define KEY_DHCP_MASK                 "Mask"
#define KEY_DHCP_DEFROUTER            "Gateway"
#define KEY_DHCP_OPTION42             "Option42"
#define KEY_DHCP_OPTION120            "Option120"
#define KEY_DHCP_DOMAINNAME           "DomainName"
#define KEY_DHCP_LEASE_TIME           "Lease (minutes)"
#define KEY_DHCP_USER_OPTION_NB       "AddOptionNumber"
#define KEY_DHCP_USER_OPTION_VALUE    "AddOptionValue"
#define KEY_DHCP_USER_OPTION_NB_1     "AddOptionNumber1"
#define KEY_DHCP_USER_OPTION_VALUE_1  "AddOptionValue1"

#define KEY_LEASE_NUMLEASES           "Lease_NumLeases"
#define KEY_LEASE_PREFIX              "Lease_"
#define KEY_LEASE_MAC                 "_MAC"
#define KEY_LEASE_IP                  "_IP"
#define KEY_LEASE_ALLOC               "_InitialOfferTime"
#define KEY_LEASE_RENEW               "_LeaseStartTime"
