# AI Interaction Log

## 2026-10-07 — Environment setup and personalisation
Tool: ChatGPT / Codex

Prompts:
Asked for an explanation of the assignment, software installation
instructions, troubleshooting help, and personalised values for IT21928192.

Assistance received:
- Guided installation of Ubuntu on WSL 2, GCC, Make, Git, and VS Code.
- Provided hello.c to verify compilation and execution.

- Guided GitHub repository setup, commits, and pushes.
- Calculated the port, NID, filenames, and storage path.
- Drafted the initial README.

Use and verification:
Followed the installation steps and compiled and ran hello.c successfully.
Confirmed uploads on GitHub.
Added the personalised values to README.md.

Remaining work:
Review the personalisation calculations against the assignment.
Record further AI assistance, testing, and changes as development continues.![alt text](image.png)

## 2026-10-07 to 2026-10-08 — Implementation and debugging

Tool: ChatGPT / Codex.

This entry is a retrospective summary of the interaction.
The descriptions below are paraphrases, except where explicitly quoted.
The original conversation contains the full generated code and screenshots.

### Requests and context

I requested code ("give me code") and repeatedly supplied screenshots
of VS Code, terminal results and assignment requirements. I confirmed
completed steps using messages such as "done". The assistant guided
the implementation incrementally.

### AI assistance used

- Generated the C TCP server and client code.
- Added newline framing, partial-send handling and exact-size binary receiving.
- Added pthread-based concurrent connection handling and registry locking.
- Implemented REGISTER, LIST, QUIT and duplicate username rejection.
- Replaced the synchronous client with a poll-based client for asynchronous messages.
- Added BCAST, PMSG and join/leave notifications.
- Added JOIN, LEAVE, ROOMS and RMSG with room membership tracking.
- Added SENDFILE upload, personalised server storage and client file delivery.
- Proposed and implemented the MSG FILE header for forwarded binary files.
- Selected a documented implementation limit of 1 MiB.
- Added timestamped server logging.
- Generated the Makefile and expanded README documentation.
- Supplied Python test commands for five simultaneous connections
  and server-side file-size rejection.
- Supplied cmp commands for text and binary file integrity checks.
- Guided Git staging, meaningful commits and pushes.

### Debugging assistance

The assistant interpreted my screenshots and helped correct:

- Application commands entered at the Linux shell prompt.
- Expected output accidentally entered as a command.
- Code pasted into the wrong source file.
- Undo/redo confusion while restoring the threaded server.
- Missing include lines and macro definitions after editing.
- A missing closing brace in the deliver function.
- An incorrect QUIT condition at the beginning of the SENDFILE branch.
- Terminal selection mistakes during multi-client tests.

### Work performed and evidence checked

I pasted and saved the supplied code, compiled it on Ubuntu/WSL,
ran the programs and supplied screenshots of the results.

Observed checks included:

- Registration, duplicate username rejection, LIST and QUIT.
- Broadcast delivery and join/leave notifications.
- Private delivery and unknown-user rejection.
- Room creation, listing, messaging and leaving.
- Rejection of room messaging after leaving.
- Text file storage and delivery checked using cmp.
- 8192-byte binary file storage and delivery checked using cmp.
- Five simultaneous registered connections and broadcast to four recipients.
- Server rejection of a declared file size above 1 MiB.
- Timestamped command, response and disconnect logging.
- Building both C programs using Makefile_8192.

The Python commands were test harnesses, not the application implementation.

### Disclosure

AI assistance was substantial and included generation of application code,
test commands and documentation text. I do not claim that the generated
code was written independently without assistance.

Compilation and the recorded tests validate specific observed behaviours;
they do not establish that every possible failure case has been tested.
The README records implementation limits and delivery semantics.

Further AI assistance with the report, design diary and reflection
will be recorded in an additional entry.
## 2026-10-08 - Compliance review and additional validation

Tool: ChatGPT / Codex.
This is a retrospective paraphrase of the interaction.

Requests:
Asked whether the assignment met the PDF requirements, how protocol
features mapped to GitHub commits, whether values were hard-coded,
and what remained after a reported deadline extension.

Assistance received and used:
- Compared the assignment requirements with the observed implementation
  and identified missing documents and validation gaps.
- Guided adding CONNECT and FILE_SENT timestamped log entries.
- Supplied Python socket tests for fragmented commands, multiple commands
  in one send, fragmented binary payloads followed immediately by LIST,
  invalid commands, disconnect cleanup, interrupted uploads and unknown
  file targets.
- Guided a room file-transfer test and cmp checks for delivered and
  stored bytes.
- Supplied commands to capture listening-port, storage and log evidence.
- Drafted the additional README validation section and this log entry.

My actions and observed results:
I inserted the logging changes, saved and compiled the C server, restarted
it, ran the supplied tests and commands, and shared screenshots.
The recorded tests passed. I committed and pushed the code changes,
test-result files and personalisation evidence.

Corrections and limits:
The earlier statement that coding and tests were finished was too broad.
The later review identified additional tests and submission documents.
Passing these tests does not prove every failure case is handled.
FILE_SENT means the server completed socket sends; it does not confirm
that the receiving client saved the file to disk.

## 2026-10-08 - Design diary draft

Tool: ChatGPT / Codex.
Prompt: "then we can create design diary?"

The AI drafted design_diary.md using the development conversation,
observed tests and commit history. It described concurrency, framing,
file storage, debugging obstacles and implementation limitations.
I created the file using the supplied terminal command and inspected
its beginning and ending in VS Code. The draft explicitly identifies
itself as retrospective rather than a diary maintained during development.
The final document still requires my review and formatting to 0.5-1 page.

## 2026-10-08 - Reflection drafting

Tool: ChatGPT / Codex.
I requested a reflection addressing the four assignment questions and
provided my own comments in Sinhala about the value of testing, verifying
file transfers and testing simultaneous clients.

The AI drafted the English reflection using my comments and the recorded
development experience. I requested removal of a sentence about further
viva preparation and replacement with practical learning about how the
protocol commands work. I saved the revised draft in reflection.md.
The wc -w command reported 393 words, including headings and Markdown.
