# CISC-210 FINAL-PROJECT
### TEAM MEMBERS: Haritima Manchanda, Becky Ren
### SECTION: CISC-210 (HONORS)

## BASIC OVERVIEW
In this project we had put together all of the concepts we have learned this semester to create a single client/server application that operates between two Raspberry Pi's and makes use of the led grid, and the joystick. The two Raspberry Pi's will connect, and send commands and responses back and forth between them to implement the program.

## PROJECT IMPLEMENTATION

### 1. STEP 1: Connecting our Pi's

Since we are at homes, our Raspberry Pi/s are behind a firewall.  To circumvent this we will use a feature of ssh that allows us to set up tunnels to redirect a port on our machine to a port on a remote machine and vice-versa.  Our application will support operation in both client and server mode, but one side will have to act as the server.  The side that is acting as the server will redirect a port on go.eecis.udel.edu to a port on their local machine.  The side that is acting as the client will redirect a port on their machine to a port on go.eecis.udel.edu.  Then the client machine will connect to localhost/port and the magic of ssh will redirect it to the server's raspberry Pi.
    An example:

On the server side, the user executes this command:
ssh -R 40000:localhost:8080 <eecis user>@go.eecis.udel.edu -fN
  
On the client side, the user executes the command:
ssh -L 8080:localhost:40000 <other eecis user>@go.eecis.udel.edu -fN
This connects port 8080 on the client to port 8080 on the server.
The -fN causes ssh to run in the background rather than launch a remote shell.
The tunnel stays active until you log out.
(Tested this with lab 7 and lab 8.  Server runs lab8 on port 8080.  client runs lab7 and uses localhost 8080 / as the parameters.)
 
### 2. STEP 2: The project

At the simplest level you are to write a program that takes either 2 or 3 arguments.
1) program <port>
2) program <port> <IP or Server Name>

Based on this, the program will either 1) start as the server, or 2) start as a client and try to connect to servername.

Once a connection is established, you will have to have both sides loop doing non-blocking recv calls (Set flag to MSG_DONTWAIT).  When one side wants to do something (for example the user on that side pressed the joystick), then that side will send a command to the other side.  This will cause the other side to actually receive something.  Based on the command, it will do something and perhaps send back a response.  The logic here can be simple.
Once you receive something (or send something), you can leave the loop and use blocking calls (with flag set to 0) to handle expected communication.

### 3.STEP 3: Program Description

A simple game: The program implements a game of pong where each screen is half of the court.  Use the joystick to control the paddle.  When the ball leaves the right side of your screen, send a message with it's location, direction, etc and have the other side pick up playing the game from that point (until someone misses).  Then send a message to the other side to update scores.  Keep score and print to the console on the Pi (i.e. with printf).
