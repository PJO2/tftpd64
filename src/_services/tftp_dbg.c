//////////////////////////////////////////////////////
//
// Projet TFTPD32.   Mai 98 Ph.jounin - June 2006
// File tftp_dbg.c:  TFTP datagram debugging
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////

#include "headers.h"


struct LL_TftpInfo *DoDebugSendBlock (struct LL_TftpInfo *pTftp)
{
#ifdef DEB_TEST
          if ((unsigned short) pTftp->c.nLastToSend<50  ||  (unsigned short) pTftp->c.nLastToSend>65500)
          {
              BinDump (pTftp->b.buf, pTftp->c.dwBytes + TFTP_DATA_HEADERSIZE, "Data:");
              LOG (10, "SendFile block #%d, %d bytes, %d total",
                    (unsigned short) pTftp->c.nLastToSend, pTftp->c.dwBytes, pTftp->st.dwTotalBytes);
          }
#endif
#ifdef DEBUG
          LOG (10, "SendFile block #%d, %d bytes, %d total",
                      (unsigned short) pTftp->c.nLastToSend, pTftp->c.dwBytes, pTftp->st.dwTotalBytes);
          BinDump (pTftp->b.buf, pTftp->c.dwBytes + TFTP_DATA_HEADERSIZE, "Data:");
#endif
return pTftp; // no warning
} // DoDebugSendBlock

struct LL_TftpInfo *DoDebugRcvAck (struct LL_TftpInfo *pTftp)
{
#ifdef DEB_TEST
struct tftphdr *tp = (struct tftphdr *) pTftp->b.ackbuf;;
         if ((unsigned short) pTftp->c.nCount<50  ||  (unsigned short) pTftp->c.nCount>65500)
         {
              LOG (10, "Read ACK block #%d, wanted #%d, Retry %d",
                            ntohs (tp->th_block), (unsigned short) pTftp->c.nCount, pTftp->c.nRetries);
              BinDump (pTftp->b.ackbuf, TFTP_DATA_HEADERSIZE, "ACK:");
         }
#endif
#ifdef DEBUG
         LOG (10, "Read ACK block #%d, wanted #%d, Retry %d",
                        ntohs (tp->th_block), (unsigned short) pTftp->c.nCount, pTftp->c.nRetries);
         BinDump (pTftp->b.ackbuf, TFTP_DATA_HEADERSIZE, "ACK:");
#endif
return pTftp; // no warning
} // DoDebugRecvAck


struct LL_TftpInfo *DoDebugSendAck (struct LL_TftpInfo *pTftp)
{
#ifdef DEB_TEST
    if ((unsigned short) pTftp->c.nCount<50  ||  (unsigned short) pTftp->c.nCount>65500)
    {
        LOG (10, "Send ACK block #%d", (unsigned short) pTftp->c.nCount);
        BinDump (pTftp->b.ackbuf, TFTP_DATA_HEADERSIZE, "ACK:");
    }
#endif
#ifdef DEBUG
    LOG (10, "Send ACK block #%d", (unsigned short) pTftp->c.nCount);
    BinDump (pTftp->b.ackbuf, TFTP_DATA_HEADERSIZE, "ACK:");
#endif
return pTftp;
} // DoDebugSendAck


struct LL_TftpInfo *DoDebugRcvData (struct LL_TftpInfo *pTftp)
{
#ifdef DEBUG
    BinDump (pTftp->b.buf, Rc, "Data:");
    LOG (10, "Read data block #%d, wanted #%d, Retry %d",
              ntohs (tp->th_block),
              (unsigned short) (pTftp->c.nCount+1), pTftp->c.nRetries  );
#endif
#ifdef DEB_TEST
    if ((unsigned short) pTftp->c.nCount<50  ||  (unsigned short) pTftp->c.nCount>65500)
        LOG (10, "Read data block #%d, wanted #%d, Retry %d, Bytes %d",
                  ntohs (tp->th_block),
                  (unsigned short) (pTftp->c.nCount+1), pTftp->c.nRetries, pTftp->st.dwTotalBytes  );
#endif
return pTftp;
} // DoDebugRcvData

