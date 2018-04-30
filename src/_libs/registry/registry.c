//////////////////////////////////////////////////////
//
// Projet DHCPD32.  January 2006 Ph.jounin
//                  A free DHCP server for Windows
// File registry.c: get configuration from either ini file or registry
//
//
// source released under European Union Public License
// 
//////////////////////////////////////////////////////


#include <windows.h>
#include <string.h>

#define READKEY(x,buf) \
(dwSize = sizeof buf, RegQueryValueEx (hKey,x,NULL,NULL,(LPBYTE) & buf,&dwSize)==ERROR_SUCCESS)
#define SAVEKEY(x,buf,type) \
( RegSetValueEx (hKey,x, 0, type, \
                type==REG_SZ ?  (LPBYTE) buf : (LPBYTE) & buf,  \
                type==REG_SZ ? 1+lstrlen ((LPSTR) buf) : sizeof buf) == ERROR_SUCCESS)


// retrieve the szKey setting
// sxzRegPath is either the key path for registry or the INI section

int ReadKey (const char *szRegPath, const char *szKey, 
             void *buf, int BufSize, int nType,
             const char *szIniFile)
{
int   Rc = FALSE;
HKEY  hKey=INVALID_HANDLE_VALUE;
char  szEntry [64], *q;  // Last registry "directory"
char  szBuf[1024], *p;

   // retrieves LastWord from szRegPath , this is the INI section
   q = strrchr (szRegPath, '\\');
   lstrcpyn (szEntry, q==NULL ?  szRegPath :  q+1, sizeof szEntry );
   szEntry[sizeof szEntry-1]= 0;

   Rc = GetPrivateProfileString (szEntry, szKey, NULL, szBuf, sizeof szBuf, szIniFile);
   if (Rc > 0)        // Key has been found into INI file
   {
      switch (nType)
      {
           case REG_DWORD :  * (DWORD *) buf = atoi (szBuf);  break;
           case REG_SZ     :  lstrcpyn (buf, szBuf, BufSize); 
                               * ( (char *) buf + BufSize - 1) = 0;
                               break;
      } // switch nType
   } 
   else  // key not found into INI file --> use registry
   {
       // double check that key has not been found (for example you may have szLocalIP= with an empty value)
       // read full section then search for key
       Rc = GetPrivateProfileString (szEntry, NULL, NULL, szBuf, sizeof szBuf, szIniFile);
       for ( p = szBuf ; p - szBuf < Rc  &&  *p!=0  && strcmp (p, szKey)!=0 ; p += strlen (p)+1 );
       // key not found
       if (p - szBuf >= Rc ||  *p==0) 
       {
           if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,   // Key handle at root level.
                             szRegPath,              // Path name of child key.
                              0,                 // Reserved.
                              KEY_READ,                // Requesting read access.
                            & hKey) == ERROR_SUCCESS)                    // Address of key to be returned.
           {
               // type is NULL since we don't need it
               Rc = (RegQueryValueEx (hKey, szKey, NULL, NULL, buf, & BufSize)==ERROR_SUCCESS);
               if (hKey!=INVALID_HANDLE_VALUE)
                        RegCloseKey (hKey);
           }
           else                        // Must clear Rc if RegOpenKeyEx fails.
           {						   // bug fixed by Anders Hartzelius
               // * (char *) buf = 0;
               Rc = 0;
           }

       }
       else   // key found in ini file
       {
            // * (char *) buf = 0 ; 
            Rc = 0;
       }
   }
   
return Rc;
} // ReadKey


// -----------------------------------------------------------------------------
// Savekey is now called from an asynchronous thread (from Nick Wagner)
// moved to registry_thread.c
// -----------------------------------------------------------------------------


// SaveKey save a key into the INI file (if any) or into the registry
int SaveKey (const char *szRegPath, const char *szKey, 
             void *buf, int BufSize, int nType,
             const char *szIniFile)
{
DWORD dwState;
int   Rc = FALSE;
HKEY  hKey;
HANDLE hINIFile;

   // does INI File exists ?
   // creation fichier ou lecture
   hINIFile = CreateFile (szIniFile, GENERIC_READ, FILE_SHARE_READ, NULL,
                          OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,NULL);   

   if (hINIFile != INVALID_HANDLE_VALUE) // file exists --> use it
   {char  szEntry [128], szBuf[1024], *q;

      CloseHandle (hINIFile);  

      q = strrchr (szRegPath, '\\');
      lstrcpy (szEntry, q==NULL ?  szRegPath :  q+1 );

      switch (nType)
     {
        case  REG_DWORD :  wsprintf (szBuf, "%d", * (DWORD *) buf);
                           Rc = WritePrivateProfileString (szEntry, szKey, szBuf, szIniFile);
                           break;
        case REG_SZ     :  Rc = WritePrivateProfileString (szEntry, szKey, buf, szIniFile);
                           break;
    } // switch mode
   }
   else     // do not use INI File, use Registry
   {
//MessageBox (NULL, szKey, "Tftpd32", MB_OK);
       if (RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                            szRegPath,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_WRITE,
                            NULL,
                            & hKey,
                            & dwState)  ==  ERROR_SUCCESS)
       {
          Rc = (RegSetValueEx (hKey,szKey, 0, nType, buf,  
                  nType==REG_SZ ? 1+lstrlen ((LPSTR) buf) : BufSize) == ERROR_SUCCESS);
          CloseHandle (hKey);
        } 
 } // Does INI file exists ?
return Rc;
} // SaveKey

