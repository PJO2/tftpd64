//////////////////////////////////////////////////////
//
// Projet TFTPD32.   Mai 98 Ph.jounin
// File SVC main.c:  The MAIN program for the service edition
//
// derivative work from sdk_service.cpp 
//                 by Craig Link - Microsoft Developer Support
// 
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////


#include "headers.h"
#include <stdio.h>
#include "service stuff.h"

// A few global variables
char      szTftpd32IniFile [MAX_PATH];  // Full Path for INI file



// internal variables

BOOL                    bDebug = FALSE;


//////////////////////////////////////////////////////
//
//  Starts services and main window
//
//////////////////////////////////////////////////////



/* ----------------------------- */
/* WinMain                       */
/* ----------------------------- */
#ifdef WINMAIN
int PASCAL WinMain ( HINSTANCE hInstance, 
					 HINSTANCE hPrevInstance,
                     LPSTR lpszCmdLine, 
					 int nCmdShow )
#endif
int main (int argc, char *argv[])
{
LPSTR lpszCmdLine = argv[1];
static const SERVICE_TABLE_ENTRY dispatchTable[] =
{
        { SZSERVICENAME, (LPSERVICE_MAIN_FUNCTION) service_main },
        { NULL, NULL }
 };

  // ------------------------------------------
  // Actions :
  //      install as service
  //      remove as service
  //      start in dos box
  //      uninstall
  // ------------------------------------------
  if ( lstrcmpi( "-install", lpszCmdLine ) == 0 )
  {
      CmdInstallService();
  }
  else if ( lstrcmpi( "-remove", lpszCmdLine ) == 0 )
  {
      CmdRemoveService();
  }
  else if ( lstrcmpi( "-debug", lpszCmdLine ) == 0 )
  {
      bDebug = TRUE;
      CmdDebugService();
  }
  else if (lstrcmpi ("-uninstall", lpszCmdLine) == 0)
  {
   	  // destroy the registry entries but not the ini file 
      Tftpd32DestroySettings ();
  }
  else 
  {
	    // this is just to be friendly
        printf ( "%s -install          to install the service\n", APPLICATION );
        printf ( "%s -remove           to remove the service\n", APPLICATION );
        printf ( "%s -debug <params>   to run as a console app for debugging\n", APPLICATION );
		printf ( "%s -uninstall        to suppress registry entries and settings\n", APPLICATION );
        printf ( "\nStartServiceCtrlDispatcher being called.\n" );
        printf ( "This may take several seconds.  Please wait.\n" );

        if (!StartServiceCtrlDispatcher(dispatchTable))
		{  
			LogToMonitor ("StartServiceCtrlDispatcher failed. Error %d", GetLastError ());
            AddToMessageLog ("StartServiceCtrlDispatcher failed.");
		}
  }
return 0;
} // WinMain


void ServiceStart (void)
{
int Rc;
WSADATA WSAData;
  // ------------------------------------------
  // Start the App
  // ------------------------------------------
     	 
     Rc = WSAStartup (MAKEWORD(2,0), & WSAData);
     if (Rc != 0)
     {
         CMsgBox (NULL, 
				  GetLastError()==WSAVERNOTSUPPORTED ?
					    "Error: Tftpd32 now requires winsock version 2" :
						"Error: Can't init Winsocket", 
				   APPLICATION, 
				   MB_OK | MB_ICONERROR);
     }
	 else
	 {
 		 // start Services
		 StartTftpd32Services (NULL);
		 LogToMonitor ("Tftpd32 Service Edition is ready\n");
		 while (bDebug) { Sleep (10000); LogToMonitor ("Still Alive\n"); }
	 } 
return ;
} /* ServiceStart */

void ServiceStop (void)
{
	StopTftpd32Services ();
	Sleep (1500);
	WSACleanup ();
	LogToMonitor ("Tftpd32 service edition has ended\n");
} // ServiceStop


