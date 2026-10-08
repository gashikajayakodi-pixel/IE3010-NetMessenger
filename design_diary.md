# NetMessenger Design Diary

Registration number: IT21928192
Development sessions: 7–8 October 2026

This diary summarises the development retrospectively using the
conversation, screenshots and Git commits.

## 7 October 2026 — Setup and Personalisation

I reviewed the assignment requirements and configured Ubuntu on WSL,
GCC, Make, Git and VS Code. I calculated the personalised values:

- Registration number: IT21928192
- Numeric part: 21928192
- Last four digits: 8192
- TCP port: 6000 + 8192 = 14192
- Numeric digits 3–6: 9281
- Node ID tag: NID:9281

I created the GitHub repository and used server_8192.c,
client_8192.c and Makefile_8192 as the project filenames.

## 7–8 October 2026 — Concurrency and Messaging

I used C and BSD TCP sockets with a detached pthread for each client.
A mutex protects shared user and room state. The client uses poll()
to monitor terminal input and incoming messages.

I implemented and tested REGISTER, LIST, QUIT, BCAST and PMSG,
followed by JOIN, LEAVE, ROOMS and RMSG. Presence notifications
showed users joining and leaving. Holding the server mutex during
outgoing transfers can delay other clients when a recipient is slow;
a send timeout bounds individual blocked sends.

## 8 October 2026 — File Transfer and Logging

I separated newline-delimited commands from binary file payloads.
The server reads exactly the declared file size and loops over partial
sends. Uploads use temporary files that are renamed after completion
and removed when an upload is interrupted.

Files are stored under storage/IT21928192/<sender>/<filename>.
I chose a 1 MiB size limit. The client uses a MSG FILE header for
recipient framing and saves files with unique suffixes.

I added timestamped logging in netmsg_IT21928192.log. A later review
added CONNECT and FILE_SENT entries. FILE_SENT records completed
socket sends rather than confirmation of recipient disk storage.

## 8 October 2026 — Debugging and Validation

I corrected a missing closing brace that caused compiler errors.
Selecting the correct terminal and distinguishing shell commands
from client commands were practical obstacles.

I tested five simultaneous clients, messaging, rooms, text and binary
file transfers, room file delivery and file-size rejection. Additional
tests covered fragmented commands and payloads, multiple commands
sent together, unknown commands, invalid file targets, disconnect
cleanup and interrupted uploads.

I used cmp to confirm that the tested stored and received files
matched the originals byte-for-byte. Disconnect testing confirmed
username release and removal of previous room membership.
Interrupted uploads left no incomplete final or new temporary file.
The personalised Makefile built both programs without warnings.