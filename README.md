# IE3010 — NetMessenger

Registration number: IT21928192

## Personalisation

- Numeric registration: 21928192
- Last four digits: 8192
- TCP port: 6000 + 8192 = 14192
- Node ID: digits 3–6 of 21928192 = 9281
- Server source: server_8192.c
- Client source: client_8192.c
- Makefile: Makefile_8192
- Server log: netmsg_IT21928192.log
- Server storage: ./storage/IT21928192/<sender_username>/<filename>
- Submission archive: IE3010_IT21928192.zip

## Requirements and build

Developed on Ubuntu 22.04 using WSL 2, GCC, GNU Make,
POSIX sockets and pthreads.

Run commands from the project directory:

```bash
make -f Makefile_8192
```

To rebuild both programs:

```bash
make -B -f Makefile_8192
```

To remove the compiled programs:

```bash
make -f Makefile_8192 clean
```

## Run

Start the server in one terminal:

```bash
./server_8192
```

Start each client in a separate terminal:

```bash
./client_8192
```

Both programs use 127.0.0.1:14192. This configuration supports
clients on the same Linux/WSL environment.

Type application commands at the client's `>` prompt.
Type shell commands at the terminal's `$` prompt.

## Client commands

Register before using other commands:

```text
REGISTER student
LIST
BCAST Hello everyone
PMSG student2 Hello privately
JOIN study
ROOMS
RMSG study Hello room
LEAVE study
SENDFILE student2 sample.txt
QUIT
```

SENDFILE accepts a username or room name as its target.
Use a plain filename in the client's current directory.
The client calculates the size automatically.

The wire command is:

```text
SENDFILE <target> <filename> <filesize>
```

Its newline is followed immediately by exactly `<filesize>`
binary bytes, without an extra newline after the payload.

## Server responses and notifications

Every OK and ERR response ends with ` NID:9281`.
Forwarded MSG lines do not have the NID suffix.

Examples:

```text
OK REGISTERED student NID:9281
OK USERS student,student2 NID:9281
OK SENT NID:9281
ERR 001 USERNAME_TAKEN NID:9281
ERR 002 USER_NOT_FOUND NID:9281
ERR 003 ROOM_NOT_FOUND NID:9281
ERR 004 FILE_TOO_LARGE NID:9281
```

Presence and message formats:

```text
MSG JOIN <username>
MSG LEAVE <username>
MSG BCAST <sender> <message>
MSG PRIV <sender> <message>
MSG ROOM <room> <sender> <message>
```

File delivery uses this implementation-defined extension:

```text
MSG FILE <sender> <filename> <filesize>
```

Exactly `<filesize>` binary bytes follow that header.
The client saves the file under
`./received/<sender>_<filename>_<unique_suffix>`.

OK FILE_RECEIVED confirms server storage.
Recipient delivery can fail separately if the recipient disconnects.

## Design and limits

- The server creates a detached pthread for each connection.
- Maximum simultaneous connections: 32.
- A mutex protects the client registry, room membership and socket writes.
- The client uses poll() to receive messages while waiting for keyboard input.
- Text commands use newline framing.
- File payloads use an exact byte count, including NUL and newline bytes.
- Maximum file size: 1 MiB (1,048,576 bytes), chosen for this implementation.
- Maximum rooms: 16; room names remain until the server restarts.
- User and room names contain 1–31 letters, digits, underscores or hyphens.
- Filenames contain up to 100 letters, digits, underscores, hyphens or dots.
- If a target matches both a user and a room, the user takes priority.
- RMSG requires room membership and goes to other connected room members.
- Room file delivery goes to other connected room members.
- Socket sends have a two-second timeout. Sends share the registry mutex,
  so slow recipients can temporarily delay other commands.
- Oversized or malformed file-size headers close the connection.
- Server logs contain local timestamps, commands, responses and disconnects.

## Validation completed

- GCC builds with -Wall -Wextra.
- Unique registration and duplicate username rejection.
- LIST and QUIT.
- Broadcast delivery and join/leave notifications.
- Private delivery and unknown-user error.
- JOIN, ROOMS, RMSG and LEAVE.
- RMSG rejection after leaving a room.
- Text file upload, storage and delivery, verified using cmp.
- An 8192-byte binary file containing all byte values,
  verified against both server storage and client delivery using cmp.
- Five simultaneous registered TCP connections and broadcast to the
  other four connections: five_clients_test.txt.
- Server rejection of a file size above 1 MiB: file_limit_test.txt.
- Timestamped logging of LIST, QUIT, responses and disconnects.

The five-connection and size-limit checks used Python as a test harness.
The application server and client are implemented in C.

## Remaining submission documents

The implementation report, design diary, AI prompt log and reflection
must accompany the source files and genuine test evidence.