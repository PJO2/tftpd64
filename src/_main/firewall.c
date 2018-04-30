// firewall.c : Defines the entry point for the console application.
// Nov 2013 by Ph. Jounin 
// based on the Microsoft C++ sample : http://msdn.microsoft.com/en-us/library/aa364726(VS.85).aspx
//


#ifdef STILL_NOT_READY

// Code to be inserted 
#ifdef STANDALONE_EDITION
{
char szIniFile[_MAX_PATH], sz [_MAX_PATH];
    // Get the path in order to find the help file
    if (GetEnvironmentVariable (TFTP_INI, sz, sizeof sz)!=0)
          SetIniFileName (sz, szIniFile);
    else  SetIniFileName (INI_FILE, szIniFile);
	ReadKey(TFTPD32_MAIN_KEY, KEY_DONOTCHECK_FIREWALL, &no, sizeof(no), REG_DWORD, szIniFile);
	if (! no) DisplayMsgBoxIfFirewall (szIniFile);
}
#endif


#undef __cplusplus

#include <windows.h>
#include <stdio.h>
#include <netfw.h>

#pragma comment( lib, "ole32.lib" )


// check if Firewall blocks Tftpd32

/* ------------------------------------------------- */
/* Firewall CallBack                                     */
/* ------------------------------------------------- */
LRESULT CALLBACK FirewallProc (HWND hWnd, UINT message, WPARAM wParam, LONG lParam)
{
int True=1;
static const char *lpIniFile;
	switch (message)
	{
		case WM_INITDIALOG :
			lpIniFile = (const char *) lParam;
			break;
		case WM_COMMAND :
			if (LOWORD (wParam)==IDOK)  EndDialog (hWnd, 0);
			break;
		case WM_CLOSE : 
			if (SendDlgItemMessage (hWnd, IDC_CHECK_FIREWALL, BM_GETCHECK, 0, 0) == BST_CHECKED)
					SaveKey (TFTPD32_MAIN_KEY, KEY_DONOTCHECK_FIREWALL, &True, sizeof True, REG_DWORD, lpIniFile);
			 EndDialog (hWnd, 0);
			break;
	}
return FALSE;
} // FirewallProc

#include <netfw.h>
HRESULT WindowsFirewallInitialize( INetFwProfile **fwProfile );
void WindowsFirewallCleanup( INetFwProfile *fwProfile );
HRESULT WindowsFirewallAppIsEnabled(IN INetFwProfile* ,IN const char* ,OUT BOOL* );
HRESULT WindowsFirewallIsOn(IN INetFwProfile* fwProfile, OUT BOOL* fwOn);

int DisplayMsgBoxIfFirewall (char *szIniFile)
{
INetFwProfile *pfwProfile = NULL;
char szAppBasePath [512];
HRESULT hr;
BOOL IsException = 0xcc, bOn=0xcc;
	hr = WindowsFirewallInitialize( &pfwProfile );
	if (SUCCEEDED (hr))
	{
		hr = WindowsFirewallIsOn (pfwProfile, & bOn);
		if (bOn)
		{
			GetModuleFileName (NULL, szAppBasePath, sizeof szAppBasePath-1);
			hr = WindowsFirewallAppIsEnabled (pfwProfile, szAppBasePath, & IsException);
			// if (SUCCEEDED (hr)  && ! IsException)
				OpenNewDialogBox (NULL, IDD_DIALOG_FIREWALL, (DLGPROC) FirewallProc, (LPARAM) szIniFile, 0);
		}
		WindowsFirewallCleanup (pfwProfile);
	}
return 0;
} // DisplayMsgBoxIfFirewall


// -----------------------------------
// Initialize firewall API
// -----------------------------------

HRESULT WindowsFirewallInitialize( INetFwProfile **fwProfile )
{
HRESULT hr;
INetFwMgr* fwMgr = NULL;
INetFwPolicy* fwPolicy = NULL;
*fwProfile = NULL;

	hr = CoInitializeEx( 0, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE );
	if( SUCCEEDED ( hr ))
	{

		// Create an instance of the firewall settings manager.
		hr = CoCreateInstance( & CLSID_NetFwMgr, NULL, CLSCTX_INPROC_SERVER,
								& IID_INetFwMgr, (void**)&fwMgr );
		if( SUCCEEDED ( hr ) )
		{
			// Retrieve the local firewall policy.
			hr = fwMgr->lpVtbl->get_LocalPolicy(fwMgr, &fwPolicy);
			if( SUCCEEDED ( hr ) )
			{
				// Retrieve the firewall profile currently in effect.
				hr = fwPolicy->lpVtbl->get_CurrentProfile(fwPolicy, fwProfile);
			} // retrieve local policy
		} // get fwMgr object 
	} // Initialise COM 

	// release all except fwProfile
	if( fwPolicy != NULL )   fwPolicy->lpVtbl->Release(fwPolicy);
	if( fwMgr != NULL )  	 fwMgr->lpVtbl->Release(fwMgr);

return hr;
}  // WindowsFirewallInitialize


// -----------------------------------
// Free resources
// -----------------------------------
void WindowsFirewallCleanup( INetFwProfile *fwProfile )
{
	if( fwProfile != NULL )  fwProfile->lpVtbl->Release(fwProfile);
	CoUninitialize();
}



// -----------------------------------
// The question : Is my app firewall enabled
// -----------------------------------
HRESULT WindowsFirewallAppIsEnabled(
            IN INetFwProfile* fwProfile,
            IN const char* ansiProcessImageFileName,
            OUT BOOL* fwAppEnabled
            )
{
HRESULT hr = S_OK;
int Rc=1;
BSTR fwBstrProcessImageFileName = NULL;
VARIANT_BOOL fwEnabled;
wchar_t fwProcessImageFileName [_MAX_PATH+_MAX_FNAME];
INetFwAuthorizedApplication* fwApp = NULL;
INetFwAuthorizedApplications* fwApps = NULL;

    if  (fwProfile == NULL   ||  fwProcessImageFileName == NULL  ||  fwAppEnabled == NULL )
		return -1;

    *fwAppEnabled = FALSE;

	// Retrieve the authorized application collection.
    hr = fwProfile->lpVtbl->get_AuthorizedApplications(fwProfile, &fwApps);
    if (SUCCEEDED (hr))
    {
		Rc = MultiByteToWideChar (CP_ACP, 
			                 0, 
						     ansiProcessImageFileName, 
							 -1,
						     fwProcessImageFileName, 
							 _MAX_PATH+_MAX_FNAME);
		if (Rc>0)
		{
			fwBstrProcessImageFileName = SysAllocString (fwProcessImageFileName);
			if (fwBstrProcessImageFileName != NULL)
			{
				// Attempt to retrieve the authorized application.
				hr = fwApps->lpVtbl->Item(fwApps, fwBstrProcessImageFileName, &fwApp);
				if (SUCCEEDED(hr))
				{
					// Find out if the authorized application is enabled.
					hr = fwApp->lpVtbl->get_Enabled(fwApp, &fwEnabled);
					if (SUCCEEDED(hr))
					{
						*fwAppEnabled =  (fwEnabled != VARIANT_FALSE);
					} // status of application 
				} // application founded in firewall list
				else {hr = S_OK; *fwAppEnabled = FALSE; }
			} // can allocate BSTR object
		} // conversion ANSI->Unicode
    } // can get list of enabled applications

    // Free resources.
    if (fwBstrProcessImageFileName!= NULL) SysFreeString(fwBstrProcessImageFileName);
    // Release the authorized application instance.
    if (fwApp != NULL)					   fwApp->lpVtbl->Release(fwApp);
    // Release the authorized application collection.
    if (fwApps != NULL)					   fwApps->lpVtbl->Release(fwApps);
return hr;
} // WindowsFirewallAppIsEnabled


HRESULT WindowsFirewallIsOn(IN INetFwProfile* fwProfile, OUT BOOL* fwOn)
{
HRESULT hr = S_OK;
VARIANT_BOOL fwEnabled;

    if (fwProfile == NULL  ||  fwOn==NULL)  return -1;

    *fwOn = FALSE;
    // Get the current state of the firewall.
    hr = fwProfile->lpVtbl->get_FirewallEnabled(fwProfile, &fwEnabled);
	if (SUCCEEDED (hr)  &&  (fwEnabled != VARIANT_FALSE))
		*fwOn = TRUE;

return hr;
} // WindowsFirewallIsOn

#endif


#ifdef TEST_CONSOLE
// -----------------------------------
// The test module
// -----------------------------------

int main(int argc, char *argv[])
{
INetFwProfile *pfwProfile = NULL;
char szAppBasePath [512];
HRESULT hr;
BOOL IsEnabled = 0xcc,  bOn=0xcc;

	hr = WindowsFirewallInitialize( &pfwProfile );
	hr = WindowsFirewallIsOn (pfwProfile, & bOn);
	printf ("le firewall est %s\n", bOn ? "actif" : "inactif");

	if (argc==2)		lstrcpy (szAppBasePath, argv[1]);
	else                GetModuleFileName (NULL, szAppBasePath, sizeof szAppBasePath-1);

	hr = WindowsFirewallAppIsEnabled (pfwProfile, szAppBasePath, & IsEnabled);
	WindowsFirewallCleanup (pfwProfile);

	if (SUCCEEDED (hr))
		printf ("Le firewall %s l'application %s\n", 
			(IsEnabled ? "accepte" : "refuse") , szAppBasePath);
	else printf ("Erreur dans la requete");
	return 0;
} // main

#endif


