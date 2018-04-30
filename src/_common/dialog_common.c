//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Mars 2000 Ph.jounin
// File dialog_common.c: 
//			A sendmsg API for both GUI and services
//          It's a kind of TcpPPSend with a complimentory
//          integer parameter
//
// released under European Union Public License
//
//////////////////////////////////////////////////////



#include "headers.h"
#include <process.h>
#include <stdio.h>


int SendMsg (SOCKET s, int type, const void *data, int size)
{
int Rc=1;
unsigned short full_size = htons (size + sizeof type);
    if (data==NULL)  size=0;
    // send length of message
    Rc = send (s, (char *) & full_size, sizeof full_size, 0);
    Rc = send (s, (char *) & type, sizeof type, 0);
    if (size>0) Rc = TcpSend (s, data, size, 0);
return Rc;
} // SendMsg


