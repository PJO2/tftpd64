//////////////////////////////////////////////////////
//
// Projet TFTPD32.       Mai 98 Ph.jounin - June 2006
// File custom.h:   general definitions
//
// released under European Union Public License
// 
//////////////////////////////////////////////////////



//////////////////////////
// Env variables
//////////////////////////
#define TFTP_LOG  "TFTP_LOG"        // Log Level
#define TFTP_INI  "TFTP_INI"        // location of alternative inni file
#define TFTP_DIR  "TFTP_DIR"        // base directory
#define TFTP_HOST "TFTP_HOST"       // The host which runs the server
#define TFTP_PWD  "TFTP_PWD"        // The pwd to connect

// Builds
enum { STANDALONE_EDITION_VALUE = 10, SERVICE_EDITION_VALUE };
extern const int g_VERSION;

// Exchanged between GUI and services

#ifdef STANDALONE_EDITION
#  define CURRENT_PROTOCOL_VERSION_BASE 0x10000
#endif
#ifdef SERVICE_EDITION
#  define CURRENT_PROTOCOL_VERSION_BASE 0
#endif
#  define CURRENT_PROTOCOL_VERSION (12+CURRENT_PROTOCOL_VERSION_BASE)


//////////////////////////
// Commandes Générales
//////////////////////////

#define  MY_WARNING(a)  CMsgBox (hWnd, a, APPLICATION, MB_OK | MB_ICONEXCLAMATION)
#define  MY_ERROR(a)    \
( CMsgBox (hWnd, a, APPLICATION, MB_OK | MB_ICONSTOP), PostMessage (hWnd, WM_CLOSE, 0, 0) )
#define  PLURAL(a)  ((a)>1 ? "s" : "")

#define  SizeOfTab(x)   (sizeof (x) / sizeof (x[0]))
#define  MakeMask(x)    ( 1 << (x) )
#define  X_LOG2PHYS(x)  ( ((x) * LOWORD(GetDialogBaseUnits()) ) / 4 )
#define  Y_LOG2PHYS(y)  ( ((y) * HIWORD(GetDialogBaseUnits()) ) / 8 )


#define READKEY(x,buf) \
(dwSize = sizeof buf, RegQueryValueEx (hKey,x,NULL,NULL,(LPBYTE) & buf,&dwSize)==ERROR_SUCCESS)
#define SAVEKEY(x,buf,type) \
( RegSetValueEx (hKey,x, 0, type, \
                type==REG_SZ ?  (LPBYTE) buf : (LPBYTE) & buf,  \
                type==REG_SZ ? 1+lstrlen ((LPSTR) buf) : sizeof buf) == ERROR_SUCCESS)
#define REG_ERROR() MessageBox (NULL, "Error during registry access", "Tftpd32", MB_ICONEXCLAMATION | MB_OK)



#define  HELPFILE               "Tftpd32.chm"
#define  INI_FILE               "Tftpd32.ini"



//////////////////////////
// Commandes et paramètres TFTP
//////////////////////////

#  define SYSLOG_PORT           514
#  define SYSLOG_MAXMSG        5120
#  define DNS_MAXMSG           1024

#define  LOOPBACK_STRING        "127.0.0.1"
#define  DIR_TEXT_FILE          "dir.txt"
#define  DEFAULT_SYSLOG_FILE    "syslog.txt"
#define  DOMAIN_NAME_FILES      "hosts.txt"

#  define TFTP_TIMEOUT            3
#  define TFTP_RETRANSMIT         6
#  define TFTPD32_DEF_LOG_LEVEL   8
#  define TFTP_DEFPORT           69

#define TFTP_MAXRETRIES          50 // do not resent same block more than # times
#define TIME_FOR_LONG_TRANSFER   10 // more than 10 seconds -> beep


enum e_Services       { TFTPD32_NONE=0,
                        TFTPD32_TFTP_SERVER  = 0x0001,
                        TFTPD32_TFTP_CLIENT  = 0x0002,
                        TFTPD32_DHCP_SERVER  = 0x0004,
                        TFTPD32_SYSLOG_SERVER= 0x0008,
                        TFTPD32_SNTP_SERVER  = 0x0010,
                        TFTPD32_DNS_SERVER   = 0x0020,

						TFTPD32_CONSOLE      = 0x1000,
						TFTPD32_REGISTRY     = 0x2000,
						TFTPD32_SCHEDULER    = 0x4000,
                      };
// services presently provided
#define TFTPD32_ALL_SERVICES  (TFTPD32_TFTP_SERVER  | TFTPD32_TFTP_CLIENT |  TFTPD32_SYSLOG_SERVER | TFTPD32_DHCP_SERVER | TFTPD32_DNS_SERVER )
#define TFTPD32_MNGT_THREADS  ( TFTPD32_CONSOLE | TFTPD32_REGISTRY | TFTPD32_SCHEDULER )

// external files : .ini and .chm
extern char                     szTftpd32Help [MAX_PATH];// Full path of the help file
extern char                     szTftpd32IniFile [MAX_PATH];// Full path of the ini file
char *SetIniFileName (const char *szIniFile, char *szFullIniFile);

// service and GUI main :
void StartTftpd32Services (void *);
void StopTftpd32Services (void);
int GuiMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow);
