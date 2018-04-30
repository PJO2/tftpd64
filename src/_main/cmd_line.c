//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Mars 2007 Ph.jounin
// File cmd_line.c:  process cmd_line
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////



#include "headers.h"

static int argc=0; 
static LPSTR argv[32]; 

static int SplitCmdLine (LPSTR lpszCmdLine)
{
LPSTR p, q; 
  argv[0] = APPLICATION; 
  p = lpszCmdLine; 
  while (p!=NULL &&  *p) 
  {  // for each argument 
     while ((*p) && (*p == ' ')) p++; // skip over leading spaces 
     if (*p == '"') 
     {  
        q = ++p; // skip double quotes
        // scan to end of argument 
        while ((*p) && (*p != '"'))  p++; 
        argv[++argc] = q; 
        if (*p)  *p++ = 0; 
     } 
     else if (*p)  
     { // delimited by spaces 
         q = p; 
         while ((*p) && (*p != ' '))  p++; 
         argv[++argc] = q; 
         if (*p)  *p++ = 0; 
     } 
  }  // parse all "words"
  // create empty 
  argv[++argc] = NULL; 
return argc;
} // SplitCmdLine


///////////////
// handle the command line

int ProcessCmdLine (LPSTR lpszCmdLine)
{
int Ark;
static char szCmdCopy[512];

  // Move to argv/argc
  strncpy (szCmdCopy, lpszCmdLine, sizeof szCmdCopy - 1);
  SplitCmdLine (szCmdCopy);
  for (Ark=1 ; Ark<argc ; Ark++)
  {
     if (argv[Ark][0] =='-' && argv[Ark+1]!=NULL)
     {
        switch (argv[Ark][1])
        {
           case 's' : SetEnvironmentVariable (TFTP_DIR, argv[++Ark]); break;
           case 'l' : SetEnvironmentVariable (TFTP_LOG, argv[++Ark]); break;
           case 'i' : SetEnvironmentVariable (TFTP_INI, argv[++Ark]); break;
#ifdef SERVICE_EDITION
		   case 'h' : SetEnvironmentVariable (TFTP_HOST, argv[++Ark]); break;
		   case 'p' : SetEnvironmentVariable (TFTP_PWD, argv[++Ark]); break;
#endif
        }
     }
  } // parse all items
  
return Ark;
} //ProcessCmdLine
