//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Nov 2010 Ph.jounin
// File convert.c : Convert text settings to structure
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////


#include <winsock2.h>
#include "convert.h"


// translate an "SnmpSet" String into a Setting structure
int ConvertTextToSettings (const char *szTxt, struct S_Setttings *pSettings);
{

} // ConvertTextToSettings 


int ConvertSettingsTotext (const struct S_Setttings *pSettings, char *szTxt);