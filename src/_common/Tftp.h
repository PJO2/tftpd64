/*
 * From TFTP.H 
 * Changes made by Ph. Jounin : Constant have been prefixed by TFTP_
 */
 
/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)tftp.h	8.1 (Berkeley) 6/2/93
 */



#ifndef _ARPA_TFTP_H
#define _ARPA_TFTP_H

/*
 * Trivial File Transfer Protocol (IEN-133)
 */
#define TFTP_SEGSIZE          512     /* data segment size */
#define TFTP_MAXSEGSIZE     16384       /* data segment size */
#define TFTP_MINSEGSIZE         8       /* data segment size */
/*
 * Packet types.
 */
#define TFTP_RRQ    01          /* read request */
#define TFTP_WRQ    02          /* write request */
#define TFTP_DATA   03          /* data packet */
#define TFTP_ACK    04          /* acknowledgement */
#define TFTP_ERROR  05          /* error code */
#define TFTP_OACK   06          /* option acknowledgement */


struct    tftphdr {
  short   th_opcode;      /* packet type */
  union {
          short   tu_block;   /* block # */
          short   tu_code;    /* error code */
          char    tu_stuff[1];    /* request packet stuff */
  } th_u;
  char    th_data[2];     /* data or error string */
};


#define  th_block    th_u.tu_block
#define  th_code     th_u.tu_code
#define  th_stuff    th_u.tu_stuff
#define  th_msg      th_data

#define TFTP_DATA_HEADERSIZE ( offsetof (struct tftphdr, th_data ) )
#define TFTP_ACK_HEADERSIZE  (  offsetof (struct tftphdr, th_block )  \
                            + sizeof ( ((struct tftphdr *) (0))->th_block) )

/*
 * Error codes.
 */
#define  EUNDEF      0       /* not defined */
#define  ENOTFOUND   1       /* file not found */
#define  EACCESS     2       /* access violation */
#define  ENOSPACE    3       /* disk full or allocation exceeded */
#define  EBADOP      4       /* illegal TFTP operation */
#define  EBADID      5       /* unknown transfer ID */
#define  EEXISTS     6       /* file already exists */
#define  ENOUSER     7       /* no such user */
#define  EBADOPTION  8        /* bad option */
#define  ECANCELLED 99      /* cancelled by administrator */


/* 
 * options 
 */
#define TFTP_OPT_TSIZE     "tsize"
#define TFTP_OPT_TIMEOUT   "timeout"
#define TFTP_OPT_BLKSIZE   "blksize"
#define TFTP_OPT_MCAST     "multicast"
#define TFTP_OPT_PORT      "udpport"

#define IS_OPT(s,opt)   (lstrcmpi (s, opt)==0)

#define PKTSIZE             TFTP_SEGSIZE+4
#define MAXPKTSIZE          TFTP_MAXSEGSIZE+4


#endif /* _ARPA_TFTP_H */
