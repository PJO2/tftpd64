//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Mars 2000 Ph.jounin
// File setinifilename.c: 
//			locate ini file in several directories
// 		    try TFTPD32_INI_DIR environment variable
//          then several standard locations
//
// released under European Union Public License
// 
//////////////////////////////////////////////////////


#include "headers.h"

struct EnvInis
{
	const char* env_var;
	const char* fmt;
};
struct EnvInis EnvInisList[] = {
	{ "TFTP_INI",         "%s"           },
	{ "TFTPD32_INI_DIR",  "%s\\%0.0s%s"  }, // ignore APPLICATION_NAME
	{ "TFTPD32_INI_DIR",  "%s\\%s\\%s"   }, // but can still try with
	{ "ALLUSERSPROFILE",  "%s\\%s\\%s"   },
	{ "APPDATA",          "%s\\%s\\%s"   },
	{ "LOCALAPPDATA",     "%s\\%s\\%s"   },
	{ "PROGRAMDATA",      "%s\\%s\\%s"   },
	{ "ProgramW6432",     "%s\\%s\\%s"   },
	{ NULL, NULL }
};

static BOOL FileExists(const char* p) {
	DWORD a = GetFileAttributes(p);
	return (a != INVALID_FILE_ATTRIBUTES) && !(a & FILE_ATTRIBUTE_DIRECTORY);
}

/* ----------------------------------- */
/* Determine the full name of ini file */
/* ----------------------------------- */
const char *SetIniFileName (const char *szIniFile, char *szFullIniFile, size_t len)
{
char szPath [_MAX_PATH];
int  Rc;
const char* p;

	szFullIniFile[0] = 0;
	szFullIniFile[--len] = 0;
	// try searching Tftpd32.ini in data directories
	for (struct EnvInis *dir = EnvInisList; dir->env_var != NULL; dir++)
	{
		p = getenv(dir->env_var);
		if (p != NULL)
		{
			sprintf_s(szFullIniFile, len, dir->fmt, p, APPLICATION_NAME, szIniFile);
			if (FileExists(szFullIniFile)) return szFullIniFile;
		}
	}
	// Try current directory
	if (GetCurrentDirectory(sizeof szPath, szPath)	!= 0)
	{
		sprintf_s(szFullIniFile, len, "%s\\%s", szPath, szIniFile);
		if (FileExists(szFullIniFile)) return szFullIniFile;
	}
	// Try Program location, caling GetModuleFileName
	if (GetModuleFileName(NULL, szPath, sizeof szPath) != 0)
	{
		sprintf_s(szFullIniFile, len, "%s\\%s", szPath, szIniFile);
		if (FileExists(szFullIniFile)) return szFullIniFile;
	}

	// try our registry entry, last NULL skip reading into Ini File !!
	if (ReadKey("Software\\" APPLICATION_NAME, "IniPath", szFullIniFile, len, REG_SZ, NULL))
		return szFullIniFile;

	strcpy_s(szFullIniFile, len, szIniFile); // not found, try local directory
	return szFullIniFile;
} // SetIniFileName

