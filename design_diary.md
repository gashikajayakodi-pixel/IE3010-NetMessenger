# NetMessenger Design Diary

Registration number: IT21928192
Development sessions: 7-8 October 2026

This diary was compiled retrospectively on 8 October from the development
conversation, screenshots and Git commits. It summarises actual decisions
and obstacles; it was not maintained as a daily diary during development.

## Architecture and concurrency

The application uses C and BSD TCP sockets. I used a single server process
with a detached pthread for each client. This allows multiple connections
to remain active while each handler reads its client's commands. Shared
user and room state is protected by a mutex.

The client uses poll to monitor terminal input and the server connection,
so it can receive messages while waiting for user input. A limitation is
that the server holds the shared mutex during outgoing messages and file
forwarding. A slow recipient can therefore delay other clients. A send
timeout bounds individual blocked sends.

## Protocol and file transfer

Commands and responses use newline framing. The server reads complete
command lines and loops over partial sends. For SENDFILE, it reads exactly
the declared byte count, keeping binary bytes separate from text parsing.

Uploads use a temporary file in the personalised storage directory.
A completed upload is renamed to its final filename; interrupted uploads
are removed. The chosen file-size limit is 1 MiB. This is an implementation
choice rather than a size specified by the brief.

The receiving client uses a MSG FILE header followed by the raw payload
and saves files with a unique suffix. This header is an implementation
extension for recipient-side framing because the brief does not define
a recipient file header. The upload acknowledgement confirms server
storage; FILE_SENT logging confirms socket sends, not recipient disk storage.

## Obstacles and changes

During manual editing, a missing closing brace caused multiple compiler
errors. Restoring the brace fixed them. Other difficulties included
selecting the correct terminal and distinguishing shell commands from
commands entered into the running client.

The first logging implementation recorded commands, responses and
disconnects. A later review added explicit CONNECT and FILE_SENT events.
An initial conclusion that testing was finished was revised after checking
the brief and identifying additional cases.

## Validation and remaining work

Tests covered five simultaneous clients, broadcast, private and room
messages, text and binary file transfer, the file-size limit, fragmented
commands and binary payloads, multiple commands sent together, unknown
commands, disconnect cleanup and interrupted uploads. Byte comparisons
confirmed the tested stored and delivered files matched their originals.
These results do not prove that every failure case is handled.

AI substantially assisted with code generation, debugging and test design;
its use is documented in prompt_log.md. The remaining submission work is
the evidenced implementation report, personal reflection and final archive.
