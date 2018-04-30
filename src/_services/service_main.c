//////////////////////////////////////////////////////
//
// Projet TFTPD32.   Feb 99 By  Ph.jounin
// File start_threads.c:  Thread management
//
// The main function of the service
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////


#include "headers.h"
#include <process.h>
#include "threading.h"
#include "bootpd_functions.h"


void StartTftpd32Services (void *param)
{
char sz[_MAX_PATH];

     // read log level (env var TFTP_LOG)
	if (GetEnvironmentVariable (TFTP_LOG, sz, sizeof sz)!=0)
          sSettings.LogLvl = atoi (sz);
    else  sSettings.LogLvl = TFTPD32_DEF_LOG_LEVEL;

    // Get the path in order to find the help file
    if (GetEnvironmentVariable (TFTP_INI, sz, sizeof sz)!=0)
          SetIniFileName (sz, szTftpd32IniFile);
    else  SetIniFileName (INI_FILE, szTftpd32IniFile);

    // Read settings (tftpd32.ini)
#ifndef TFTP_CLIENT_ONLY
    Tftpd32ReadSettings ();
#else
	sSettings.uServices = TFTPD32_TFTP_CLIENT;
#endif
//	DHCPReadConfig ();

    // starts worker threads
    StartMultiWorkerThreads (FALSE);
	LogToMonitor ("Worker threads started\n");
} // StartTftpd32Services

void StopTftpd32Services (void)
{
   TerminateWorkerThreads (FALSE);
}