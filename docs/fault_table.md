# Two-fault test record

AI-generated technical test record from actual local TCP tests, not Secnet.
The existing handling was tested and retested; no failing pre-fix run is claimed.

| Fault | Expected | Observed | Fix / implemented handling | Retest |
| --- | --- | --- | --- | --- |
| Client killed mid-transfer | Release the incomplete session and keep serving other clients. | A child client announced 100 bytes, sent 1 byte, and was killed. The server logged `peer_closed_or_incomplete` and `disconnected`. A new client decoded the PNG successfully. | `receive_all()` returns on EOF; `session()` stops processing the incomplete upload and the worker closes its socket. The parent reaps the worker and releases the slot. | Repeated with another killed child. The same log events occurred and the next PNG request succeeded. |
| Connection reset | Handle the receive error without crashing; continue serving new requests. | A partial-upload socket closed with `SO_LINGER={1,0}`. The server logged `receive_error` with Linux `ECONNRESET` (104), then `disconnected`. A new client decoded the PNG successfully. | `receive_all()` returns a failure on unrecoverable socket errors; the session ends and the worker closes its socket. | Repeated the reset. Error 104 was detected again and the next PNG request succeeded. |

## Reproduce

```sh
make test
```

Section 4 of `tests/integration.py` performs both faults twice. Each case waits
for the relevant server log events before checking recovery. The killed client
is a real subprocess, not simply a socket closed by the test runner.

Evidence: `evidence/fault_log.jsonl` and `tests/results.md`.
