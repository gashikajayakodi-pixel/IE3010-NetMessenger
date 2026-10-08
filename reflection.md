# Reflection on Learning and AI Use

## 1. AI tools and stages of use

I used ChatGPT/Codex during environment setup, implementation, debugging,
testing and documentation. It helped me configure the development tools,
calculate the personalised values for IT21928192, and generate substantial
parts of the C server and client. It also provided explanations, Git
commands and Python socket tests. I recorded this assistance in the AI
prompt log rather than presenting the generated code as independently
written.

## 2. What AI did well and where it went wrong

AI was useful for dividing the implementation into manageable steps and
helping diagnose compiler errors. Its testing guidance was particularly
valuable because it demonstrated why assumptions must be checked against
actual network behaviour. However, an earlier statement that coding and
testing were complete was too broad. Reviewing the assignment brief
revealed additional validation and documentation requirements. There was
also a misunderstanding about removing highlighted README text, which
caused an unnecessary change and reversal. These experiences showed me
that AI guidance needs careful checking against my intended request and
the specification.

## 3. Changes and additions to AI output

With further AI guidance, I added explicit connection and file-send
logging. I also ran additional tests for fragmented commands, multiple
commands sent together, fragmented binary transfers, invalid commands,
disconnect cleanup and interrupted uploads. These checks provided
evidence beyond successful compilation. Byte comparisons verified that
the tested stored and delivered files matched their originals. I retained
the explanation that a successful socket send does not confirm that the
recipient saved the file. I also identified the design diary as
retrospective rather than suggesting that it had been maintained
throughout development.

## 4. Learning and understanding

The testing stage gave me greater confidence in understanding how the
complete client-server system behaves. Verifying file transfers and
testing several simultaneous clients helped connect the implementation
to observable results. This assignment significantly improved my
practical understanding of TCP socket programming. Running commands
helped me understand how REGISTER identifies a connected user, LIST
retrieves registered users, BCAST sends messages to other clients, and
PMSG targets a specific user. JOIN, LEAVE and RMSG demonstrated how room
membership controls message delivery. SENDFILE showed why the server
must read exactly the declared number of bytes, while QUIT demonstrated
a clean connection closure. Testing these behaviours showed me the
importance of validating assumptions through actual network communication.