#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sense/sense.h>
#include <linux/input.h>

#define SPEED 1 // SPEED: the speed of the paddle with which it moves when joystick is pressed either left or right. 

int run = 1; // Keeps account of whether the program is running
int score = 0;
int runJoyStick = 0; // Keeps account of whether joystick is running and also the direction (left or right) in which it is pressed 

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
	game->ballx = game->bally = game->ballxprev = game->ballyprev = 0;
}

void drawPaddle(sense_fb_bitmap_t *screen, int startingPaddleIndex, uint16_t color)
{
	clearBitmap(fb->bitmap, getColor(0,0,0));

	int i = startingPaddleIndex;
	int count = 0; // Number of paddle dots 

	while(i < 8 && i >= 0 && count < 3)
	{
		setPixel(screen, i, 0, color);
		setPixel(screen, i, 1, color);
		i++;
		count++;
	}
}

void drawBall(sense_fb_bitmap_t *screen, gamestate_t *state, uint16_t color)
{     
	setPixel(screen, state->ballxprev, state->ballyprev, 0);
	setPixel(screen, state->ballx, state->bally, color);
}

void moveBall(gamestate_t *state){
	int x = state->ballx;
	int y = state->bally;
	state->ballxprev = state->ballx;
	state->ballyprev = state->bally;
	if(y>=8){
		ballYVel=-1;
	}	
	else if(y<=0){
		ballYVel = 1;
	}
	state->bally += ballYVel;

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
	int cnt = 0, i, startingPaddleIndex = 0, paddleSpeed = 0;

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
	drawBall(fb->bitmap, &game, getColor(255,0,0));
	device = geti2cDevice();
	if(device)
	{
		configureAccelGyro(device);
		while(run)
		{
			usleep(2000);

			while(run && getGyroPosition(device, &data))
			{
				drawBall(fb->bitmap, &game, getColor(255,0,0));
				moveBall(&game);

				pollJoystick(joystick, callbackFn, 0);
				if(runJoyStick == 1 || runJoyStick == -1)
				{
					printf("\nJOYSTICK RUNNING");
					startingPaddleIndex +=  movePaddle(fb->bitmap, runJoyStick);
					printf("\n Paddle Speed: %d", startingPaddleIndex);
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
