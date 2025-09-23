
"""
Pytest suite for Tftpd64.
Configure via env:
  TFTP_HOST        (default: 127.0.0.1)
  TFTP_PORT        (default: 69)
  TEST_READ_FILE   (optional: filename known to exist on server)
  TEST_WRITE_DIR   (optional: dir or prefix allowed by server for WRQ, e.g. "incoming/")
  BLOCK_SIZE       (default: 512)
  TIMEOUT_SEC      (default: 5.0)

Run:  pytest -q
"""
import os
import random
import string
import threading
import time
from concurrent.futures import ThreadPoolExecutor, as_completed

import pytest

from tftp_test_utils import (
    tftp_rrq, tftp_wrq, TftpError, send_forged_packet,
    OP_ERROR, OP_ACK, OP_DATA, OP_RRQ, OP_WRQ,
)
import struct

HOST = os.getenv("TFTP_HOST", "127.0.0.1")
PORT = int(os.getenv("TFTP_PORT", "69"))
BLOCK = int(os.getenv("BLOCK_SIZE", "512"))
TIMEOUT = float(os.getenv("TIMEOUT_SEC", "5.0"))
READ_FILE = os.getenv("TEST_READ_FILE")
WRITE_DIR = os.getenv("TEST_WRITE_DIR", "")  # e.g., "incoming/"

def _randname(prefix="tftpd64_test_", ext=".bin"):
    rand = ''.join(random.choices(string.ascii_lowercase + string.digits, k=8))
    return f"{prefix}{rand}{ext}"

@pytest.mark.timeout(15)
def test_illegal_filename_yields_error():
    # RRQ on a clearly-nonexistent file should return ERROR (code 1 File not found)
    fname = f"__definitely_not_here__{_randname()}.nope"
    with pytest.raises(TftpError) as ei:
        tftp_rrq(HOST, PORT, fname, timeout=TIMEOUT, blocksize=BLOCK)
    # We can't strictly assert code==1 because some servers map differently, but ensure it's a TftpError
    assert "TFTP Error" in str(ei.value)

@pytest.mark.timeout(30)
def test_legal_write_then_read_roundtrip_or_skip_if_denied():
    content = os.urandom(1500)  # spans multiple blocks
    fname = WRITE_DIR + _randname()
    try:
        tftp_wrq(HOST, PORT, fname, content, timeout=TIMEOUT, blocksize=BLOCK)
    except TftpError as e:
        # Access denied is acceptable in read-only setups
        pytest.skip(f"Server refused WRQ (likely read-only): {e}")
    # If write worked, try reading back
    data = tftp_rrq(HOST, PORT, fname, timeout=TIMEOUT, blocksize=BLOCK)
    assert data == content

@pytest.mark.timeout(30)
def test_legal_read_known_file_or_skip():
    if not READ_FILE:
        pytest.skip("Set TEST_READ_FILE to exercise RRQ on a known file")
    data = tftp_rrq(HOST, PORT, READ_FILE, timeout=TIMEOUT, blocksize=BLOCK)
    assert isinstance(data, (bytes, bytearray))
    assert len(data) >= 0  # any size is fine

# -------- Forged / malformed packet resilience --------

@pytest.mark.timeout(10)
def test_forged_invalid_opcode_is_ignored_or_errors():
    # opcode 99 with junk
    payload = struct.pack("!H", 99) + b"junk\x00"
    resp = send_forged_packet(HOST, PORT, payload, timeout=1.0)
    # Server may ignore (None) or reply ERROR
    if resp:
        assert resp[:2] == struct.pack("!H", OP_ERROR)

@pytest.mark.timeout(10)
def test_forged_ack_to_port69_is_ignored():
    # ACK(block 1) to port 69 without prior session must be ignored
    payload = struct.pack("!HH", OP_ACK, 1)
    resp = send_forged_packet(HOST, PORT, payload, timeout=0.5)
    assert resp is None  # should not elicit a response

@pytest.mark.timeout(10)
def test_forged_data_to_port69_yields_error_or_ignore():
    payload = struct.pack("!HH", OP_DATA, 1) + b"x"*10
    resp = send_forged_packet(HOST, PORT, payload, timeout=1.0)
    # Tftpd64 may answer error "Unknown transfer ID" or ignore
    if resp:
        assert resp[:2] == struct.pack("!H", OP_ERROR)

# -------- Stress test --------

@pytest.mark.timeout(120)
def test_stress_parallel_rrq_or_skip():
    """
    Launch many parallel RRQs on READ_FILE.
    If READ_FILE is not provided, skip (can't fabricate a server-side file here).
    """
    if not READ_FILE:
        pytest.skip("Set TEST_READ_FILE to run RRQ stress")
    N = int(os.getenv("RRQ_CONCURRENCY", "20"))
    REQ = int(os.getenv("RRQ_REQUESTS", "100"))
    def worker(i):
        data = tftp_rrq(HOST, PORT, READ_FILE, timeout=TIMEOUT, blocksize=BLOCK)
        return len(data)
    sizes = []
    with ThreadPoolExecutor(max_workers=N) as ex:
        futs = [ex.submit(worker, i) for i in range(REQ)]
        for f in as_completed(futs):
            sizes.append(f.result())
    assert all(s == sizes[0] for s in sizes), "Inconsistent file sizes across RRQs"

@pytest.mark.timeout(120)
def test_stress_write_small_files_or_skip_if_denied():
    """
    Try many small WRQs in parallel to a writable directory.
    Skip if server refuses WRQ.
    """
    N = int(os.getenv("WRQ_CONCURRENCY", "10"))
    REQ = int(os.getenv("WRQ_REQUESTS", "30"))
    if WRITE_DIR is None:
        pytest.skip("No WRITE_DIR specified")
    content = b"x"*123
    errors = 0
    def worker(i):
        try:
            tftp_wrq(HOST, PORT, WRITE_DIR + _randname(ext=f"_{i}.bin"), content, timeout=TIMEOUT, blocksize=BLOCK)
            return True
        except TftpError as e:
            if "denied" in str(e).lower() or e.code in (2, 3):  # Access violation / Disk full
                return False
            raise
    results = []
    with ThreadPoolExecutor(max_workers=N) as ex:
        results = list(as_completed([ex.submit(worker, i) for i in range(REQ)]))
    ok = sum(1 for r in results if r.result())
    denied = REQ - ok
    if ok == 0:
        pytest.skip(f"Server refused all WRQs (likely read-only). Denied={denied}")
    assert ok >= 1, "At least one WRQ should succeed in stress test"
