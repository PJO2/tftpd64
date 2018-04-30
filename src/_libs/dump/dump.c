/* --------------------
* dump.c is an extract form Tcp4u and therefore released under GPL license
* Note that e-mail address is invalid
* --------------------- */

/*
 * From Tcp4u v 3.31         Last Revision 08/12/1997  3.31-01
 *
 *===========================================================================
 *
 * Project: Tcp4u,      Library for tcp protocol
 * File:    tcp4_log.c
 * Purpose: Some logging / debugging
 *
 *===========================================================================
 *
 * This software is Copyright (c) 1996-1999 by Philippe Jounin
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 *
 *
 *  If you make modifications to this software that you feel
 *  increases it usefulness for the rest of the community, please
 *  email the changes, enhancements, bug fixes as well as any and
 *  all ideas to me. This software is going to be maintained and
 *  enhanced as deemed necessary by the community.
 *
 *
 *             Philippe Jounin (ph.jounin@computer.org)
 */


#include <windows.h>
#include <ctype.h>

#include <process.h>

#include "dump.h"

void OutputDebugStringW95 (LPCTSTR lpOutputString)
{

    HANDLE heventDBWIN;  /* DBWIN32 synchronization object */
    HANDLE heventData;   /* data passing synch object */
    HANDLE hSharedFile;  /* memory mapped file shared data */
    LPSTR lpszSharedMem;
    /*
        Do a regular OutputDebugString so that the output is
        still seen in the debugger window if it exists.

        This ifdef is necessary to avoid infinite recursion
        from the inclusion of W95TRACE.H
    */
#ifdef _UNICODE
    OutputDebugStringW(lpOutputString);
#else
    OutputDebugStringA(lpOutputString);
#endif

    /* bail if it's not Win95 */
    {
        OSVERSIONINFO VerInfo;
        VerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        GetVersionEx(&VerInfo);
        if ( VerInfo.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS )
            return;
    }

    /* make sure DBWIN is open and waiting */
    heventDBWIN = OpenEvent(EVENT_MODIFY_STATE, FALSE, "DBWIN_BUFFER_READY");
    if ( !heventDBWIN )
    {
        //MessageBox(NULL, "DBWIN_BUFFER_READY nonexistent", NULL, MB_OK);
        return;
    }

    /* get a handle to the data synch object */
    heventData = OpenEvent(EVENT_MODIFY_STATE, FALSE, "DBWIN_DATA_READY");
    if ( !heventData )
    {
        // MessageBox(NULL, "DBWIN_DATA_READY nonexistent", NULL, MB_OK);
        CloseHandle(heventDBWIN);
        return;
    }

    hSharedFile = CreateFileMapping((HANDLE)-1, NULL, PAGE_READWRITE, 0, 4096, "DBWIN_BUFFER");
    if (!hSharedFile)
    {
        //MessageBox(NULL, "DebugTrace: Unable to create file mapping object DBWIN_BUFFER", "Error", MB_OK);
        CloseHandle(heventDBWIN);
        CloseHandle(heventData);
        return;
    }

    lpszSharedMem = (LPSTR)MapViewOfFile(hSharedFile, FILE_MAP_WRITE, 0, 0, 512);
    if (!lpszSharedMem)
    {
        //MessageBox(NULL, "DebugTrace: Unable to map shared memory", "Error", MB_OK);
        CloseHandle(heventDBWIN);
        CloseHandle(heventData);
        return;
    }

    /* wait for buffer event */
    WaitForSingleObject(heventDBWIN, INFINITE);

    /* write it to the shared memory */
    *((LPDWORD)lpszSharedMem) = _getpid();
    lstrcpy (lpszSharedMem + sizeof(DWORD), lpOutputString);

    /* signal data ready event */
    SetEvent(heventData);

    /* clean up handles */
    CloseHandle(hSharedFile);
    CloseHandle(heventData);
    CloseHandle(heventDBWIN);

    return;
}



/* -------------------------------------------------------------- */
/* dump a binary or text frame. The output is the debug window    */
/* for Windows system and stderr for unix                         */
/* The code is a port of the xdump function from the cmu snmp lib */
/* Ajout du cast (unsigned char)                                  */
/* -------------------------------------------------------------- */
void BinDump (LPCSTR cp, int nLen, LPCSTR szPrefix)
{
int   col, count, nPos;
char  szLine [128];
static const char tCvtHex[] = "0123456789ABCDEF";

    if (nLen==0)    /* Empty message -> has to be dumped */
    {
        if (szPrefix!=NULL)   lstrcpyn  (szLine, szPrefix, 20);
        else                  szLine[0]=0;
        lstrcat (szLine, " Empty Message\n");
        OutputDebugString (szLine);
        return;
    }

    count = 0;
    while (count < nLen)
    {
        if (szPrefix!=NULL)   lstrcpyn  (szLine, szPrefix, 20);
        else                  szLine[0]=0;
        nPos = lstrlen (szLine);
        szLine[nPos++] = ' ';

        for (col = 0 ;   count + col < nLen   &&   col < 16 ;   col++)
        {
            if (col == 8)   szLine[nPos++] = '-', szLine[nPos++] = ' ' ;
            szLine [nPos++] = tCvtHex [(unsigned char ) cp[count + col] >> 4];
            szLine [nPos++] = tCvtHex [(unsigned char ) cp[count + col] & 0x0F];
            szLine [nPos++] = ' ';
        }

        while(col++ < 16)      /* pad end of buffer with zeros */
        {
            if (col == 8)  szLine[nPos++] = ' ', szLine[nPos++] = ' ';
            szLine[nPos++] = ' ';
            szLine[nPos++] = ' ';
            szLine[nPos++] = ' ';
        }
        szLine[nPos++] = ' ';
        szLine[nPos++] = ' ';

        for (col = 0;  count + col < nLen && col < 16 ;   col++)
        {
          szLine[nPos++] = isprint(cp[count + col]) ? cp[count + col] : '.';
        }
        lstrcpy (& szLine[nPos], "\n");
        OutputDebugStringW95 (szLine);

        count += col;
    } /* while buffer nor printed */

} /* BinDump */


