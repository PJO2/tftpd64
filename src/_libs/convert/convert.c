//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Nov 2010 Ph.jounin
// File convert.h : Convert text settings to structure
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////

enum {
		E_STG_ADDRIP   =  'a',
		E_STG_CHAR     =  'c',
		E_STG_INTEGER  =  'i',
		E_STG_STRING   =  's',
     };

#define MAX_STG_DATA	20

struct S_Settings
{
	char		 cType;
	unsigned int nLen;
	char		 cData [MAX_STG_DATA];
}; 

int ConvertTextToSettings (const char *szTxt, struct S_Setttings *pSettings);
int ConvertSettingsTotext (const struct S_Setttings *pSettings, char *szTxt);


