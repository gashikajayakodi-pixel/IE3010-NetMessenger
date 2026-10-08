# AI Interaction Log

This log summarises substantive ChatGPT/Codex assistance used for
the assignment. The original conversation contains the detailed
instructions, generated code and screenshots.

## 2026-10-07 — Environment setup and personalisation

Tool: ChatGPT / Codex.

### Requests and context

I requested explanations of the assignment requirements, development
environment setup, troubleshooting and personalised values for
registration number IT21928192.

### Assistance received and used

- Guided installation and configuration of Ubuntu on WSL, GCC, Make,
  Git and VS Code.
- Provided a basic hello.c program to verify compilation and execution.
- Explained the personalisation calculations:
  numeric registration 21928192, last four digits 8192,
  port 6000 + 8192 = 14192, and digits 3–6 giving NID:9281.
- Guided the personalised source filenames, Makefile name, log filename
  and storage directory.
- Provided guidance for GitHub repository setup, commits and pushes.

### My actions

I followed the setup instructions, ran compilation commands, created
the repository and shared screenshots of results and errors.

## 2026-10-07 to 2026-10-08 — Implementation and debugging

Tool: ChatGPT / Codex.

### Requests and context

I asked for guidance on implementing the assignment requirements,
understanding the code and resolving errors. I shared screenshots
of source code, terminal output and test results to obtain feedback
and instructions for the next steps.

I requested help with client-server communication, messaging, rooms,
file transfer, compilation, testing and GitHub commits. I ran the
suggested commands and shared further screenshots to check the results.

### Assistance received and used

ChatGPT/Codex generated substantial parts of the C server and client
and provided explanations and debugging guidance for:

- BSD TCP sockets and a pthread-based multi-client server.
- Shared user and room state protected by a mutex.
- A poll-based client that receives messages while accepting input.
- REGISTER, LIST and QUIT, including duplicate username handling.
- BCAST, PMSG and user join/leave notifications.
- JOIN, LEAVE, ROOMS and RMSG.
- Partial sends and newline-delimited command processing.
- SENDFILE payload byte counting, server storage and recipient delivery.
- Temporary upload files and cleanup of incomplete uploads.
- A recipient MSG FILE header followed by binary payload bytes.
- A chosen 1 MiB file-size limit.
- Timestamped server logging.
- The personalised Makefile and README.
- Python socket tests and cmp checks.
- Explicit Git staging, commits and pushes.

### Debugging and changes

I shared screenshots of compiler errors and terminal output.
The AI helped identify a missing closing brace and guided its correction.
It also provided instructions for selecting the appropriate terminal
and distinguishing shell commands from commands entered into the client.

### My actions and observed results

I inserted and saved the supplied code, compiled both programs,
ran clients and tests, and shared screenshots of the results.

Observed tests included registration, duplicate username rejection,
user listing, broadcast and private messaging, room commands,
text file transfer and binary file transfer.

The five-client test confirmed simultaneous registered connections
and broadcast delivery to the four other clients. File-size testing
confirmed rejection above the chosen 1 MiB limit. Byte comparisons
confirmed that the tested stored and received files matched the originals.

### Disclosure and interpretation

AI substantially assisted with code generation, debugging and test
design. The generated code is not presented as independently written
without assistance.

Successful compilation and recorded tests demonstrate the specific
behaviours checked. They do not establish that every failure case
has been tested. The README records implementation choices and limits.

## 2026-10-08 — Compliance review and additional validation

Tool: ChatGPT / Codex.

### Requests

I asked whether the implementation met the PDF requirements and how
the protocol features corresponded to the GitHub commits.

### Assistance received and used

- Compared the requirements with the observed implementation and
  identified documentation and validation gaps.
- Guided adding CONNECT and FILE_SENT timestamped log entries.
- Supplied Python socket tests for fragmented commands and multiple
  commands sent together.
- Supplied a fragmented binary transfer test with LIST immediately
  after the payload.
- Supplied tests for invalid commands, disconnect cleanup,
  interrupted uploads and unknown file targets.
- Guided a room file-transfer test and cmp checks.
- Provided commands to capture listening-port, storage and log evidence.
- Drafted the additional README validation section.

### My actions and observed results

I saved and compiled the logging changes, restarted the server,
ran the tests and shared screenshots.

The recorded tests passed. Room file delivery and server storage
matched the original file. Disconnect testing confirmed username
release and removal of previous room membership. Interrupted uploads
left no incomplete final file or new temporary upload file.

I committed and pushed the code changes, test-result files,
personalisation evidence and README update.

### Corrections and limits

An earlier statement that coding and testing were complete was too
broad. The later review identified additional tests and documents.

FILE_SENT records completion of socket sends; it does not confirm
that the receiving client saved the file to disk.

## 2026-10-08 — Design diary draft

Tool: ChatGPT / Codex.

### Requests

I asked how to structure and prepare the design diary using actual
development decisions, obstacles and test results. I also requested
instructions for saving the draft and uploading it to GitHub.

### Assistance received and used

ChatGPT/Codex drafted the diary from the development conversation,
screenshots and commit history. It described concurrency, framing,
file storage, debugging obstacles and validation.

### My actions

I created design_diary.md, inspected its content in VS Code,
and used the suggested Git commands to commit and push it.
The diary identifies itself as a retrospective summary.

## 2026-10-08 — Reflection drafting

Tool: ChatGPT / Codex.

### Requests

I provided my own comments about the value of testing, verifying
file transfers and testing simultaneous clients. I asked for help
structuring a reflection addressing the four assignment questions.

I also asked how to save the reflection and upload it to GitHub.

### Assistance received and used

The AI drafted the reflection using my comments and the recorded
development experience. I requested a revision to focus the learning
section on practical understanding of the protocol commands.
The AI supplied the revised wording and file-saving instructions.

### My actions

I saved the revised draft in reflection.md and checked its word count.
The wc -w command reported 393 words, including headings and Markdown.
I committed and pushed the reflection and recorded the drafting assistance.

## 2026-10-09 — Design diary and prompt log revision

Tool: ChatGPT / Codex.

### Requests

I requested dated design diary sections, first-day port and NID
calculations, and descriptions matching implemented features and
observed tests.

I then requested clearer wording for the prompt log's requests,
design diary and reflection entries, followed by a complete log.

### Assistance received and used

It supplied revised prompt log wording describing the guidance sought,
screenshots shared and how the outputs were used, while retaining
the record of substantive AI assistance.

### My actions

I saved, committed and pushed the revised design diary.
The revised prompt log was supplied for my review and saving.