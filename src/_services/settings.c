//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Mai 98 Ph.jounin
// File tftp_sec.c:   Settings
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////

// registry key :
//       HKEY_LOCAL_MACHINE\SOFTWARE\TFTPD32

// some shortcurts

#include <stdio.h>
#include "headers.h"


static struct
{
  char *szEntry;
  void *pValue;
  int   nType;
  int   nBufSize;
}
tTftpd32Entry[] =
{
    { KEY_BASEDIR,                 sSettings.szBaseDirectory,           REG_SZ,      sizeof sSettings.szBaseDirectory   },
    { KEY_PORT,                  & sSettings.Port,                      REG_DWORD,   sizeof sSettings.Port              },
    { KEY_HIDE,                  & sSettings.bHide,                     REG_DWORD,   sizeof sSettings.bHide             },
    { KEY_WINSIZE,               & sSettings.WinSize,                   REG_DWORD,   sizeof sSettings.WinSize           },
    { KEY_NEGOCIATE,             & sSettings.bNegociate,                REG_DWORD,   sizeof sSettings.bNegociate        },
    { KEY_PXE,                   & sSettings.bPXECompatibility,         REG_DWORD,   sizeof sSettings.bPXECompatibility },
    { KEY_DIRTEXT,               & sSettings.bDirText,                  REG_DWORD,   sizeof sSettings.bDirText          },
    { KEY_PROGRESS,              & sSettings.bProgressBar,              REG_DWORD,   sizeof sSettings.bProgressBar      },
    { KEY_TIMEOUT,               & sSettings.Timeout,                   REG_DWORD,   sizeof sSettings.Timeout           },
    { KEY_MAXRETRANSMIT,         & sSettings.Retransmit,                REG_DWORD,   sizeof sSettings.Retransmit        },
    { KEY_DEFSECURITY,           & sSettings.SecurityLvl,               REG_DWORD,   sizeof sSettings.SecurityLvl       },
    { KEY_UNIX,                  & sSettings.bUnixStrings,              REG_DWORD,   sizeof sSettings.bUnixStrings      }, 
    { KEY_BEEP,                  & sSettings.bBeep,                     REG_DWORD,   sizeof sSettings.bBeep             },
    { KEY_VROOT,                 & sSettings.bVirtualRoot,              REG_DWORD,   sizeof sSettings.bVirtualRoot      },
    { KEY_MD5,                   & sSettings.bMD5,                      REG_DWORD,   sizeof sSettings.bMD5              },
    { KEY_TFTPLOCALIP,           sSettings.szTftpLocalIP,               REG_SZ,      sizeof sSettings.szTftpLocalIP     },
    { KEY_SERVICES,              & sSettings.uServices,                 REG_DWORD,   sizeof sSettings.uServices         },
    { KEY_TFTPLOGFILE,             sSettings.szTftpLogFile,             REG_SZ,      sizeof sSettings.szTftpLogFile     },
    { KEY_SYSLOGFILE,              sSettings.szSyslogFile,              REG_SZ,      sizeof sSettings.szSyslogFile      },
    { KEY_SYSLOGPIPE,            & sSettings.bSyslogPipe,               REG_DWORD,   sizeof sSettings.bSyslogPipe       },
    { KEY_LOWEST_PORT,           & sSettings.nTftpLowPort,              REG_DWORD,   sizeof sSettings.nTftpLowPort      },
    { KEY_HIGHEST_PORT,          & sSettings.nTftpHighPort,             REG_DWORD,   sizeof sSettings.nTftpHighPort     },
    { KEY_MCAST_PORT,            & sSettings.dwMCastPort,               REG_DWORD,   sizeof sSettings.dwMCastPort       },
    { KEY_MCAST_ADDR,              sSettings.szMCastAddr,               REG_SZ,      sizeof sSettings.szMCastAddr       },
    { KEY_PERS_LEASES,           & sSettings.bPersLeases,               REG_DWORD,   sizeof sSettings.bPersLeases       },
    { KEY_UNICAST_BOOTP,         & sSettings.bUnicastBOOTP,             REG_DWORD,   sizeof sSettings.bUnicastBOOTP     },
    { KEY_PING,                  & sSettings.bPing,                     REG_DWORD,   sizeof sSettings.bPing             },
    { KEY_DOUBLE_ANSWER,         & sSettings.bDoubleAnswer,             REG_DWORD,   sizeof sSettings.bDoubleAnswer      },
	{ KEY_DHCP_LOCIP,              sSettings.szDHCPLocalIP,             REG_SZ,      sizeof sSettings.szDHCPLocalIP     },
    { KEY_MAX_TRANSFERS,         & sSettings.dwMaxTftpTransfers,        REG_DWORD,   sizeof sSettings.dwMaxTftpTransfers },
	{ KEY_USE_EVENTLOG,          & sSettings.bEventLog,                 REG_DWORD,   sizeof sSettings.bEventLog         },
    { KEY_CONSOLE_PWD,             sSettings.szConsolePwd,              REG_SZ,      sizeof sSettings.szConsolePwd      },
    { KEY_PORT_OPTION,           & sSettings.bPortOption,               REG_DWORD,   sizeof sSettings.bPortOption       },
    { KEY_GUI_REMANENCE,         & sSettings.nGuiRemanence,             REG_DWORD,   sizeof sSettings.nGuiRemanence     },
    { KEY_IGNORE_LASTBLOCK_ACK,  & sSettings.bIgnoreLastBlockAck,       REG_DWORD,   sizeof sSettings.bIgnoreLastBlockAck     },
//	{ KEY_IPv4,                  & sSettings.bIPv4,                     REG_DWORD,   sizeof sSettings.bIPv4             },
	{ KEY_IPv6,                  & sSettings.bIPv6,                     REG_DWORD,   sizeof sSettings.bIPv6             },
	{ KEY_REDUCE_PATH,           & sSettings.bReduceTFTPPath,           REG_DWORD,   sizeof sSettings.bReduceTFTPPath   },

   { KEY_HTTP_DIR,              sSettings.szHttpDirectory,             REG_SZ,      sizeof sSettings.szHttpDirectory   },
};

#define READKEY(x,buf) \
(dwSize = sizeof buf, RegQueryValueEx (hKey,x,NULL,NULL,(LPBYTE) & buf,&dwSize)==ERROR_SUCCESS)
#define SAVEKEY(x,buf,type) \
( RegSetValueEx (hKey,x, 0, type, \
                type==REG_SZ ?  (LPBYTE) buf : (LPBYTE) & buf,  \
                type==REG_SZ ? 1+lstrlen ((LPSTR) buf) : sizeof buf) == ERROR_SUCCESS)



#define  RESET_DEFAULT_TEXT  "Reset current configuration\nand destroy registry entries ?"


struct S_Tftpd32Settings sSettings =
{
	  ".",                   // Base directory
	  TFTPD32_DEF_LOG_LEVEL, // Log level
	  TFTP_TIMEOUT,          // default timeout
	  TFTP_RETRANSMIT,       // def retransmission7
	  0,                     // WinSize
	  SECURITY_STD,          // Security
	  TFTP_DEFPORT,          // Tftp Port
	  FALSE,                 // Do not Hide
	  TRUE,                  // RFC 1782-1785 Negociation
	  FALSE,                 // PXE Compatibility
	  TRUE,                  // show progress bar ?
	  FALSE,                 // do not create dir.txt file
	  FALSE,                 // do not create MD5 file
	  FALSE,                 // do not resume
	  TRUE,                  // Unix like files "/tftpboot/.."
	  FALSE,                 // Do not beep for long transfert
	  FALSE,                 // Virtual Root is not enabled
	  "",                    // do not filter TFTP'slistening interface
	  TFTPD32_ALL_SERVICES,  // all services are enabled
	  0,  0,                 // use ports assigned by Windows
	  "",                    // No log file
	  "",                    // do not save syslog msg into a file
	  FALSE,                 // do not forward syslog msg to pipe
	  "",                    // Default mcast address
	  0,                     // Default mcast port
	  1,                     // persistant leases
	  0,                     // Unicast offer (BOOTP Compatibility)
	  1,                     // ping address before assignation
	  0,                     // Do not double answer
	  "",                    // do not filter DHCP'slistening interface
	  FALSE,				 // report errors into event log
	  DFLT_CONSOLE_PWD,      // console password
	  FALSE,                 // do not support port option
	  5,					 // after 5 seconds delete Tftp record
	  FALSE,				 // wait for ack of last TFTP packet
	  TRUE,					 // IPv4
	  TRUE,					 // IPv6
	  FALSE,				 // Reduce '//' in TFTP path
  
	  ".",					// Http Directory

	  // unsaved
	  100,                   // Max Simultaneous Transfers
	  ".",                   // Working Directory
	  2000,                  // refresh Interval
	  TFTPD32_TCP_PORT,		 // default port
};



/////////////////////////////////////////////////////////
// Read settings :
// Parse the settings table and call ReadKey
/////////////////////////////////////////////////////////
BOOL Tftpd32ReadSettings (void)
{
int Ark, Rc=TRUE;
char szCurDir[_MAX_PATH];
   for (Ark=0 ;   Ark< SizeOfTab (tTftpd32Entry) ;   Ark++)
     ReadKey (   TFTPD32_MAIN_KEY, 
                 tTftpd32Entry[Ark].szEntry,
                 tTftpd32Entry[Ark].pValue,
                 tTftpd32Entry[Ark].nBufSize,
                 tTftpd32Entry[Ark].nType,
                 szTftpd32IniFile );
  if (sSettings.uServices == TFTPD32_NONE)  sSettings.uServices = TFTPD32_ALL_SERVICES;
  
    // field WorkingDirectory : 
    // try env variable then szBaseDir and current dir
    if (    GetEnvironmentVariable (TFTP_DIR, szCurDir, sizeof szCurDir) != 0 
         && IsValidDirectory ( szCurDir ) )
                GetFullPathName ( szCurDir, 
                                  sizeof sSettings.szWorkingDirectory,
                                  sSettings.szWorkingDirectory, 
                                  NULL );
    else if ( IsValidDirectory ( sSettings.szBaseDirectory ) )
                GetFullPathName ( sSettings.szBaseDirectory, 
                                  sizeof sSettings.szWorkingDirectory,
                                  sSettings.szWorkingDirectory, 
                                  NULL );
    else        GetCurrentDirectory ( sizeof sSettings.szWorkingDirectory,
                                       sSettings.szWorkingDirectory );
// release for the Australian Department of defence
#ifdef BAE_SYSTEMS_LEN_WHITE
  sSettings.uServices = TFTPD32_TFTP_SERVER;
#endif
  // Tftp assigned to a physical interface ?
  sSettings.bTftpOnPhysicalIf = sSettings.szTftpLocalIP[0]!=0 && !isdigit (sSettings.szTftpLocalIP[0]);
return Rc;
} // Tftpd32ReadSettings


/////////////////////////////////////////////////////////
// Save Settings into ini file/registry
/////////////////////////////////////////////////////////
BOOL Tftpd32SaveSettings (void)
{
int Ark, Rc;

   for (Ark=0, Rc=TRUE ; Rc  &&  Ark< SizeOfTab (tTftpd32Entry) ; Ark++)
    Rc = AsyncSaveKey ( TFTPD32_MAIN_KEY, 
                   tTftpd32Entry[Ark].szEntry,                     
                   tTftpd32Entry[Ark].pValue,
                   tTftpd32Entry[Ark].nBufSize, 
                   tTftpd32Entry[Ark].nType,
                   szTftpd32IniFile );
    if (! Rc)  REG_ERROR();
return Rc;

} // Tftpd32SaveSettings


/////////////////////////////////////////////////////////
// Delete all Tftpd32's settings 
/////////////////////////////////////////////////////////
BOOL Tftpd32DestroySettings (void)
{
int   Rc;
FILE *f;
    // destroy keys
    RegDeleteKey (HKEY_LOCAL_MACHINE, TFTPD32_DHCP_KEY);
    Rc = RegDeleteKey (HKEY_LOCAL_MACHINE, TFTPD32_MAIN_KEY);
        
        // delete ini file
/*    if (szTftpd32IniFile[0]!=0)
    {
        f = fopen (szTftpd32IniFile, "wt");
        if (f!=NULL) fclose (f);
    }
*/
     if (szTftpd32IniFile[0]!=0)
    {
        fopen_s (&f, szTftpd32IniFile, "wt");
        if (f!=NULL) fclose (f);
    }       
return Rc==ERROR_SUCCESS;
}  // Tftpd32DestroySettings



