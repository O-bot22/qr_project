# Test results

Results of 
Terminal 1:
  ./QRServer PORT 2012 RATE 3 60 MAX_USERS 3 TIME_OUT 80
Terminal 2:
  ./QRClient localhost 2012 tests/url.png tests/url.jpg
Showing successful decoding of two qr codes.

## Client 
### Commmand line output
tests/url.png: code=0 https://www.wpi.edu/
tests/url.jpg: code=0 https://www.wpi.edu/


### Logs:
student@csvm-ay2025-2026-1:~/qr_project$ cat server.jsonl

{"timestamp":"2026-11-11 02:46:03","client_ip":"0.0.0.0","client_id":66779,"event":"startup","error_code":0,"epoch_us":1794383163570235,"bytes":0}
{"timestamp":"2026-11-11 02:46:28","client_ip":"127.0.0.1","client_id":67204,"event":"connected","error_code":0,"epoch_us":1794383188924606,"bytes":0}
{"timestamp":"2026-11-11 02:46:28","client_ip":"127.0.0.1","client_id":67204,"event":"image_received","error_code":0,"epoch_us":1794383188931937,"bytes":1484}
{"timestamp":"2026-11-11 02:46:28","client_ip":"127.0.0.1","client_id":67204,"event":"success","error_code":0,"epoch_us":1794383188937848,"bytes":0}
{"timestamp":"2026-11-11 02:46:28","client_ip":"127.0.0.1","client_id":67204,"event":"image_received","error_code":0,"epoch_us":1794383188979647,"bytes":20642}
{"timestamp":"2026-11-11 02:46:28","client_ip":"127.0.0.1","client_id":67204,"event":"success","error_code":0,"epoch_us":1794383188987293,"bytes":0}
{"timestamp":"2026-11-11 02:46:29","client_ip":"127.0.0.1","client_id":67204,"event":"peer_closed_or_incomplete","error_code":0,"epoch_us":1794383189028494,"bytes":0}
{"timestamp":"2026-11-11 02:46:29","client_ip":"127.0.0.1","client_id":67204,"event":"disconnected","error_code":0,"epoch_us":1794383189028556,"bytes":0}

## Server
### Logs:
student@csvm-ay2025-2026-1:~/qr_project$ cat server.jsonl 

{"timestamp":"2026-11-11 02:46:03","client_ip":"0.0.0.0","client_id":66779,"event":"startup","error_code":0,"epoch_us":1794383163570235,"bytes":0}
{"timestamp":"2026-11-11 02:46:28","client_ip":"127.0.0.1","client_id":67204,"event":"connected","error_code":0,"epoch_us":1794383188924606,"bytes":0}
{"timestamp":"2026-11-11 02:46:28","client_ip":"127.0.0.1","client_id":67204,"event":"image_received","error_code":0,"epoch_us":1794383188931937,"bytes":1484}
{"timestamp":"2026-11-11 02:46:28","client_ip":"127.0.0.1","client_id":67204,"event":"success","error_code":0,"epoch_us":1794383188937848,"bytes":0}
{"timestamp":"2026-11-11 02:46:28","client_ip":"127.0.0.1","client_id":67204,"event":"image_received","error_code":0,"epoch_us":1794383188979647,"bytes":20642}
{"timestamp":"2026-11-11 02:46:28","client_ip":"127.0.0.1","client_id":67204,"event":"success","error_code":0,"epoch_us":1794383188987293,"bytes":0}
{"timestamp":"2026-11-11 02:46:29","client_ip":"127.0.0.1","client_id":67204,"event":"peer_closed_or_incomplete","error_code":0,"epoch_us":1794383189028494,"bytes":0}
{"timestamp":"2026-11-11 02:46:29","client_ip":"127.0.0.1","client_id":67204,"event":"disconnected","error_code":0,"epoch_us":1794383189028556,"bytes":0}
{"timestamp":"2026-11-11 02:52:02","client_ip":"0.0.0.0","client_id":1770,"event":"startup","error_code":0,"epoch_us":1794383522746106,"bytes":0}
{"timestamp":"2026-11-11 02:52:44","client_ip":"10.81.41.1","client_id":1771,"event":"connected","error_code":0,"epoch_us":1794383564309376,"bytes":0}
{"timestamp":"2026-11-11 02:52:44","client_ip":"10.81.41.1","client_id":1771,"event":"image_received","error_code":0,"epoch_us":1794383564309582,"bytes":1484}
{"timestamp":"2026-11-11 02:52:44","client_ip":"10.81.41.1","client_id":1771,"event":"success","error_code":0,"epoch_us":1794383564318255,"bytes":0}
{"timestamp":"2026-11-11 02:52:44","client_ip":"10.81.41.1","client_id":1771,"event":"image_received","error_code":0,"epoch_us":1794383564319234,"bytes":20642}
{"timestamp":"2026-11-11 02:52:44","client_ip":"10.81.41.1","client_id":1771,"event":"success","error_code":0,"epoch_us":1794383564327342,"bytes":0}
{"timestamp":"2026-11-11 02:52:44","client_ip":"10.81.41.1","client_id":1771,"event":"peer_closed_or_incomplete","error_code":0,"epoch_us":1794383564368695,"bytes":0}
{"timestamp":"2026-11-11 02:52:44","client_ip":"10.81.41.1","client_id":1771,"event":"disconnected","error_code":0,"epoch_us":1794383564368729,"bytes":0}
{"timestamp":"2026-11-11 02:57:57","client_ip":"0.0.0.0","client_id":1770,"event":"shutdown","error_code":0,"epoch_us":1794383877077918,"bytes":0}
