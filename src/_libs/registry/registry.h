//////////////////////////////////////////////////////
//
// Projet TFTPD32.  May 1998      Ph.jounin
// Projet DHCPD32.  January 2006  Ph.jounin
// File registry.h: Settings
//
// source released under artistic License (see license.txt)
// 
//////////////////////////////////////////////////////

// registry key :
//      HKEY_LOCAL_MACHINE\SOFTWARE\TFTPD32

int ReadKey (const char *szRegPath, const char *szKey, 
            void *buf, int BufSize, int nType,
             const char *szIniFile);
int AsyncSaveKey (const char *szRegPath, const char *szKey, 
                  void *buf, int BufSize, int nType,
                  const char *szIniFile);
int SaveKey (const char *szRegPath, const char *szKey, 
             void *buf, int BufSize, int nType,
             const char *szIniFile);
int InitAsyncSaveKey ();
int CleanupAsyncSaveKey ();

