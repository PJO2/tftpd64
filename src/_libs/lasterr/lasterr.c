//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Mars 2000 Ph.jounin
// File lasterr.c:   function LastErrorText 
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////

#include <windows.h>
#include "lasterr.h"


// Function LastErrorText
// A wrapper for FormatMessage : retrieve the message text for a system-defined error 
char *LastErrorText (void)
{
static char szLastErrorText [512];
LPVOID      lpMsgBuf;
LPSTR       p;

   FormatMessage(
         FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
         NULL,
         GetLastError(),
         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
         (LPTSTR) &lpMsgBuf,
         0,
         NULL );
   memset (szLastErrorText, 0, sizeof szLastErrorText);
   lstrcpyn (szLastErrorText, lpMsgBuf, sizeof szLastErrorText);
    // Free the buffer.
   LocalFree( lpMsgBuf );
   // remove ending \r\n
   p = strchr (szLastErrorText, '\r');
   if (p!=NULL)  *p = 0;
return szLastErrorText;
} // LastErrorText


