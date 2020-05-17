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
#define MAX 5


//GLOBAL VARIABLES
//run: Keeps account of whether the program is running.
//score: Keeps account of score of each team.
//runJoyStick: Keeps account of whether the joystick is presses and also the direction (left or right) in which it is pressed.
int run = 1; 
int score = 0;
int runJoyStick = 0;
int startingPaddleIndex = 0;


pi_framebuffer_t *fb;

int ballXVel;
int ballYVel;

typedef struct
{
	int ballx;
	int bally;
	int ballxprev;
	int ballyprev;
}gamestate_t;

// handler() : Exits the program on ctrl C

void handler(int sig){
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

int socketCreate()
{
	int server_socket;
	printf("\nCreate the socket\n");

	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	return server_socket;

}

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
	
	listen(sockfd, 5);
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);

	printf("\nNewSocket Created");
	if(newsockfd < 0)
	{
		error("Error in Accepting");
	}

	bzero(buffer, 256);

	n = recv(newsockfd, buffer,255, 0);
	if(n < 0)
	{
		error("Error reading from socket");
	}

	printf("Here is the message: %s\n", buffer);

	n = send(newsockfd, "I got your message", 18, 0);
	if(n < 0)
	{
		error("Error writing to socket");
	}

	printf("\nMessage Sent");
	close(newsockfd);
	close(sockfd);

	return 1;
}

int runAsClient(int portno, char* IP_addr)
{
	int sockfd, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[4096];

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

	strcpy(buffer, "GET /\r\n\r\n");
	strcat(buffer, IP_addr);

	n = send(sockfd,buffer,strlen(buffer),0);
	if(n<0)
	{
		error("ERROR writing to socket");
	}
	bzero(buffer, 4096);

	n = recv(sockfd,buffer ,sizeof(buffer),0);
	if(n<0 || strlen(buffer) > 4096)
	{
		error("ERROR reading from socket");
	}

	printf("%s\n", buffer);
	close(sockfd);

	return 1;
}


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

//function to generate a random number from -3 to 3
int generate_random()
{
	return (rand() % 3) - 3;
}

//Detects if the pixel reaches the paddles' x coordinates, returns 1 if collision detected to be used together with
//y in moveBall
int collision(int paddle_xpos, int ball_xpos)
{
 
	if(ball_xpos >= paddle_xpos && ball_xpos <= (paddle_xpos + 2))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void moveBall(gamestate_t *state, int paddle_x)
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
	if(y >= 7)
	{
		ballYVel=-1;
		state->ballx+=generate_random();
	}	
	else if(y <= 1 && collision(paddle_x,x))
	{
		ballYVel = 1;
		state->ballx+=generate_random();
	}
	else if(y <= 1 && (collision(paddle_x,x) == 0))
	{
		run = 0;
	}

	state->ballx += ballXVel;
	state->bally += ballYVel;

	usleep(200000);
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

//ballPositionSent(): return the ball's position as data when it reaches the end of the screen.
char* ballPositionSent(gamestate_t *state, int paddle_x)
{
	int x = state->ballx;
	int y = state->bally;

	static char data[MAX]; // It is not a good idea to return the address of a local variable outside the function,
       			      //so we would have to define the local variable as static. 

	if(x == 7 && (collision(paddle_x, x) == 1))
	{	
		strcat(data,(char*)x);
		strcat(data,(char*)y);
		return data;
	}
	else
	{
		return NULL;
	}

}

int main(int argc, char* argv[])
{
	int portno, i;
	int paddleSpeed = 0;
	int serverRunning = 0, clientRunning = 0;

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

	if(serverRunning == 1 || clientRunning == 1)
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
			configureAccelGyro(device);
			while(run)
			{
				usleep(2000);

				while(run)
				{
					drawBall(fb->bitmap, &game, getColor(255,0,0));
					moveBall(&game,startingPaddleIndex);

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
		freeJoystick(joystick);}
		return 0;

}
