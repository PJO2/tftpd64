//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Mai 98 Ph.jounin - Jan 2003
// File async_log.c:  Asynchronous multithread Log management
//
// source released under artistic license (see license.txt)
//
//////////////////////////////////////////////////////


#define LOGSIZE 512

// Synchronous log via OutputDebugString
void LogToMonitor (char *sz, ...);

