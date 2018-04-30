//////////////////////////////////////////////////////
//
// Projet TFTPD32.  April 2007 Ph.jounin
//                  A free TFTP server for Windows
// File console_action.c: reply to complex GUI request
//      All procedures are handled by console thread
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////

#include "headers.h"
#include "threading.h"
#include <stdlib.h>

static struct S_DirectoryContent Dir;

int SendDirectoryContentCbk (char *sz_line_descr, DWORD dummy)
{
   if (Dir.nb < SizeOfTab (Dir.ent))
   {
        lstrcpyn (Dir.ent[Dir.nb++].file_descr, sz_line_descr, sizeof Dir.ent[Dir.nb++].file_descr - 1);
   }
return TRUE;
} // SendDirectoryContentCbk

// Copy directory content into a S_DirectoryContent Structure
void SendDirectoryContent (void)
{
    memset (&Dir, 0, sizeof Dir);
    ScanDir (SendDirectoryContentCbk, 0, sSettings.szWorkingDirectory);
    SendMsgRequest  ( C_REPLY_DIRECTORY_CONTENT, 
                    & Dir, 
                      Dir.nb * sizeof Dir.ent[0] + offsetof (struct S_DirectoryContent, ent),
                      FALSE,
                      FALSE );
} // SendDirectoryContent
