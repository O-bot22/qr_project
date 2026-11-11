# Test results

# Sucessful decoding of two qr codes.
Results of 
Terminal 1:
  ./QRServer PORT 2012 RATE 3 60 MAX_USERS 3 TIME_OUT 80
Terminal 2:
  ./QRClient localhost 2012 tests/url.png tests/url.jpg

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

# Sending not an image

Response to client

student@csvm-ay2025-2026-1:~/qr_project$ ./QRClient 10.81.41.2 2012 tests/not_image.txt 
tests/not_image.txt: code=1 

Server log

{"timestamp":"2026-11-11 15:12:45","client_ip":"10.81.41.1","client_id":3320,"event":"connected","error_code":0,"epoch_us":1794427965503573,"bytes":0}
{"timestamp":"2026-11-11 15:12:45","client_ip":"10.81.41.1","client_id":3320,"event":"image_received","error_code":0,"epoch_us":1794427965503831,"bytes":15}
{"timestamp":"2026-11-11 15:12:45","client_ip":"10.81.41.1","client_id":3320,"event":"invalid_request","error_code":1,"epoch_us":1794427965505952,"bytes":0}
{"timestamp":"2026-11-11 15:12:45","client_ip":"10.81.41.1","client_id":3320,"event":"peer_closed_or_incomplete","error_code":0,"epoch_us":1794427965506366,"bytes":0}
{"timestamp":"2026-11-11 15:12:45","client_ip":"10.81.41.1","client_id":3320,"event":"disconnected","error_code":0,"epoch_us":1794427965506383,"bytes":0}

# Sending a blank PNG

Response to client

student@csvm-ay2025-2026-1:~/qr_project$ ./QRClient 10.81.41.2 2012 tests/blank.png 
tests/blank.png: code=1 

Server log

{"timestamp":"2026-11-11 15:13:25","client_ip":"10.81.41.1","client_id":3325,"event":"connected","error_code":0,"epoch_us":1794428005137851,"bytes":0}
{"timestamp":"2026-11-11 15:13:25","client_ip":"10.81.41.1","client_id":3325,"event":"image_received","error_code":0,"epoch_us":1794428005138057,"bytes":358}
{"timestamp":"2026-11-11 15:13:25","client_ip":"10.81.41.1","client_id":3325,"event":"invalid_request","error_code":1,"epoch_us":1794428005142126,"bytes":0}
{"timestamp":"2026-11-11 15:13:25","client_ip":"10.81.41.1","client_id":3325,"event":"peer_closed_or_incomplete","error_code":0,"epoch_us":1794428005142613,"bytes":0}
{"timestamp":"2026-11-11 15:13:25","client_ip":"10.81.41.1","client_id":3325,"event":"disconnected","error_code":0,"epoch_us":1794428005142639,"bytes":0}

# Sending a truncated png

Response to client

student@csvm-ay2025-2026-1:~/qr_project$ ./QRClient 10.81.41.2 2012 tests/truncated.png 
tests/truncated.png: code=1 

Server log

{"timestamp":"2026-11-11 15:14:01","client_ip":"10.81.41.1","client_id":3327,"event":"connected","error_code":0,"epoch_us":1794428041586763,"bytes":0}
{"timestamp":"2026-11-11 15:14:01","client_ip":"10.81.41.1","client_id":3327,"event":"image_received","error_code":0,"epoch_us":1794428041586849,"bytes":60}
{"timestamp":"2026-11-11 15:14:01","client_ip":"10.81.41.1","client_id":3327,"event":"invalid_request","error_code":1,"epoch_us":1794428041590559,"bytes":0}
{"timestamp":"2026-11-11 15:14:01","client_ip":"10.81.41.1","client_id":3327,"event":"peer_closed_or_incomplete","error_code":0,"epoch_us":1794428041590957,"bytes":0}
{"timestamp":"2026-11-11 15:14:01","client_ip":"10.81.41.1","client_id":3327,"event":"disconnected","error_code":0,"epoch_us":1794428041590997,"bytes":0}


# Sending a malicious file

Response to client

student@csvm-ay2025-2026-1:~/qr_project$ ./QRClient 10.81.41.2 2012 tests/hostile.bin
tests/hostile.bin: code=1 

Server log

{"timestamp":"2026-11-11 15:14:30","client_ip":"10.81.41.1","client_id":3329,"event":"connected","error_code":0,"epoch_us":1794428070755210,"bytes":0}
{"timestamp":"2026-11-11 15:14:30","client_ip":"10.81.41.1","client_id":3329,"event":"image_received","error_code":0,"epoch_us":1794428070755327,"bytes":4104}
{"timestamp":"2026-11-11 15:14:30","client_ip":"10.81.41.1","client_id":3329,"event":"invalid_request","error_code":1,"epoch_us":1794428070758885,"bytes":0}
{"timestamp":"2026-11-11 15:14:30","client_ip":"10.81.41.1","client_id":3329,"event":"peer_closed_or_incomplete","error_code":0,"epoch_us":1794428070759249,"bytes":0}
{"timestamp":"2026-11-11 15:14:30","client_ip":"10.81.41.1","client_id":3329,"event":"disconnected","error_code":0,"epoch_us":1794428070759280,"bytes":0}
{"timestamp":"2026-11-11 15:15:18","client_ip":"0.0.0.0","client_id":3316,"event":"shutdown","error_code":0,"epoch_us":1794428118321923,"bytes":0}