//////////////////////////////////////////////////////
//
// Projet DHCPD32.  November 2006 Ph.jounin
//                  A free DHCP server for Windows
// File registry_thread.c: asynchrously write configuration 
//                                        to either ini file or registry
//                                        from Nick Wagner
//
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////

#include "headers.h"

#include "threading.h"

struct S_ini
{
	char* path;
	char* key;
	void* buf;
	int bufsize;
	int type;
	char* inifile;
}; //      
    
    
// ---------------------------------------------------------------
// linked list management
// ---------------------------------------------------------------
    
// Populate the S_ini structure
static struct S_ini *PopulateMsg (struct S_ini *pmsg, const char* szRegPath, const char* szKey, void* buf, int BufSize, int nType, const char* szIniFile)
{
	pmsg->path = malloc(strlen(szRegPath) + 1);
	if(!pmsg->path)
		return 0;
	lstrcpy(pmsg->path, szRegPath);
	pmsg->key = malloc(strlen(szKey) + 1);
	if(!pmsg->key)
		return 0;
	lstrcpy(pmsg->key, szKey);
	if(nType == REG_SZ)
	{
		pmsg->buf = malloc(BufSize + 1);
		((char*)(pmsg->buf))[BufSize] = '\0';
	}
	else
		pmsg->buf = malloc(BufSize);
	if(!pmsg->buf)
		return 0;
	memcpy(pmsg->buf, buf, BufSize);
	pmsg->bufsize = BufSize;
	pmsg->type = nType;
	pmsg->inifile = malloc(strlen(szIniFile) + 1);
	if(!pmsg->inifile)
		return 0;
	lstrcpy(pmsg->inifile, szIniFile);
return pmsg;
}  // PopulateMsg



//Does the delete
static void DeleteMsg (struct S_ini *pdel)
{
	free(pdel->path);
	free(pdel->key);
	free(pdel->buf);
	free(pdel->inifile);
    free (pdel);
} // DeleteMsg

// ---------------------------------------------------------------
//a therad which asynchronously save th registry entries
// ---------------------------------------------------------------
void AsyncSaveKeyBckgProc (void *param)
{
struct S_ini *pmsg;
    LL_Create (LL_ID_SETTINGS, 500);
	tThreads[TH_ASYNCSAVEKEY].bInit = TRUE;		// inits OK

    do
    {
        WaitForSingleObject (tThreads[TH_ASYNCSAVEKEY].hEv, INFINITE);
        Sleep (10);
        for ( pmsg = LL_PopMsg (LL_ID_SETTINGS); 
              pmsg != NULL ; 
              pmsg = LL_PopMsg (LL_ID_SETTINGS) )
        { 
             SaveKey (pmsg->path, pmsg->key, pmsg->buf, pmsg->bufsize, pmsg->type, pmsg->inifile);
             DeleteMsg(pmsg);
        }
        ResetEvent ( tThreads[TH_ASYNCSAVEKEY].hEv );
    }
    while ( tThreads[TH_ASYNCSAVEKEY].gRunning );
    LL_Destroy (LL_ID_SETTINGS);

	LogToMonitor ("end of registry thread\n");
_endthread ();        
} // AsyncSaveKeyProc


// The "real" function called by Tftpd32 modules
int AsyncSaveKey(const char* szRegPath, const char* szKey, void* buf, int BufSize, int nType, const char* szIniFile)
{
struct S_ini msg;	
    PopulateMsg ( & msg, szRegPath, szKey, buf, BufSize, nType, szIniFile );
	LL_PushMsg (LL_ID_SETTINGS, & msg, sizeof msg);
    // tell reg thread something is to be saved
    WakeUpThread (TH_ASYNCSAVEKEY);
	return 1;
}

