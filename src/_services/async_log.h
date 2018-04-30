//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Mai 98 Ph.jounin - Jan 2003
// File async_log.c:  Asynchronous multithread Log management
//
// source released under artistic license (see license.txt)
//
//////////////////////////////////////////////////////


#define MAX_MSG_IN_QUEUE 300
#define LOGSIZE 512

// Asynchronous log
void __cdecl LOG (int DebugLevel, const char *szFmt, ...);
void __cdecl SVC_ERROR (const char *szFmt, ...);
void __cdecl SVC_WARNING (const char *szFmt, ...);





