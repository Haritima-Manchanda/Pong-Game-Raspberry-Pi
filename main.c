#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sense/sense.h>
#include <linux/input.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


// SPEED: the speed of the paddle with which it moves when joystick is pressed either left or right. 
#define SPEED 1
#define MAX 4096
int  initializeGameSetUp();

//GLOBAL VARIABLES
//run: Keeps account of whether the program is running.
//score: Keeps account of score of each team.
//runJoyStick: Keeps account of whether the joystick is presses and also the direction (left or right) in which it is pressed.
//serverRunning: Keeps account of whether server is running
//clientRunning: Keeps account of whether client is running
int run = 1; 
int scorePlayer = 0;
int runJoyStick = 0;
int startingPaddleIndex = 0;
int serverRunning = 0;
int clientRunning = 0;

pi_framebuffer_t *fb;

//ballXVel and ballYVel : Used to change direction of ball 
int ballXVel;
int ballYVel;

//structure to contain the coordinates of ball
typedef struct
{
	int ballx;
	int bally;
	int ballxprev;
	int ballyprev;
}gamestate_t;

// handler() : Exits the program on ctrl C
void handler(int sig)
{
	printf("\nEXITING...\n");
	run = 0;
}

// error(): sends an error with the message passed during function call.
// 	    exits the program.
void error(char *msg)
{
	perror(msg);
	exit(0);
}

//socketCreate(): Creates the socket and returns the value of socket descriptor
int socketCreate()
{
	int server_socket;
	printf("\nCreate the socket\n");

	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	return server_socket;
}

//Used by the server side to bind
int bindCreatedSocket(int server_socket, int portno)
{
	int n = -1;
	struct sockaddr_in serv_addr;

	bzero((char*)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	n = bind(server_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	return n;
}

//function to run as server if only 2 arguments are passed
int  runAsServer(int portno)
{
	int sockfd, newsockfd, clilen;
	char buffer[256];
	char newBuffer[1024];

	struct sockaddr_in serv_addr, cli_addr;
	int n;

	sockfd = socketCreate();
	if(sockfd < 0)
	{
		error("Error in Creating Socket");
	}

	if(bindCreatedSocket(sockfd,portno) < 0)
	{
		error("Error Bind Failed");
	}
	
	
	listen(sockfd, 10);
	clilen = sizeof(cli_addr);

	int data = initializeGameSetUp();

	if(data != 0)
	{
		newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);

		if(newsockfd < 0)
		{
			error("Error in Accepting");
		}

		bzero(buffer, 256);

		while(1)
		{

			n = recv(newsockfd, buffer, 255, MSG_DONTWAIT);	

			if(n > 0)
			{
				break;
			}
	
		}

		printf("Here is the message: %s\n", buffer);

		newBuffer[0] = data;		//When the ball reaches the end of screen, data has the X coordinate of the ball
		newBuffer[1] = 7;		// When the ball reaches the end of scree, the Y coordinate is 7
		newBuffer[2] = scorePlayer;	//score of the player

		n = send(newsockfd, newBuffer, strlen(newBuffer), 0);
		if(n < 0)
		{
			error("Error writing to socket");
		}

		printf("\nMessage Sent %s\n", newBuffer);
	}
	close(newsockfd);
	close(sockfd);

	return 1;
}

//function to run as client if 3 arguments are passed
int runAsClient(int portno, char* IP_addr)
{
	int sockfd, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[MAX];

	sockfd = socketCreate();

	if(sockfd<0)
	{
		error("Error opening socket");
	}

	server = gethostbyname(IP_addr);
	
	if(server == NULL)
	{
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
	}

	bzero((char*)&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;

	bcopy((char*)server->h_addr,(char*)&serv_addr.sin_addr.s_addr,server->h_length);
	serv_addr.sin_port = htons(portno);

	if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0)
	{
		error("ERROR connecting");
	}

	int data = initializeGameSetUp();

	if(data != 0)
	{
		//The two statements below is to check whether the client side runs using the GET request from browser.
	//	strcpy(buffer, "GET/\r\n\r\n");
	//	strcat(buffer, IP_addr);
	
		buffer[0] = data;		// When the ball reaches the end of screen the value stored in data is the X coordinate
		buffer[1] = 7; 			// When the ball reaches the end of screen the Y coordinate is 7
		buffer[3] = scorePlayer;	// Sends the score

		n = send(sockfd,buffer,strlen(buffer),0);
		if(n<0)
		{
			error("ERROR writing to socket");
		}

		bzero(buffer,4096);

		while(1)
		{
			n = recv(sockfd,buffer ,sizeof(buffer),MSG_DONTWAIT);
		
			if(n > 0)
			{
				break;
			}
		}

		printf("%s\n", buffer);
	}

	close(sockfd);

	return 1;
}

//Keeps account of event.
//Event occurs when the joystick is pressed
void callbackFn(unsigned int code)
{
	switch(code)
	{
		case KEY_RIGHT:
			runJoyStick = 1;
			break;
		case KEY_LEFT:
			runJoyStick = -1;
			break;
		default:
			runJoyStick = 0;
			break;
	}
}

//Sets up Initial Position of the ball
void initGame(gamestate_t *game)
{
	game->ballx = game->bally = game->ballxprev = game->ballyprev = 1;
}


void drawPaddle(sense_fb_bitmap_t *screen, int startingPaddleIndex, uint16_t color)
{
	clearBitmap(fb->bitmap, getColor(0,0,0));

	int i = startingPaddleIndex;
	int count = 0; // Number of paddle dots 

	while(i < 8 && i >= 0 && count < 3)
	{
		setPixel(screen, i, 0, color);
		i++;
		count++;
	}
}

void drawBall(sense_fb_bitmap_t *screen, gamestate_t *state, uint16_t color)
{     
	setPixel(screen, state->ballxprev, state->ballyprev,0);
	setPixel(screen, state->ballx, state->bally, color);
}

//Function to generate random movement of the ball
int generate_random()
{
	return (rand() % 3) - 3;
}

//Detects collision with paddle
int collision(int paddle_xpos, int ball_xpos)
{
 
	if(ball_xpos >= paddle_xpos && ball_xpos <= (paddle_xpos + 2))
	{
		scorePlayer++;
		return 1;
	}
	else
	{
		return 0;
	}
}

int  moveBall(sense_fb_bitmap_t *screen, gamestate_t *state, int paddle_x)
{

	int x = state->ballx;
	int y = state->bally;

	state->ballxprev = state->ballx;
	state->ballyprev = state->bally;

	if(x >= 7)
	{
		ballXVel = -1;
	}

	if(x <= 0)
	{
		ballXVel = 1;
	}

	if(y == 7)
	{
		clearBitmap(fb->bitmap, 0); // Clears the screen when the ball leaves the player's side
		return x;		    
	}

	else if(y <= 1 && collision(paddle_x,x))
	{
		ballYVel = 1;
		state->ballx+=generate_random();
	}

	else if(y <= 1 && (collision(paddle_x,x) == 0))
	{
		run = 0; // If the player misses the ball, the game ends
	}

	state->ballx += ballXVel;
	state->bally += ballYVel;

	usleep(200000); // Used to reduce the speed of the ball

	return 0;
}

int  movePaddle(sense_fb_bitmap_t *screen, int direction)
{
	int paddleX = 0;
	if(direction == 1)
	{
		paddleX += SPEED * direction;	
	}
	else if(direction == -1)
	{
		paddleX += SPEED * direction;
	}
	return paddleX;
}


int main(int argc, char* argv[])
{
	int portno, i;
	int paddleSpeed = 0;

	if(argc == 2)
	{
		printf("\n Initializing as Server...");
		portno = atoi(argv[1]);
		serverRunning = runAsServer(portno);
	}
	
	else if(argc == 3)
	{
		printf("\n Initializing as client...");
		portno = atoi(argv[1]);
		clientRunning = runAsClient(portno, argv[2]);
	}
	else
	{
		error("\nWrong number of arguments provided...");
	}
	return 0;
}

int initializeGameSetUp()
{

	pi_i2c_t *device;
	coordinate_t data;

	gamestate_t game;
	initGame(&game);

	signal(SIGINT, handler);

	fb = getFBDevice();
	pi_joystick_t* joystick = getJoystickDevice();

	if(!fb)
	{
		return 0;
	}

	clearBitmap(fb->bitmap, 0);

	drawPaddle(fb->bitmap,startingPaddleIndex, getColor(0,0,255));
	drawBall(fb->bitmap, &game,getColor(255,0,0));

	device = geti2cDevice();

	if(device)
	{
		while(run)
		{
			usleep(2000);

			while(run)
			{
				drawBall(fb->bitmap, &game, getColor(255,0,0));

				int data = moveBall(fb->bitmap, &game, startingPaddleIndex);
				if(data != 0)
				{
					return data;
				}

				pollJoystick(joystick, callbackFn, 0);
				if(runJoyStick == 1 || runJoyStick == -1)
				{
					printf("\nJOYSTICK RUNNING");
					startingPaddleIndex +=  movePaddle(fb->bitmap, runJoyStick);
					printf("\n Paddle Starting X-Pos: %d", startingPaddleIndex);
					drawPaddle(fb->bitmap, startingPaddleIndex, getColor(0,0,255));
					runJoyStick = 0;
				}

			}
		}
		freei2cDevice(device);
	}
	
	clearBitmap(fb->bitmap, 0);
	freeFrameBuffer(fb);
	freeJoystick(joystick);
	return 0;

}
