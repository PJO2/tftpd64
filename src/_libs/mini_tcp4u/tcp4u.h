// -----------------------------------------------------------------------------------------------------
// TCP4U
//   Source released under GPL license
// -----------------------------------------------------------------------------------------------------


#define  TCP4U_SUCCESS           1  /* >=1 function OK            */
#define  TCP4U_ERROR            -1  /* error                      */
#define  TCP4U_TIMEOUT          -2  /* timeout has occured        */
#define  TCP4U_BUFFERFREED      -3  /* the buffer has been freed  */
#define  TCP4U_HOSTUNKNOWN      -4  /* connect to unknown host    */
#define  TCP4U_NOMORESOCKET     -5  /* all socket has been used   */
#define  TCP4U_NOMORERESOURCE   -5  /* or no more free resource   */
#define  TCP4U_CONNECTFAILED    -6  /* connect function has failed*/
#define  TCP4U_UNMATCHEDLENGTH  -7  /* TcpPPRecv : Error in length*/
#define  TCP4U_BINDERROR        -8  /* bind failed (Task already started?) */
#define  TCP4U_OVERFLOW         -9  /* Overflow during TcpPPRecv  */
#define  TCP4U_EMPTYBUFFER     -10  /* TcpPPRecv receives 0 byte  */
#define  TCP4U_CANCELLED       -11  /* Call cancelled by signal   */
#define  TCP4U_INSMEMORY       -12  /* Not enough memory          */
#define  TCP4U_BADPORT         -13  /* Bad port number or alias   */
#define  TCP4U_SOCKETCLOSED      0  /* Host has closed connection */
#define  TCP4U_FILE_ERROR      -14  /* A file operation has failed*/
#define  TCP4U_VERSION_ERROR   -15  /* Versions differs           */
#define  TCP4U_BAD_AUTHENT     -16 /* Bad authentication          */

/* ------------------------------- */
/* Different modes for TcpRecv     */
/* ------------------------------- */
#define TCP4U_WAITFOREVER    0
#define TCP4U_DONTWAIT      ((unsigned) -1)




SOCKET TcpGetListenSocket (int family, LPCSTR szService, unsigned short *pPort);
int TcpRecv (SOCKET s, LPSTR szBuf, unsigned uBufSize, unsigned uTimeOut, HANDLE hLogFile);
int TcpSend (SOCKET s, LPCSTR szBuf, unsigned uBufSize, HANDLE hLogFile);
SOCKET TcpConnect (LPCSTR  szHost,
                   LPCSTR  szService, 
				   int     family,
                   unsigned short nPort);
int TcpPPSend (SOCKET s, LPCSTR szBuf, unsigned uBufSize, HANDLE hLogFile);
int TcpPPRecv (SOCKET s, LPSTR szBuf, unsigned uBufSize, int uTimeOut, HANDLE hLogFile);

int TcpExchangeChallenge (SOCKET s, int seed, int nVersion, int *peerVersion, const char *key);

int UdpSend (int nFromPort, struct sockaddr *sa_to, int sa_len, const char *data, int len);


