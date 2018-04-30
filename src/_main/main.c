//////////////////////////////////////////////////////
//
// Projet TFTPD32.   Mai 98 Ph.jounin
// File tftp_mai.c:  The MAIN program
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////


#include "headers.h"

//////////////////////////////////////////////////////
//
//  Starts services and main window
//
//////////////////////////////////////////////////////

// A few global variables
char      szTftpd32Help [MAX_PATH];     // Full Path for Help file
char      szTftpd32IniFile [MAX_PATH];  // Full Path for INI file


/* ----------------------------- */
/* WinMain                       */
/* ----------------------------- */
int PASCAL WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpszCmdLine, int nCmdShow)
{
WSADATA               WSAData;
int                   Rc;

  // ------------------------------------------
  // Quick Uninstaller
  // ------------------------------------------
  if (lstrcmpi ("-uninstall", lpszCmdLine) == 0)
  {
	// destroy the registry entries but not the ini file 
    Tftpd32DestroySettings ();
    return 0;
  }

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
         return FALSE;
     }


	 // start Services through a thread
	 // the thread start the console, wait for 200ms
	 // (time to start the GUI) then starts the services
	 // this way, an early call to the SVC_ERROR procedure happens
	 // when the console/GUI socket is connected.
	 _beginthread ( StartTftpd32Services, 0, NULL );
	 // let the console opens its socket
	 // Pause needs to be higher than the one in StartMultiWorkerThreads (start_threads.c)
	 // otherwise DHCP settings will not be loaded in GUI
	 //Sleep (500);
     // opens Gui
     GuiMain (hInstance, hPrevInstance,lpszCmdLine, nCmdShow);

     WSACleanup ();
	LogToMonitor ("That's all folks\n");
return 0;
} /* WinMain */


