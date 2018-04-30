//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Feb 99  Ph.jounin
// File Headers.h :   Gestion du protocole
//
// released under artistic license (see license.txt)
// 
//////////////////////////////////////////////////////

// #define TFTP_CLIENT_ONLY 1


#define WIN32_LEAN_AND_MEAN // this will assume smaller exe
#define _CRT_SECURE_NO_DEPRECATE

#pragma pack()

#include <windows.h>
#include <windowsx.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <IPHlpApi.h>
#include <commctrl.h>
#include <shellapi.h>

#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <process.h>

#include "_libs/GUI/cmsgbox.h"
#include "_libs/GUI/centerchild.h"
#include "_libs/GUI/listview.h"
#include "_libs/GUI/window.h"
#include "_libs/registry/registry.h"
#include "_libs/linked_list/linked_list.h"
#include "_libs/lasterr/lasterr.h"
#include "_libs/md5/global.h"
#include "_libs/md5/md5.h"
#include "_libs/mini_tcp4u/tcp4u.h"
#include "_libs/scandir/scandir.h"
#include "_libs/log/logtomonitor.h"


#include "_common\settings.h"
#include "_common\bootpd_util.h"
#pragma pack(1)
#include "_common\tftp.h"
#pragma pack()
#include "_common\tftp_struct.h"



#include "_common\custom.h"
#include "_gui\tftpd32.h"
#include "_gui\gui_struct.h"

#pragma pack(1)
#include "_common\dialog_socket.h"
#pragma pack()
#include "_common\dialog_common.h"

#include "_gui\gui_functions.h"


typedef unsigned char  u_int8_t;
typedef unsigned short u_int16_t;
typedef unsigned long  u_int32_t;

#pragma pack(1)
#include "_services\dhcp.h"
#pragma pack()

#ifndef MSVC
#  define sscanf_s sscanf
#endif
