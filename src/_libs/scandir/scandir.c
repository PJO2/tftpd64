//////////////////////////////////////////////////////
//
// Projet TFTPD32.         January 2006 Ph.jounin
// PING_Scandir.c -- Display content of current directory
//
//
// source released under European Union Public License
// 
//////////////////////////////////////////////////////

#include <windows.h>
#include "scandir.h"


int IsValidDirectory (const char *path)
{

int Rc ; 
   Rc = GetFileAttributes (path) ;
   return Rc == INVALID_FILE_ATTRIBUTES ? FALSE : Rc & FILE_ATTRIBUTE_DIRECTORY ;
                                          
} // IsValidDirectory


////////////////////////
// Creates a line of the dir.txt file
// use a callback function as argument since ScanDir is used either to
// create dir.txt or to dispaly the dir window

void ScanDir ( int (*f)(char *s, DWORD dw), DWORD dwParam, const char *szDirectory)
{
WIN32_FIND_DATA  FindData;
FILETIME    FtLocal;
SYSTEMTIME  SysTime;
char        szLine [256], szFileSpec [_MAX_PATH + 5];
char        szDate [sizeof "jj/mm/aaaa"];
HANDLE      hFind;

    szFileSpec [_MAX_PATH - 1] = 0;
    lstrcpyn (szFileSpec, szDirectory, _MAX_PATH);
    lstrcat (szFileSpec, "\\*.*");
    hFind = FindFirstFile (szFileSpec, &FindData);
    if (hFind !=  INVALID_HANDLE_VALUE)
    do
    {
       // display only files, skip directories
       if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)  continue;
       FileTimeToLocalFileTime (& FindData.ftCreationTime, & FtLocal);
       FileTimeToSystemTime (& FtLocal, & SysTime);
       GetDateFormat (LOCALE_SYSTEM_DEFAULT,
                      DATE_SHORTDATE,
                      & SysTime,
                      NULL,
                      szDate, sizeof szDate);
       szDate [sizeof "jj/mm/aaaa" - 1]=0;    // truncate date
       FindData.cFileName[62] = 0;      // truncate file name if needed
	   // dialog structure allow up to 64 char
       wsprintf (szLine, "%s\t%s\t%d",
                 FindData.cFileName, szDate, FindData.nFileSizeLow);

       (*f) (szLine, dwParam);
    }
    while (FindNextFile (hFind, & FindData));

    FindClose (hFind);

}  // ScanDir


