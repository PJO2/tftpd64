//////////////////////////////////////////////////////
//
// Projet TFTPD32.  June 2006 Ph.jounin
// File eventlog.c : write event into event log
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////



#include <windows.h>
#include "eventlog.h"

int WriteIntoEventLog (const char *txt, WORD evId)
{
HANDLE hEvLog;
int    Rc;
const char *t[] = { "\n", txt }; 
	hEvLog = RegisterEventSource (NULL, "Tftpd32");
	if ( hEvLog != INVALID_HANDLE_VALUE )
	{
		Rc = ReportEvent ( hEvLog,
						   EVENTLOG_ERROR_TYPE,
						   0,			// wCategory
						   evId,		// identifier
					 	   NULL,		// security
						   sizeof t / sizeof t[0],		     // one string to add
						   lstrlen (txt),
					 	   t,
					 	   (void *) txt );
		DeregisterEventSource (hEvLog);
	}
return Rc;
} // WriteIntoEventLog
