
/* --------------------
* dump.c is an extract form Tcp4u and therefore released under GPL license
* --------------------- */


/* -------------------------------------------------------------- */
/* dump a binary or text frame. The output is the debug window    */
/* for Windows system and stderr for unix                         */
/* The code is a port of the xdump function from the cmu snmp lib */
/* Ajout du cast (unsigned char)                                  */
/* -------------------------------------------------------------- */
void BinDump (LPCSTR cp, int nLen, LPCSTR szPrefix);
void OutputDebugStringW95 (LPCTSTR lpOutputString);
