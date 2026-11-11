#!/usr/bin/env python3
"""Run from any directory with: python3 tests/integration.py"""
import errno
import json
import os
from pathlib import Path
import socket
import struct
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parent.parent
os.chdir(ROOT)
PORT = 2316
URL = b'https://www.wpi.edu/'
results = []


def connect():
    sock = socket.create_connection(('127.0.0.1', PORT), timeout=3)
    sock.settimeout(5)
    return sock


def read_exact(sock, count):
    data = b''
    while len(data) < count:
        part = sock.recv(count - len(data))
        if not part:
            raise EOFError('Server closed before sending the complete response')
        data += part
    return data


def response(sock):
    code, size = struct.unpack('!II', read_exact(sock, 8))
    assert size <= 4 * 1024 * 1024, 'Response is too large'
    return code, read_exact(sock, size)


def send_image(sock, image):
    sock.sendall(struct.pack('!I', len(image)) + image)
    return response(sock)


def check(name, actual, expected):
    passed = actual == expected
    results.append((name, repr(expected), repr(actual), passed))
    print(('PASS: ' if passed else 'FAIL: ') + name, flush=True)
    assert passed, f'Expected {expected!r}; received {actual!r}'


def log_lines():
    return Path('server.jsonl').read_text().splitlines()


def wait_for_event(event, start, count=1):
    # Wait for the server's actual log entry instead of guessing its timing.
    deadline = time.monotonic() + 3
    while time.monotonic() < deadline:
        entries = [json.loads(line) for line in log_lines()[start:]]
        matches = [entry for entry in entries if entry['event'] == event]
        if len(matches) >= count:
            return matches
        time.sleep(0.02)
    raise AssertionError('Missing server event: ' + event)


# This child is used only for the killed-client fault: send an unfinished upload.
if '--hold' in sys.argv:
    with connect() as sock:
        sock.sendall(struct.pack('!I', 100) + b'x')
        print('ready', flush=True)
        sock.recv(1)
    sys.exit(0)


for folder in ['evidence', 'roadmap4', 'Logs']:
    Path(folder).mkdir(exist_ok=True)
# Preserve older runs; each evidence excerpt starts at this run's log offset.
Path('server.jsonl').touch()
run_start = len(log_lines())
server = subprocess.Popen(['./QRServer', 'PORT', str(PORT),
                           'RATE', '100', '60', 'MAX_USERS', '2', 'TIME_OUT', '2'])
try:
    wait_for_event('startup', run_start)
    if server.poll() is not None:
        raise RuntimeError('Server failed to start')
    qr = Path('tests/url.png').read_bytes()

    # 1. Six input files: two valid QR images and four invalid inputs.
    for filename in ['url.png', 'url.jpg', 'not_image.txt',
                     'blank.png', 'truncated.png', 'hostile.bin']:
        with connect() as sock:
            expected = (0, URL) if filename in ['url.png', 'url.jpg'] else (1, b'')
            check(filename, send_image(sock, Path('tests', filename).read_bytes()), expected)
        wait_for_event('disconnected', run_start, len(results))

    # 2. Upload one real PNG in small writes and check its complete receipt.
    start = len(log_lines())
    with connect() as sock:
        packet = struct.pack('!I', len(qr)) + qr
        for offset in range(0, len(packet), 7):
            sock.sendall(packet[offset:offset + 7])
            time.sleep(0.001)
        check('small-write PNG decoded', response(sock), (0, URL))
    receipt = wait_for_event('image_received', start)[0]
    check('full PNG byte count', receipt['bytes'], len(qr))
    wait_for_event('disconnected', start)
    Path('evidence/small_segment_receipt.jsonl').write_text('\n'.join(log_lines()[start:]) + '\n')

    # 3. Two clients overlap; a third is refused while both slots are occupied.
    start = len(log_lines())
    with connect() as first, connect() as second:
        first.sendall(struct.pack('!I', len(qr)) + qr[:10])
        check('second client decoded while first uploads', send_image(second, qr), (0, URL))
        with connect() as third:
            check('third client refused', response(third), (1, b''))
        first.sendall(qr[10:])
        check('first client decoded', response(first), (0, URL))
    ends = wait_for_event('disconnected', start, 2)
    begins = wait_for_event('connected', start, 2)
    check('two client sessions overlap', max(e['epoch_us'] for e in begins) <
          min(e['epoch_us'] for e in ends), True)
    Path('roadmap4/evidence_log.txt').write_text('\n'.join(log_lines()[start:]) + '\n')

    # 4. Run each selected fault twice, including a healthy request after it.
    fault_start = len(log_lines())
    for attempt in [1, 2]:
        start = len(log_lines())
        child = subprocess.Popen([sys.executable, __file__, '--hold'], stdout=subprocess.PIPE, text=True)
        try:
            assert child.stdout.readline().strip() == 'ready'
            wait_for_event('connected', start)
            child.kill()
            child.wait(timeout=3)
        finally:
            if child.poll() is None:
                child.kill()
                child.wait()
            child.stdout.close()
        wait_for_event('peer_closed_or_incomplete', start)
        wait_for_event('disconnected', start)
        with connect() as sock:
            check(f'killed-client recovery, run {attempt}', send_image(sock, qr), (0, URL))
        wait_for_event('disconnected', start, 2)

        start = len(log_lines())
        with connect() as sock:
            wait_for_event('connected', start)
            sock.sendall(struct.pack('!I', 100) + b'x')
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_LINGER, struct.pack('ii', 1, 0))
        reset = wait_for_event('receive_error', start)[0]
        check(f'connection reset detected, run {attempt}', reset['error_code'], errno.ECONNRESET)
        wait_for_event('disconnected', start)
        with connect() as sock:
            check(f'reset recovery, run {attempt}', send_image(sock, qr), (0, URL))
        wait_for_event('disconnected', start, 2)
    Path('evidence/fault_log.jsonl').write_text('\n'.join(log_lines()[fault_start:]) + '\n')

    # 5. Check the supplied C++ client too, rather than only the Python sender.
    client = subprocess.run(['./QRClient', 'localhost', str(PORT), 'tests/url.png'],
                            capture_output=True, text=True, timeout=5)
    check('C++ client decodes PNG', (client.returncode, 'https://www.wpi.edu/' in client.stdout), (0, True))
finally:
    server.terminate()
    try:
        server.wait(timeout=8)
    except subprocess.TimeoutExpired:
        server.kill()
        server.wait()
        raise
    Path('Logs/sample_server.log').write_text('\n'.join(log_lines()[run_start:]) + '\n')
    report = '# Test results\n\nActual local run; rerun on Secnet for Secnet evidence.\n\n'
    report += '| Test | Expected | Actual | Result |\n| --- | --- | --- | --- |\n'
    for name, expected, actual, passed in results:
        report += f'| {name} | {expected} | {actual} | {"PASS" if passed else "FAIL"} |\n'
    Path('tests/results.md').write_text(report)
print(f'{len(results)} checks passed.')
