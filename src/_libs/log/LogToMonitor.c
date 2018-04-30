//////////////////////////////////////////////////////
//
// Projet TFTPD32.  March 2007 Ph.jounin
// File logtomonitor.c:  multithread Log management
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////



#include <windows.h>
#include <stdio.h>
#include <stdarg.h>

#include "logtomonitor.h"


///////////////////////////////
// Synchronous log to debug string
// use DBwin to display debug strings
///////////////////////////////

void LogToMonitor (char *fmt, ...)
{
va_list args;
char sz [LOGSIZE];
int n;

    sz[sizeof sz - 1] = 0;
     va_start (args, fmt );
#ifdef MSVC
     n = sprintf_s (sz, sizeof sz - 1, "Th%5d :", GetCurrentThreadId ());
     vsprintf_s (& sz[n], sizeof sz - n - 1, fmt, args );
#else
     n = sprintf (sz, "Th%5d :", GetCurrentThreadId ());
     wvsprintf (& sz[n], fmt, args );
#endif
     OutputDebugString (sz);
    
}  // LogToFile
