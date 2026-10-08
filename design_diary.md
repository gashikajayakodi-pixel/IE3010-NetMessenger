# NetMessenger Design Diary

Registration number: IT21928192

This diary was compiled retrospectively from the development
conversation, screenshots and Git commits for 7–8 October 2026.

## 7 October 2026 — Initial Setup

I reviewed the assignment requirements and configured Ubuntu on WSL,
GCC, Make, Git and VS Code. The numeric registration is 21928192.
Its last four digits are 8192, giving port 6000 + 8192 = 14192.
Digits 3–6 are 9281, giving NID:9281.

I created the GitHub repository and verified basic C compilation.
The personalised project files are server_8192.c, client_8192.c
and Makefile_8192.

## 7–8 October 2026 — Messaging and Rooms

The server uses a detached pthread for each client, with a mutex
protecting shared user and room state. This supports simultaneous
connections. The client uses poll() to monitor terminal input and
incoming messages. Holding the server mutex during outgoing transfers
can delay other clients when a recipient is slow.

I implemented and tested REGISTER, LIST, QUIT, BCAST and PMSG.
Five-client testing confirmed that a broadcast reached the four
other clients. I also tested JOIN, LEAVE, ROOMS and RMSG, including
room message delivery and an error after leaving a room.
Presence notifications showed users joining and leaving.

## 8 October 2026 — File Transfer and Logging

The main challenge was separating binary payloads from text commands.
The server reads exactly the declared file size, allowing newline and
NUL bytes inside files. Temporary uploads are renamed after completion
and removed if the connection closes before completion.

Files are stored under storage/IT21928192/<sender>/<filename>.
The client saves received files with unique suffixes. I chose a
1 MiB size limit and used a MSG FILE header for recipient framing.

Timestamped events are recorded in netmsg_IT21928192.log.
A later review added CONNECT and FILE_SENT entries. FILE_SENT records
completed socket sends rather than confirmation of recipient storage.

## 8 October 2026 — Debugging and Validation

A missing closing brace caused compiler errors and was corrected.
Selecting the correct terminal and distinguishing shell commands
from client commands were practical obstacles.

Tests verified text and binary transfers, room file delivery,
file-size rejection, fragmented commands and binary payloads,
multiple commands sent together, invalid commands and unknown file
targets. Byte comparisons confirmed that the tested stored and
delivered files matched their originals.

Disconnect testing confirmed username release and removal of previous
room membership. Interrupted uploads left no incomplete final file
or new temporary upload file. The personalised Makefile built both
programs without compiler warnings.