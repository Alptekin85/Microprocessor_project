#include <stm32f031x6.h>
#include "display.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define SCREEN_HEIGHT 128
#define SCREEN_WIDTH 64
#define SCORE_DIVISOR 5
#define xLimitR 110
#define xLimitL 10
#define yLimitD 140
#define yLimitU 16
#define PLAYER_BASE_SPEED 3
#define SPEED_PENALTY 2


//stuctures
struct player_Data
{
    int position[2]; //player positions (x,y) where position[0] = x and position[1] = y
    int sprite_ID; //player's sprite id
    int sprite_Inversion[2]; //player inversion status (x,y) where sprite_Inversion[0] = xinversion and sprite_Inversion[1] = yinversion 
};

struct ghost_data
{
	int position[2]; //ghost positions (x,y) where position[0] = x and position[1] = y
    int sprite_ID; //ghost's sprite id
    int sprite_Inversion[2]; //ghost inversion status (x,y) where sprite_Inversion[0] = xinversion and sprite_Inversion[1] = yinversion 
};


struct ball_Data
{
	int position[2]; //ball positions (x,y) where position[0] = x and position[1] = y
};



void initClock(void);
void initSysTick(void);
void SysTick_Handler(void);
void delay(volatile uint32_t dly);
void setupIO();
int isInside(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint16_t px, uint16_t py);
void enablePullUp(GPIO_TypeDef *Port, uint32_t BitNumber);
void pinMode(GPIO_TypeDef *Port, uint32_t BitNumber, uint32_t Mode);
void pauseMenu(struct player_Data *player1, struct ball_Data ball1, int *playerLives, int *ballExist, int *currentScore);
void mainMenu(struct player_Data *player1, struct ball_Data ball1, int *playerLives, int *ballExist, int *currentScore);
void playerHud(int userScore, int playerLives);
void ballSpawner(int *randX, int *randY, int *spawnTimer, int *ballExist, struct ball_Data *ball1);
int ballCollision(int x, int y, int x1, int y1, int currentScore, int *ballExist);
void reDraw(struct player_Data *player1, struct ball_Data ball1, int *wait_Input);
void restartGame(int *playerLives, int *ballExist, int *currentScore, int *wait_Input, struct player_Data *player1);
void ghostChase(struct player_Data player1, struct ghost_data *ghost1, int currentScore, int *playerLives);

volatile uint32_t milliseconds;






const uint16_t deco1[]=
{
0,0,0,0,4,4,4,4,4,0,0,0,0,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,65415,65415,65415,248,65415,0,0,0,0,0,0,0,65415,65415,65415,65415,65415,8068,0,0,0,0,0,0,65415,65415,65415,4096,4096,0,0,0,0,0,0,0,0,65415,65415,65415,0,0,0,0,0,0,0,0,0,7936,7936,7936,0,0,0,0,0,0,0,0,7936,7936,65535,7936,0,0,0,0,0,0,0,0,7936,7936,65535,7936,7936,7936,7936,0,0,0,0,0,7936,7936,65535,65535,65535,65535,7936,0,0,0,0,0,7936,7936,7936,7936,7936,7936,7936,0,0,0,0,0,7936,7936,7936,7936,0,0,0,0,0,0,0,0,0,7936,65535,7936,0,0,0,0,0,0,0,0,0,7936,65535,7936,0,0,0,0,0,0,0,0,0,7936,65535,7936,0,0,0,0,0,0,0,0,0,7940,7940,7940,7940,0,0,0,
};

const uint16_t deco2[]= 
{
0,0,0,0,0,4,4,4,4,4,0,0,0,0,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,65415,65415,65415,248,65415,0,0,0,0,0,0,0,65415,65415,65415,65415,65415,8068,0,0,0,0,0,0,65415,65415,65415,4096,4096,0,0,0,0,0,0,0,0,65415,65415,65415,0,0,0,0,0,0,0,0,7936,7936,7936,0,0,0,0,0,0,0,0,7936,7936,65535,7936,0,0,0,0,0,0,0,0,7936,7936,65535,7936,7936,7936,7936,0,0,0,0,0,7936,7936,65535,65535,65535,65535,7936,0,0,0,0,0,7936,7936,7936,7936,7936,7936,7936,0,0,0,0,0,7936,7936,7936,7936,0,0,0,0,0,0,0,0,0,40224,7936,65535,7936,0,0,0,0,0,0,0,40224,40224,7936,65535,7936,0,0,0,0,0,0,65315,40224,40224,7936,65535,40224,0,0,0,0,0,0,0,65315,0,65315,65315,65315,65315,0,0,
};

const uint16_t deco3[]= 
{
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,4,0,0,0,0,0,0,0,0,0,0,4,4,0,0,0,0,0,0,0,0,0,4,4,4,4,0,0,0,0,0,0,0,4,4,4,4,4,4,0,0,0,0,7936,7936,4,4,4,4,4,4,7936,7936,0,0,65535,65535,4,4,4,4,4,4,65535,65535,0,0,7936,7936,4,4,4,4,4,4,7936,7936,0,0,0,0,0,4,4,4,4,0,0,0,0,0,0,0,0,0,24327,24327,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

const uint16_t dg1[]=
{
	0,0,16142,16142,16142,16142,16142,16142,16142,16142,0,0,0,0,0,16142,16142,16142,16142,16142,16142,0,0,0,0,0,16142,16142,16142,16142,16142,16142,16142,16142,0,0,0,0,16142,16142,16142,1994,1994,16142,16142,16142,0,0,0,0,16142,16142,16142,1994,16142,1994,16142,16142,0,0,0,0,16142,16142,16142,1994,16142,1994,16142,16142,0,0,0,0,16142,16142,16142,1994,16142,1994,16142,16142,0,0,0,0,16142,16142,16142,1994,1994,16142,16142,16142,0,0,0,0,16142,16142,16142,16142,16142,16142,16142,16142,0,0,0,0,16142,16142,16142,1994,1994,1994,16142,16142,0,0,0,0,16142,16142,16142,1994,16142,16142,16142,16142,0,0,0,0,16142,16142,16142,1994,16142,16142,16142,16142,0,0,0,0,16142,16142,16142,1994,16142,1994,16142,16142,0,0,0,0,16142,16142,16142,1994,1994,1994,16142,16142,0,0,0,0,0,16142,16142,16142,16142,16142,16142,0,0,0,0,0,0,16142,16142,16142,16142,16142,16142,0,0,0,
};

const uint16_t pacmanLR[]=
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,58112,65287,65287,65287,65287,58112,0,0,0,0,0,64262,65287,65287,65287,0,65287,65287,64262,0,0,0,61187,65287,65287,65287,65287,0,61187,65287,65287,61187,0,0,61187,65287,65287,65287,65287,65287,65287,65287,65287,61187,0,57600,65287,65287,65287,65287,65287,65287,65287,65287,0,0,0,57600,65287,65287,65287,65287,65287,64262,0,0,0,0,0,57600,65287,65287,65287,61187,0,0,0,0,0,0,0,57600,65287,65287,65287,65287,65287,64262,0,0,0,0,0,57600,65287,65287,65287,65287,65287,65287,65287,65287,0,0,0,0,61187,65287,65287,65287,65287,65287,65287,65287,65287,61187,0,0,61187,65287,65287,65287,65287,65287,65287,65287,65287,61187,0,0,0,64262,65287,65287,65287,65287,65287,65287,64262,0,0,0,0,0,58112,65287,65287,65287,65287,58112,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

const uint16_t pacmanLR2[]=
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,24327,24327,24327,24327,0,0,0,0,0,0,0,24327,24327,24327,0,24327,24327,0,0,0,0,0,24327,24327,24327,24327,0,0,24327,24327,0,0,0,0,24327,24327,24327,24327,24327,24327,24327,24327,0,0,0,0,24327,24327,24327,24327,24327,24327,24327,24327,0,0,0,0,24327,24327,24327,24327,24327,24327,24327,24327,0,0,0,0,0,24327,24327,24327,24327,24327,24327,0,0,0,0,0,0,0,24327,24327,24327,24327,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};
const uint16_t pacmanUD[]=
{
	0,0,0,0,0,0,57600,57600,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65287,65287,65287,65287,0,0,0,0,0,0,65287,65287,65287,65287,65287,65287,65287,64262,0,0,0,58112,65287,65287,65287,24327,65287,65287,65287,65287,58112,0,0,65287,65287,65287,65287,24327,65287,65287,65287,65287,65287,0,0,65287,65287,65287,65287,24327,65287,65287,65287,65287,65287,0,0,65287,65287,65287,65287,0,65287,65287,65287,65287,65287,0,0,65287,65287,65287,65287,0,24327,65287,0,0,65287,0,0,65287,65287,65287,24327,0,24327,65287,0,65287,65287,0,0,65287,65287,65287,0,0,0,65287,24327,65287,0,0,0,0,65287,65287,0,0,0,65287,65287,65287,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

const uint16_t ball01[]=
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2378,65535,65535,65535,65535,0,0,0,0,0,0,0,65535,65535,65535,65535,65535,65535,0,0,0,0,0,65535,65535,65535,65535,65535,65535,65535,65535,0,0,0,0,65535,65535,65535,65535,65535,65535,65535,65535,0,0,0,0,65535,65535,65535,65535,65535,65535,65535,65535,0,0,0,0,0,65535,65535,65535,65535,65535,65535,0,0,0,0,0,0,0,65535,65535,65535,65535,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

const uint16_t bomb[]=
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,24327,40224,0,0,0,0,0,0,0,0,0,0,0,65535,0,0,0,0,0,0,0,0,0,0,0,0,65535,0,0,0,0,0,0,0,0,0,0,0,65535,0,0,0,0,0,0,0,0,0,0,61307,61307,61307,0,0,0,0,0,0,0,0,61307,61307,61307,61307,61307,0,0,0,0,0,0,0,61307,61307,61307,61307,61307,0,0,0,0,0,0,0,61307,61307,61307,61307,61307,0,0,0,0,0,0,0,0,61307,61307,61307,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};


const uint16_t maze[] = {
    // 128x160 display - using 8x8 blocks for simplicity
    // 1 = wall (blue), 0 = path (black)
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // Top wall
    1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,
    1,0,1,1,1,1,0,1,1,0,1,1,1,1,0,1,
    1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1,
    1,0,1,0,1,1,1,1,1,1,1,1,0,1,0,1,
    1,0,0,0,1,0,0,0,0,0,0,1,0,1,0,1,
    1,1,1,0,1,0,1,1,1,1,0,1,0,1,0,1,
    1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1,
    1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,
    1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1  // Bottom wall
};







int main()
{
	// Draw initial scene
    fillRectangle(0,0,128,160,0); // Clear screen
    
	struct player_Data player1;
	struct ghost_data ghost1;
	struct ball_Data ball1;

	
	int playerLives = 3;
	int playerSpriteID = 0;
	int ballExist = 0;
	int spawnTimer = 0;
	int currentScore = 0;
	int hinverted = 0;
	int vinverted = 0;
	int toggle = 0;
	int hmoved = 0;
	int vmoved = 0;
	int randX; 
	int randY; 
	int i;

	uint16_t x = 50;
	uint16_t y = 50;
	uint16_t oldx = x;
	uint16_t oldy = y;
	player1.position[0] = x;
	player1.position[1] = y;
	ghost1.position[0] = 20;
	ghost1.position[1] = 20;                                                                                                                                                                                                 
	initClock();
	initSysTick();
	setupIO();
	srand(time(NULL));
	

	putImage(50,20,12,16,bomb,0,0);
	fillCircle(30, 40, 5, RGBToWord(0,0,255));
	putImage(50,50,12,16,pacmanLR,0,0);
	mainMenu(& player1, ball1, &playerLives, &ballExist, &currentScore);



		
	




	while(1)
	{

		/*
		// Draw maze (each block is 8x8 pixels)
		for(int row = 0; row < 19; row++) {
			for(int col = 0; col < 16; col++) {
				if(maze[row * 16 + col]) {
					fillRectangle(col*8, row*8, 8, 8, RGBToWord(0,0,255)); // Blue walls
				}
			}
		}
		*/
		


		
		
		spawnTimer++;
		

		
		
		playerHud(currentScore, playerLives);
		ballSpawner(&randX , &randY, &spawnTimer, &ballExist, & ball1);
		

		if (ballCollision(x, y, randX, randY, currentScore, &ballExist) == 1)
		{
			currentScore = currentScore + 1;

		}
		

		hmoved = vmoved = 0;
		//hinverted = vinverted = 0;

		if ((GPIOB->IDR & (1 << 4))==0) // right pressed
		{					
			if (x < xLimitR)
			{
				x = x + PLAYER_BASE_SPEED + (currentScore / SCORE_DIVISOR);
				hmoved = 2;
				hinverted=0;
			}						
		}

		if ((GPIOB->IDR & (1 << 5))==0) // left pressed
		{			
			
			if (x > xLimitL)
			{
				x = x - PLAYER_BASE_SPEED - (currentScore / SCORE_DIVISOR);
				hmoved = 2;
				hinverted=2;
			}			
		}

		if ( (GPIOA->IDR & (1 << 11)) == 0) // down pressed
		{
			if (y < yLimitD)
			{
				y = y + PLAYER_BASE_SPEED + (currentScore / SCORE_DIVISOR);			
				vmoved = 2;
				vinverted = 0;
			}
		}

		if ( (GPIOA->IDR & (1 << 8)) == 0) // up pressed
		{			
			if (y > yLimitU)
			{
				y = y - PLAYER_BASE_SPEED - (currentScore / SCORE_DIVISOR);
				vmoved = 2;
				vinverted = 2;
			}
		}



		if ((vmoved) || (hmoved))
		{
			// only redraw if there has been some movement (reduces flicker)
			fillRectangle(oldx,oldy,12,16,0);
			oldx = x;
			oldy = y;	
			
			if (hmoved)
			{
				if (toggle)
				{
				
					putImage(x,y,12,16,pacmanLR,hinverted,0);
					playerSpriteID = 0;
				}	
				else
				{

					putImage(x,y,12,16,pacmanLR2,hinverted,0);
					playerSpriteID = 1;
				}
				toggle = toggle ^ 1;
			}

			else
			{
				
				putImage(x,y,12,16,pacmanUD,0,vinverted);
				playerSpriteID = 2;
			}



			// Now check for an overlap by checking to see if ANY of the 4 corners of deco are within the target area
			if (isInside(50,20,12,16,x,y) || isInside(50,20,12,16,x+12,y) || isInside(50,20,12,16,x,y+16) || isInside(50,20,12,16,x+12,y+16) )
			{
				//printTextX2("GLUG!", 10, 20, RGBToWord(0xff,0xff,0), 0);
				//currentScore++;
				for (i = 0; i < playerLives; i++)
				{
					//putImage(80 + 5 * i, 3, 12, 16, pacmanLR,0,0);
					fillRectangle(85 + 15 * i, 2, 9, 9, RGBToWord(0,0,0));
				}

				playerLives--;

				if (playerLives <= 0)
				{
					printTextX2("Paused", 30, 30, RGBToWord(0,0,255), 0);
				}

				delay(500);
			}


		}		


		//set player1's structure values
		player1.position[0] = x;
		player1.position[1] = y;
		player1.sprite_ID = playerSpriteID;
		player1.sprite_Inversion[0] = hinverted;
		player1.sprite_Inversion[1] = vinverted;

	
		ghostChase(player1, & ghost1, currentScore, &playerLives);
		pauseMenu(& player1, ball1, &playerLives, &ballExist, &currentScore);




		delay(50);
	}
	return 0;
}



//draws the ball when spawnTimer is satisfied
void ballSpawner(int *randX, int *randY,int *spawnTimer, int *ballExist, struct ball_Data *ball1)
{


	if ((*spawnTimer >= 50) && (*ballExist == 0) )
	{	
		//randomise spawn coordinates
		*randX = (rand() % xLimitR - 24 + 1) + xLimitL + 18;
		*randY = (rand() % yLimitD - 32 + 1) + yLimitU + 24;

		//set ball1's positions
		ball1 -> position[0] = *randX;
		ball1 -> position[1] = *randY;

		//draw the ball
		putImage(*randX,*randY,12,16,ball01,0,0);
		
		//reset spawn timer, set ball's existence to true
		*spawnTimer = 0;
		*ballExist = 1;

		//a small delay so the score isn't incremented more than once.
		delay(150);

	}


	
}



//checks if player has made contact with a ball, increment the score if collision occured
int ballCollision(int x, int y, int x1, int y1, int currentScore, int *ballExist)
{
	




	//check if a collison occured
	if ((isInside(x1,y1,12,16,x,y) || isInside(x1,y1,12,16,x+12,y) || isInside(x1,y1,12,16,x,y+16) || isInside(x1,y1,12,16,x+12,y+16)) && *ballExist == 1)
	{	

		//collision occured, set existence to false
		*ballExist = 0;

		//delete the image of the ball
		fillRectangle(x1, y1, 12, 16, RGBToWord(0,0,0));
		
		//return true so player score is incremented.
		return 1;

	}

	//no collision occured
	return 0;
}


//halt function. forces the game into a while loop so long as the player is in: main menu, paused, dead, passed the stage.
void pauseMenu(struct player_Data *player1, struct ball_Data ball1, int *playerLives, int *ballExist, int *currentScore)
{	

	int menu_Index = 1;
	int index_Move = 0;
	int toggle = 1;
	int	wait_Input = 1;

	//check if pause button is pressed
	if ((GPIOB->IDR & (1 << 1)) == 0 ) 
	{					
		
		//print pause menu
		fillRectangle(0, 0, 128, 256, RGBToWord(0,0,0));
		printTextX2("Paused", 30, 30, RGBToWord(0,0,255), 0);
		printText("Resume", 30, 60, RGBToWord(0,0,255), 0);
		printText("Main menu", 30, 80, RGBToWord(0,0,255), 0);
		printText("Reset", 30, 100, RGBToWord(0,0,255), 0);
		
		//delay so the button does not exit while instantly
		delay(250);

		while (wait_Input == 1)
		{

			//go to top if no more options remain at the bottom
			if (menu_Index > 3)
			{
				menu_Index = 1;
			}
			
			//go to bottom if no more options remain at the top
			else if (menu_Index < 1)
			{
				menu_Index = 3;
			}
			

			//toggle between sprites for menu index
			if (toggle)
			{
			
				putImage(15, 35 + 20 * menu_Index, 12, 16, pacmanLR , 0, 0);
			}	
			else
			{

				putImage(15, 35 + 20 * menu_Index, 12, 16, pacmanLR2, 0, 0);
			}
			toggle = toggle ^ 1;


			//menu index controller
			if ( (GPIOA->IDR & (1 << 11)) == 0) // down pressed
			{
				menu_Index++;
				index_Move = 1;
				delay(50);
			}
	
			if ( (GPIOA->IDR & (1 << 8)) == 0) // up pressed
			{			
				menu_Index--;
				index_Move = 1;
				delay(50);
			}

			//only clear indicator if it has moved
			if (index_Move == 1)
			{
				fillRectangle(15, 55, 12, 111, RGBToWord(0,0,0));
				index_Move = 0;
			}
		
			//resume selected, redraw objects
			if (menu_Index == 1)
			{
				reDraw(& *player1, ball1, &wait_Input);
			}

			if (menu_Index == 2)
			{
				if ((GPIOB->IDR & (1 << 1)) == 0 ) 
				{
					mainMenu(& *player1, ball1, &*playerLives, &*ballExist, &*currentScore);
					delay(50);
					wait_Input = 0;
				}
			}
			

			//reset selected, clear playfield
			if (menu_Index == 3)
			{
				restartGame(&*playerLives, &*ballExist, &*currentScore, &wait_Input, & *player1);
			}
			
			



			delay(100);
			
		}
		
	}

}


void mainMenu(struct player_Data *player1, struct ball_Data ball1, int *playerLives, int *ballExist, int *currentScore)
{
	
	int menu_Index = 1;
	int index_Move = 0;
	int toggle = 1;
	int	wait_Input = 1;

					
		
	//print main menu
	fillRectangle(0, 0, 128, 256, RGBToWord(0,0,0));
	printTextX2("Mainmenu", 30, 30, RGBToWord(0,0,255), 0);
	printText("Single player", 30, 60, RGBToWord(0,0,255), 0);
	printText("Multi player", 30, 80, RGBToWord(0,0,255), 0);

		
	//delay so the button does not exit while instantly
	delay(250);

	while (wait_Input == 1)
	{

		//go to top if no more options remain at the bottom
		if (menu_Index > 2)
		{
			menu_Index = 1;
		}
		
		//go to bottom if no more options remain at the top
		else if (menu_Index < 1)
		{
			menu_Index = 2;
		}
		

		//toggle between sprites for menu index
		if (toggle)
		{
		
			putImage(15, 35 + 20 * menu_Index, 12, 16, pacmanLR , 0, 0);
		}	
		else
		{

			putImage(15, 35 + 20 * menu_Index, 12, 16, pacmanLR2, 0, 0);
		}
		toggle = toggle ^ 1;


		//menu index controller
		if ( (GPIOA->IDR & (1 << 11)) == 0) // down pressed
		{
			menu_Index++;
			index_Move = 1;
			delay(50);
		}

		if ( (GPIOA->IDR & (1 << 8)) == 0) // up pressed
		{			
			menu_Index--;
			index_Move = 1;
			delay(50);
		}

		//only clear indicator if it has moved
		if (index_Move == 1)
		{
			fillRectangle(15, 55, 12, 111, RGBToWord(0,0,0));
			index_Move = 0;
		}
	
		//single player selected, set playfield
		if (menu_Index == 1)
		{
			restartGame(&*playerLives, &*ballExist, &*currentScore, &wait_Input, & *player1);
		}

		//multi player selected, set playfield
		if (menu_Index == 2)
		{
			restartGame(&*playerLives, &*ballExist, &*currentScore, &wait_Input, & *player1);
		}
		
		



		delay(100);
		
	}
		
	
}


void reDraw(struct player_Data *player1, struct ball_Data ball1, int *wait_Input)
{

	if ((GPIOB->IDR & (1 << 1))==0) // select
			{	
				//reset screen
				fillRectangle(0, 0, 128, 256, RGBToWord(0,0,0));

					
				//switch used to redraw players sprite if "Paused" text corrupted the image
				switch (player1 -> sprite_ID)
				{
					case 0:
					{	
						
						putImage(player1 -> position[0],player1 -> position[1],12,16,pacmanLR,player1 -> sprite_Inversion[0],0); //open mouth sprite, horizontal
						break;
					}

					case 1:
					{	

						putImage(player1 -> position[0],player1 -> position[1],12,16,pacmanLR2,player1 -> sprite_Inversion[0],0); //closed mouth sprite, horizontal
						break;
					}

					default:
					{
						putImage(player1 -> position[0],player1 -> position[1],12,16,pacmanUD,0,player1 -> sprite_Inversion[1]); //open mouth sprite, vertical
						break;
					}
				}

				//draw the ball again
				putImage(ball1.position[0],ball1.position[1],12,16,ball01,0,0);

				*wait_Input = 0;
				delay(50);
		
				

			}
}


void restartGame(int *playerLives, int *ballExist, int *currentScore, int *wait_Input, struct player_Data *player1)
{
	if ((GPIOB->IDR & (1 << 1))==0) // select
	{
		
		*playerLives = 3;
		*ballExist = 0;
		*currentScore = 0;
		*wait_Input = 0;
		delay(50);

		fillRectangle(0, 0, 128, 256, RGBToWord(0,0,0));
		putImage(player1 -> position[0],player1 -> position[1],12,16,pacmanLR,0,0);
		delay(50);
		/*
		player1 -> position[0] = 50;
		player1 -> position[1] = 50;


		

		fillRectangle(0, 0, 128, 256, RGBToWord(0,0,0));
		putImage(player1 -> position[0],player1 -> position[1],12,16,pacmanLR,0,0);
		delay(50);
		*/
	}
}



//players hud, shows score and remaining lives
void playerHud(int userScore, int playerLives)
{	
	int i;

	//draw the frame of the hud
	drawRectangle(0, 0, 127, 12, RGBToWord(0,255,0));
	
	//players score
	printText("Score:",3, 3, RGBToWord(0,255,0), RGBToWord(0,0,0));
	printNumber(userScore, 44, 3, RGBToWord(0,255,0), RGBToWord(0,0,0));

	//players lives
	for (i = 0; i < playerLives; i++)
	{
		//putImage(80 + 5 * i, 3, 12, 16, pacmanLR,0,0);
		fillRectangle(85 + 15 * i, 2, 9, 9, RGBToWord(0,255,0));
	}
	



}


void ghostChase(struct player_Data player1, struct ghost_data *ghost1, int currentScore, int *playerLives)
{
	int toggle = 0;
	int i;
	//check if ghost is at the same position with the player
	if (player1.position[0] != ghost1 -> position[0])
	{	
		//check if the ghost is left to the player
		if (player1.position[0] - ghost1 -> position[0] > 0)
		{
				//clear previous sprite
				fillRectangle(ghost1 -> position[0],ghost1 -> position[1], 12, 16, RGBToWord(0,0,0));
				
				//make the move
				ghost1 -> position[0] = ghost1 -> position[0] + PLAYER_BASE_SPEED + (currentScore / SCORE_DIVISOR) - SPEED_PENALTY;
				
				//draw the ghost
				if (toggle)
				{
				
					putImage(ghost1 -> position[0],ghost1 -> position[1],12,16,bomb,0,0);

				}	
				else
				{

					putImage(ghost1 -> position[0],ghost1 -> position[1],12,16,bomb,0,0);

				}
				toggle = toggle ^ 1;
		}
		//check if the ghost is right to the player
		else if (player1.position[0] - ghost1 -> position[0] < 0)
		{
				//Clears previous position
				fillRectangle(ghost1 -> position[0],ghost1 -> position[1], 12, 16, RGBToWord(0,0,0));
				
				//Set new position for the ghost
				ghost1 -> position[0] = ghost1 -> position[0] - PLAYER_BASE_SPEED - (currentScore / SCORE_DIVISOR) + SPEED_PENALTY;

				//Draw ghost
				if (toggle)
				{
				
					putImage(ghost1 -> position[0],ghost1 -> position[1],12,16,bomb,0,0);

				}	
				else
				{

					putImage(ghost1 -> position[0],ghost1 -> position[1],12,16,bomb,0,0);

				}
				toggle = toggle ^ 1;
		}
	}

	//Checks if the ghost is on the same co ordinates as the player
	if (player1.position[1] != ghost1 -> position[1])
	{
		//Moves towards the player (below)
		if (player1.position[1] - ghost1 -> position[1] > 0)
		{
				
				fillRectangle(ghost1 -> position[0],ghost1 -> position[1], 12, 16, RGBToWord(0,0,0));
				
				ghost1 -> position[1] = ghost1 -> position[1] + PLAYER_BASE_SPEED + (currentScore / SCORE_DIVISOR) - SPEED_PENALTY;

				if (toggle)
				{
				
					putImage(ghost1 -> position[0],ghost1 -> position[1],12,16,bomb,0,0);

				}	
				else
				{

					putImage(ghost1 -> position[0],ghost1 -> position[1],12,16,bomb,0,0);

				}
				toggle = toggle ^ 1;
		}

		//Moves towards the player (above)
		else if (player1.position[1] - ghost1 -> position[1] < 0)
		{
				fillRectangle(ghost1 -> position[0],ghost1 -> position[1], 12, 16, RGBToWord(0,0,0));
				
				ghost1 -> position[1] = ghost1 -> position[1] - PLAYER_BASE_SPEED - (currentScore / SCORE_DIVISOR) + SPEED_PENALTY;

				if (toggle)
				{
				
					putImage(ghost1 -> position[0],ghost1 -> position[1],12,16,bomb,0,0);

				}	
				else
				{

					putImage(ghost1 -> position[0],ghost1 -> position[1],12,16,bomb,0,0);

				}
				toggle = toggle ^ 1;
		}
	}	 
		
	if (isInside(ghost1 -> position[0],ghost1 -> position[1],12,16,player1.position[0],player1.position[1]) || isInside(ghost1 -> position[0],ghost1 -> position[1],12,16,player1.position[0]+12,player1.position[1]) || isInside(ghost1 -> position[0],ghost1 -> position[1],12,16,player1.position[0],player1.position[1]+16) || isInside(ghost1 -> position[0],ghost1 -> position[1],12,16,player1.position[0]+12,player1.position[1]+16) )
	{
		
		if (*playerLives > 0)
		{
			
		
			delay(500);
			for (i = 0; i < *playerLives; i++)
			{
				//putImage(80 + 5 * i, 3, 12, 16, pacmanLR,0,0);
				fillRectangle(85 + 15 * i, 2, 9, 9, RGBToWord(0,0,0));
			}
			
			(*playerLives)--;
			delay(500);
		}
		
		
		//printNumber(*playerLives,10,20,65535,0);
	}
			
		
		
	
	
}



void initSysTick(void)
{
	SysTick->LOAD = 48000;
	SysTick->CTRL = 7;
	SysTick->VAL = 10;
	__asm(" cpsie i "); // enable interrupts
}




void SysTick_Handler(void)
{
	milliseconds++;
}




void initClock(void)
{
// This is potentially a dangerous function as it could
// result in a system with an invalid clock signal - result: a stuck system
        // Set the PLL up
        // First ensure PLL is disabled
        RCC->CR &= ~(1u<<24);
        while( (RCC->CR & (1 <<25))); // wait for PLL ready to be cleared
        
// Warning here: if system clock is greater than 24MHz then wait-state(s) need to be
// inserted into Flash memory interface
				
        FLASH->ACR |= (1 << 0);
        FLASH->ACR &=~((1u << 2) | (1u<<1));
        // Turn on FLASH prefetch buffer

        FLASH->ACR |= (1 << 4);
        // set PLL multiplier to 12 (yielding 48MHz)
        RCC->CFGR &= ~((1u<<21) | (1u<<20) | (1u<<19) | (1u<<18));
        RCC->CFGR |= ((1<<21) | (1<<19) ); 

        // Need to limit ADC clock to below 14MHz so will change ADC prescaler to 4
        RCC->CFGR |= (1<<14);

        // and turn the PLL back on again
        RCC->CR |= (1<<24);        
        // set PLL as system clock source 
        RCC->CFGR |= (1<<1);
}




void delay(volatile uint32_t dly)
{
	uint32_t end_time = dly + milliseconds;
	while(milliseconds != end_time)
		__asm(" wfi "); // sleep
}





void enablePullUp(GPIO_TypeDef *Port, uint32_t BitNumber)
{
	Port->PUPDR = Port->PUPDR &~(3u << BitNumber*2); // clear pull-up resistor bits
	Port->PUPDR = Port->PUPDR | (1u << BitNumber*2); // set pull-up bit
}




void pinMode(GPIO_TypeDef *Port, uint32_t BitNumber, uint32_t Mode)
{
	/*
	*/
	uint32_t mode_value = Port->MODER;
	Mode = Mode << (2 * BitNumber);
	mode_value = mode_value & ~(3u << (BitNumber * 2));
	mode_value = mode_value | Mode;
	Port->MODER = mode_value;
}




//isInside function checks if given positions are overlapping (colliding) if so return 1 otherwise 0.
int isInside(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint16_t px, uint16_t py)
{
	// checks to see if point px,py is within the rectange defined by x,y,w,h
	// where (x1,y1) are top-right of the sprite/image and (x2,y2) is the bottom-left
	// px and py are mostly the player's sprite
	uint16_t x2,y2;
	x2 = x1+w;
	y2 = y1+h;
	int rvalue = 0;
	if ( (px >= x1) && (px <= x2)) //check collision on x axis
	{
		// ok, x constraint met
		if ( (py >= y1) && (py <= y2))//check collison on y axis
			//collison confirmed, return 1
			rvalue = 1;
	}
	return rvalue;
}







void setupIO()
{
	RCC->AHBENR |= (1 << 18) + (1 << 17); // enable Ports A and B
	display_begin();
	pinMode(GPIOB,4,0);
	pinMode(GPIOB,5,0);
	pinMode(GPIOB,1,0);
	pinMode(GPIOB,7,0);
	pinMode(GPIOA,8,0);
	pinMode(GPIOA,11,0);
	enablePullUp(GPIOB,4);
	enablePullUp(GPIOB,5);
	enablePullUp(GPIOB,1);
	enablePullUp(GPIOB,7);
	enablePullUp(GPIOA,11);
	enablePullUp(GPIOA,8);

}
