# IPC-examinator

_C(V) driven development_ **by Dawid Olszewski and Mateusz Cerulewicz**

Fully made in C programinng language.

The Image can by downloaded from [here](https://hub.docker.com/repository/docker/dawidolszewski/ipc-examinator/general)

Technologies, that were used here:

- Inter Proccess Communication by UNIX sockets and shared memory.
- Proccess Forking
- Multithreading
- Multistage Dockerfile
- Client-Server Architecture
- Write from/to files

## How to use it

1. Get the executable:
   there are two options:

- forking repo and compiling on your own, using commands below (of course gcc is necessary):

  `gcc server.c server-utils.c errors.c player.c global.c game.c stopwatch.c file-access.c time-utils.c -pthread -lrt -lm -o server.o `

  `gcc client.c errors.c -o client.o`

- using docker:

  `docker run -itd --name ipc-examinator dawidolszewski/ipc-examinator:latest`

2. run server `./server.o` (this is our broker, in this terminal we can see the logs and get the score board. We can also finish the game by simple closing this process)
3. run as many clients as you wish (everyone should be in its own terminal) `./client.o {username of player}` example `./client.o Dawid`
4. on servers terminal write `start`. From now server is closed for new players, because game is in progress already.
5. every player gets one of question and set of possible anwsers (A,B,C,D), player has to chose one and send it to server.
6. after player replies, he receives the scoreboard, and waits for others to finish.
7. the new question is generated and the proccess repeats until there are no more question.
8. the score is saved is specified file, and the game is over.

**GOOD LUCK**
