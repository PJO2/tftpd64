//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Mars 2000 Ph.jounin
// File setinifilename.c: 
//			
//
// released under European Union Public License
// 
//////////////////////////////////////////////////////



#include "headers.h"

/* ----------------------------------- */
/* Determine the full name of ini file */
/*                                     */
/* From helmut giritzer                */
/* ----------------------------------- */
char *SetIniFileName (const char *szIniFile, char *szFullIniFile)
{
char *pszDummy = NULL;
char szAppBasePath [_MAX_PATH];
int  Ark, nLen;
const static struct S_Developpement
{  char *szPath; int len; } 
tDev[] = { { "\\DEBUG", sizeof "\\DEBUG" - 1 }, { "\\RELEASE", sizeof "\\RELEASE" - 1 }, { "\\LCC", sizeof "\\LCC" - 1 } };

     // GH: determine application (exe) path (e.g. to access INI file nearby)
   // Call GetModuleFileName function instead of __argv[0] to be compliant with LCC
   GetModuleFileName (NULL, szAppBasePath, sizeof szAppBasePath-1);
   pszDummy = strrchr( szAppBasePath, '\\' );
   if( NULL != pszDummy )
   {
        // get path component...
        szAppBasePath [pszDummy-szAppBasePath] = '\0';
        nLen = lstrlen (szAppBasePath);
        for (Ark=0 ; Ark < sizeof tDev / sizeof tDev[0] ; Ark++)
        {
          if( nLen >= tDev[Ark].len   &&
              0 == lstrcmpi ( & szAppBasePath[nLen - tDev[Ark].len],  tDev[Ark].szPath) )
          {
                 // cut of last DEBUG subdir...
                 szAppBasePath[nLen - tDev[Ark].len] = '\0';
                 break;
          }
        } // For
        lstrcat( szAppBasePath, "\\" );
     }
     // now build the full INI filename (expect it to be besides the exe file)...
     lstrcpy ( szFullIniFile, szAppBasePath );
     lstrcat ( szFullIniFile, szIniFile );
return szFullIniFile;
} // SetIniFileName

