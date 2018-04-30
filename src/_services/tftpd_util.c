
//
// source released under European Union Public License
//

#include "headers.h"
#include "tftpd_functions.h"

//////////////////////////////////////////
// creates dir.txt files
//////////////////////////////////////////
static int CbkWrite (char *szLine, DWORD dw)
{
DWORD Dummy;
static char EOL [] = "\r\n";
       WriteFile ((HANDLE) dw, szLine, lstrlen (szLine), &Dummy, NULL);
       WriteFile ((HANDLE) dw, EOL, sizeof (EOL)-1, &Dummy, NULL);
       return 0;
}

int CreateIndexFile (void)
{
HANDLE           hDirFile;
static int       Semaph=0;
char szDirFile [_MAX_PATH];

   if (Semaph++!=0)  return 0;

   wsprintf (szDirFile, "%s\\%s", sSettings.szWorkingDirectory, DIR_TEXT_FILE);
    hDirFile =  CreateFile (szDirFile,
                            GENERIC_WRITE,
                            FILE_SHARE_READ,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_SEQUENTIAL_SCAN ,
                            NULL);
    if (hDirFile == INVALID_HANDLE_VALUE) return 0;
    // Walk through directory
    ScanDir (CbkWrite, (DWORD) hDirFile, sSettings.szWorkingDirectory);
    CloseHandle (hDirFile);
    Semaph = 0;
return 1;
} // CreateIndexFile
