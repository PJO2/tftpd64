
"""
Minimal TFTP client utilities for testing Tftpd64 (RFC 1350).
No third-party deps. Python 3.8+.
"""
import os
import socket
import struct
import time
from typing import Tuple, Optional

# TFTP opcodes
OP_RRQ   = 1
OP_WRQ   = 2
OP_DATA  = 3
OP_ACK   = 4
OP_ERROR = 5
OP_OACK  = 6  # not used here, but defined

DEFAULT_MODE = b'octet'

class TftpError(Exception):
    def __init__(self, code:int, msg:str):
        super().__init__(f"TFTP Error {code}: {msg}")
        self.code = code
        self.msg = msg

def _build_request(op:int, filename:bytes, mode:bytes=DEFAULT_MODE) -> bytes:
    if b'\x00' in filename:
        raise ValueError("filename must not contain NUL")
    return struct.pack("!H", op) + filename + b'\x00' + mode + b'\x00'

def _parse_error(pkt:bytes) -> Tuple[int,str]:
    _, errcode = struct.unpack("!HH", pkt[:4])
    msg = pkt[4:].split(b'\x00',1)[0].decode(errors='replace')
    return errcode, msg

def tftp_rrq(host:str, port:int, filename:str, timeout:float=5.0, blocksize:int=512) -> bytes:
    """Read a file from TFTP server. Returns the content as bytes."""
    addr = (host, port)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(timeout)
    try:
        # send RRQ to well-known port
        sock.sendto(_build_request(OP_RRQ, filename.encode('utf-8')), addr)
        data = bytearray()
        expected_block = 1
        server_addr = None
        while True:
            pkt, srv = sock.recvfrom(4 + 2 + blocksize)
            if server_addr is None:
                server_addr = srv  # server chooses a new ephemeral port (TID)
            if pkt[:2] == struct.pack("!H", OP_ERROR):
                code, msg = _parse_error(pkt)
                raise TftpError(code, msg)
            op = struct.unpack("!H", pkt[:2])[0]
            if op != OP_DATA:
                # ignore unexpected
                continue
            _, block = struct.unpack("!HH", pkt[:4])
            if block == expected_block:
                payload = pkt[4:]
                data.extend(payload)
                # ACK
                sock.sendto(struct.pack("!HH", OP_ACK, block), server_addr)
                expected_block = (expected_block + 1) & 0xFFFF
                if len(payload) < blocksize:
                    break
            else:
                # Duplicate/out-of-order: ACK last good
                last = (expected_block - 1) & 0xFFFF
                sock.sendto(struct.pack("!HH", OP_ACK, last), server_addr)
        return bytes(data)
    finally:
        sock.close()

def tftp_wrq(host:str, port:int, filename:str, content:bytes, timeout:float=5.0, blocksize:int=512) -> None:
    """Write a file to a TFTP server. Raises TftpError if refused or failed."""
    addr = (host, port)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(timeout)
    try:
        sock.sendto(_build_request(OP_WRQ, filename.encode('utf-8')), addr)
        # Expect ACK(0) from server with new TID
        pkt, server_addr = sock.recvfrom(4 + 2)
        if pkt[:2] == struct.pack("!H", OP_ERROR):
            code, msg = _parse_error(pkt)
            raise TftpError(code, msg)
        if pkt[:2] != struct.pack("!H", OP_ACK) or pkt[2:4] != b'\x00\x00':
            raise RuntimeError("Unexpected response to WRQ")
        # send DATA blocks
        block = 1
        offset = 0
        while True:
            chunk = content[offset:offset+blocksize]
            sock.sendto(struct.pack("!HH", OP_DATA, block) + chunk, server_addr)
            # wait ACK(block)
            pkt, _ = sock.recvfrom(4 + 2)
            if pkt[:2] == struct.pack("!H", OP_ERROR):
                code, msg = _parse_error(pkt)
                raise TftpError(code, msg)
            op, ack_block = struct.unpack("!HH", pkt[:4])
            if op != OP_ACK or ack_block != block:
                # retry simple once on mismatch
                pkt, _ = sock.recvfrom(4 + 2)
                op, ack_block = struct.unpack("!HH", pkt[:4])
                if op != OP_ACK or ack_block != block:
                    raise RuntimeError("Did not receive matching ACK")
            offset += len(chunk)
            block = (block + 1) & 0xFFFF
            if len(chunk) < blocksize:
                break
    finally:
        sock.close()

def send_forged_packet(host:str, port:int, payload:bytes, timeout:float=1.0) -> Optional[bytes]:
    """Send a raw UDP packet to TFTP well-known port and return any immediate reply (if any)."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(timeout)
    try:
        sock.sendto(payload, (host, port))
        try:
            resp, _ = sock.recvfrom(1500)
            return resp
        except socket.timeout:
            return None
    finally:
        sock.close()
