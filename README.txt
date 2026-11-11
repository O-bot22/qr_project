CS 3516 QR client/server implementation and test package
AI-generated code and test scaffolding, 2026-11-10

STATUS
This is a working implementation with local test evidence, not a completed
submission. Read STUDENT_WORK.txt before submitting. No student-authored
protocol specification, reflection, library justification, explain-back answer,
or packet-capture verdict has been generated or backdated.

BUILD (Linux)
  make all
Requires make, a C compiler, and a C++17 compiler. No package downloads are
needed to build: quirc and stb_image sources are included in vendor/.
Run executables from this directory: the server invokes ./QRDecode.
  make clean
removes all compiled outputs, preserving evidence and input files.

RUN
Terminal 1:
  ./QRServer PORT 2012 RATE 3 60 MAX_USERS 3 TIME_OUT 80
Terminal 2:
  ./QRClient localhost 2012 tests/url.png tests/url.jpg
Options can appear in any order. Defaults are port 2012, 3 requests per
connection per 60 seconds, 3 concurrent users, and 80 seconds inactivity.
MAX USERS and TIME OUT are also accepted as two separate words or one quoted
argument. Port must be 2000 through 3000. Ctrl+C stops the server and reaps
workers. A request error makes the client exit status nonzero, but it continues
through remaining file arguments unless the connection is closed or broken.

IMAGE-ONLY REQUESTS
Every client request is a 32-bit unsigned image-file length followed by exactly
that many image bytes. The length uses network byte order. There is no type
field and there are no mode switches. Every payload is treated as an image for
PNG/JPEG QR decoding. Invalid images receive code 1 with an empty response body;
the connection remains usable. Server responses remain a 32-bit return code,
a 32-bit response-byte length, and the response bytes.
These implementation notes follow the framing in CS3516 Project Spec.pdf;
they do not replace the student's own specification.

TESTS
  make test
One Python script runs five clearly labeled groups:
  1. Six valid/invalid input files.
  2. A PNG uploaded in small pieces, with received-byte-count verification.
  3. Two overlapping clients and rejection of a third client.
  4. Two faults, each tested twice: killed client and connection reset.
  5. A request using the supplied C++ client.

Python 3's standard library is sufficient. Port 2316 must be free. The script
starts one server, runs 19 checks, stops it, and writes tests/results.md plus
the evidence logs. No separate C++ test executable or threading test is needed.
Run the same command on Secnet to collect Secnet evidence. Small writes alone
do not prove TCP packet boundaries, because the kernel can coalesce writes.

The smaller suite does not separately exercise rate-window expiry, inactivity
timeouts, or oversized/extraneous-byte recovery. Those behaviors remain in the
server; this suite focuses on the updated assignment's evidence requirements.

ROADMAP 4 - STUDENT EXPLANATION (COMPLETE IN YOUR OWN WORDS)
The updated assignment replaces the slow-client demo with these answers.
The earlier AI policy still reserves explain-back answers for the students.
1. Which call would block a single-threaded server, and on what?
  A single threaded server would be blocked on the recv call. When a client connects,
  but does not immediately send a QR code, then there is a chance another user could
  connect and send a QR code. In such a case, the second client must wait until the
  server has finished talking to the first client, which is dependent on the time it
  takes for the first server to send its QR code, or by a timeout.
2. How does your design serve clients at the same time?
  Our server is able to handle concurrent clients by maintaining a master thread
  inititally, and then forking off child processes to handle each client. Thus, 
  each child is able to wait on its own time for its client to send an image without
  affecting the behavior of the other clients.
3. What happens when your client limit is reached?
  In our server, once the client limit is reached, the server logs it as a "server_busy"
  error, and then sends an error code message back to the client. The server log is 
  useful so that a network administrator would be able to see how often clients are
  denied, and if the MAX_USERS parameter should be increased. By sending an error
  message back to the client, the client is in theory able to retry after some amount
  of time and or tell the user that the server is busy.

Evidence: roadmap4/evidence_log.txt is an unedited excerpt from an actual local
run. It includes two distinct client IDs with successful decodes and overlapping
connection intervals. epoch_us supplies microsecond timestamps. The integration
test regenerates the excerpt and checks the overlap. No serial mode or separate
slow-client demonstration script is included.

ROADMAP 1 - BINARY FILE RECEIPT
The fragmented-write test sends tests/url.png, which the final server accepts.
evidence/small_segment_receipt.jsonl records its received byte count. The test
compares that count with the file size and checks that decoding returns the
expected URL. This verifies complete receipt and decoding, not byte-for-byte
identity. Small TCP segments still need verification on Secnet; small writes
may be coalesced.

ROADMAP 5 - TWO SELECTED FAULTS
The selected faults are client killed mid-transfer and connection reset. See
 docs/fault_table.md, evidence/fault_log.jsonl, and tests/results.md.
The script observes the server's EOF/reset logs and verifies a successful
request after each fault. It repeats both cases as the retest.

FILES
  common.hpp         client/send transfer helper, framing helpers, numeric checks
  QRServer.cpp       blocking receive_all loop, forked sessions, limits, logging
  QRClient.cpp       DNS resolution, uploads, bounded response reading
  QRDecode.cpp       subprocess decoder using included quirc and stb_image
  vendor/            decoder dependencies and their license notices
  tests/             reproducible tests and all input files
  evidence/          actual local build output, results, logs, source hashes
  roadmap4/          actual overlapping-client log excerpt
  docs/              two-fault technical table and supplied student spec PDF
  Logs/              sample log from the local regression run
  tests/results.md   expected versus actual results from the recorded run
  AI_USE_LOG.txt     factual prompt record; student decisions remain unfilled
  STUDENT_WORK.txt   required work and evidence still to complete
  THIRD_PARTY.txt    provenance and local vendor changes

LIMITS AND ASSIGNMENT AMBIGUITIES TO REVIEW
- Image uploads are capped at 4 MiB. Only PNG/JPEG are enabled. Decoded images
  are capped at 4096 per axis and 16 million pixels. The decoder runs with
  256 MiB address-space, 3 CPU-second and 5 wall-second limits. This is resource
  containment, not a complete security sandbox or proof of parser safety.
- One request is outstanding at a time: wait for a response before sending
  another. Bytes already queued beyond the declared payload are discarded.
  No TCP framing can distinguish arbitrarily late extra bytes from the next
  request without an additional delimiter/handshake. Snapshot draining is
  bounded to prevent an endless incoming stream from trapping the worker.
- Oversized headers are rejected immediately with code 1 and no payload;
  currently queued bytes are discarded. The connection stays open. A sender
  must stop that upload on the error and resume at a clean request boundary.
  Arbitrarily delayed oversized bodies cannot be reliably resynchronized by
  this length-only protocol. Confirm the desired interpretation with the TA.
- The PDF demands a human-readable busy error but defines no busy code, and
  requires an empty body for code 1. This implementation uses code 1 with an
  empty body, then closes, and logs server_busy. That follows the defined wire
  codes but cannot also supply the requested explanatory text. Ask the TA
  which requirement takes precedence before changing it.!!!!!!!!!!!!!!!!!!!!!!!!!!!
- Rate accounting is per connection, including complete invalid requests;
  rejected oversized headers are not counted. Reconnecting resets the counter.
  There is no authenticated user identity in this protocol. This is not a
  persistent per-IP abuse-control service.
- Server reads use receive_all() with blocking recv() and SO_RCVTIMEO. Each
  recv waits up to TIME_OUT seconds for more bytes. EOF ends the session;
  EAGAIN/EWOULDBLOCK reports a timeout; EINTR retries. During shutdown the
  worker socket is shut down to wake a blocked recv immediately.
- Inactivity is measured while awaiting incoming bytes. Decoding is separately
  bounded. A client trickling bytes faster than the timeout remains active;
  there is no total upload-duration limit.
- Logs are JSON lines in server.jsonl with timestamp, client_ip, client_id
  (worker PID), event, error_code, epoch_us, and byte count. They use local server time. JSON records
  start with the timestamp field, rather than a literal unstructured timestamp
  prefix; this satisfies the rubric's machine-parseable format.
- TCP connect uses the OS timeout; the client's subsequent I/O timeout is 90s.
