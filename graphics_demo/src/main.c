#include <stm32f031x6.h>
#include "display.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "serial.h"
#include "nvm.h"
#include "sound.h"
#include "musical_notes.h"

#define SCREEN_HEIGHT 128
#define SCREEN_WIDTH 64
#define SCORE_DIVISOR 5
#define XLIMITR 110
#define XLIMITL 10
#define YLIMITD 140
#define YLIMITU 16
#define PLAYER_BASE_SPEED 3
#define SPEED_PENALTY 2
#define SCORE_AMOUNT 8


//stuctures
struct player_Data
{
    uint16_t position[2]; //player positions (x,y) where position[0] = x and position[1] = y
    uint16_t sprite_ID; //player's sprite id
    uint16_t sprite_Inversion[2]; //player inversion status (x,y) where sprite_Inversion[0] = xinversion and sprite_Inversion[1] = yinversion 
	uint16_t moved[2]; // players movement checks, where moved [0] = xmoved and moved[1] = ymoved (replaces initial xmoved and ymoved to move player movement to another function)
};

struct ghost_data
{
	uint16_t position[2]; //ghost positions (x,y) where position[0] = x and position[1] = y
    uint16_t sprite_ID; //ghost's sprite id
    uint16_t sprite_Inversion[2]; //ghost inversion status (x,y) where sprite_Inversion[0] = xinversion and sprite_Inversion[1] = yinversion 
};


struct ball_Data
{
	uint16_t position[2]; //ball positions (x,y) where position[0] = x and position[1] = y
};



void initClock(void);
void initSysTick(void);
void SysTick_Handler(void);
void delay(volatile uint32_t dly);
void setupIO();
uint16_t isInside(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint16_t px, uint16_t py);
void enablePullUp(GPIO_TypeDef *Port, uint32_t BitNumber);
void pinMode(GPIO_TypeDef *Port, uint32_t BitNumber, uint32_t Mode);
void pauseMenu(struct player_Data *player1, struct ball_Data ball1, uint16_t *playerLives, uint16_t *ballExist, uint16_t *currentScore);
void mainMenu(struct player_Data *player1, struct ball_Data ball1, uint16_t *playerLives, uint16_t *ballExist, uint16_t *currentScore);
void playerHud(uint16_t userScore, uint16_t playerLives);
void ballSpawner(uint16_t *randX, uint16_t *randY, uint16_t *spawnTimer, uint16_t *ballExist, struct ball_Data *ball1);
uint16_t ballCollision(struct player_Data player1, uint16_t x1, uint16_t y1, uint16_t currentScore, uint16_t *ballExist);
void reDraw(struct player_Data *player1, struct ball_Data ball1, uint16_t *wait_Input);
void restartGame(uint16_t *playerLives, uint16_t *ballExist, uint16_t *currentScore, uint16_t *wait_Input, struct player_Data *player1);
void ghostChase(struct player_Data player1, struct ghost_data *ghost1, uint16_t currentScore, uint16_t *playerLives);
void playerMovement(struct player_Data *player1, uint16_t *oldx, uint16_t *oldy,uint16_t *toggle, uint16_t currentScore);
void deathScreen(struct player_Data *player1, struct ball_Data ball1, uint16_t *playerLives, uint16_t *ballExist, uint16_t *currentScore);
void victoryScreen(struct player_Data *player1, struct ball_Data ball1, uint16_t *playerLives, uint16_t *ballExist, uint16_t *currentScore);
void scoreBoard(struct player_Data *player1, struct ball_Data ball1, uint16_t *playerLives, uint16_t *ballExist, uint16_t *currentScore);
void playBackgroundTune(uint32_t * notes, uint32_t * times, uint32_t count, uint32_t repeat);
uint16_t menuIndexer(uint16_t itemCount);
uint16_t regularMode(uint16_t currentScore);
void endlessMode(uint16_t *currentScore);
void sortScores(void);

volatile uint32_t milliseconds;

// select note patterns
uint32_t my_tune_notes[]={E4, D4, C4, E4, F4, E4, D4, C4, G4, F4, E4, D4, C4, E4, G4, F4, A4, G4, F4, E4, C4, E4, D4, C4}; //sound  played
uint32_t my_tune_times[]={800, 800, 800, 800, 800, 800, 800, 800, 1000, 800, 800, 800, 800, 800, 800, 1000, 1000, 800, 800, 800, 800, 800, 800, 1000}; //duration played


//audio event handlers, when to play what sound
uint32_t * background_tune_notes=0;
uint32_t * background_tune_times;
uint32_t background_note_count;
uint32_t background_tune_repeat;






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

const uint16_t ghost[]=
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40224,40224,40224,40224,0,0,0,0,0,0,0,40224,40224,40224,40224,40224,40224,0,0,0,0,0,0,40224,65535,40224,40224,65535,40224,0,0,0,0,0,40224,0,65535,40224,0,65535,40224,40224,0,0,0,0,40224,0,65535,40224,0,65535,40224,40224,0,0,0,0,40224,40224,40224,40224,40224,40224,40224,40224,0,0,0,0,40224,40224,40224,40224,40224,40224,40224,40224,0,0,0,0,40224,40224,40224,40224,40224,40224,40224,40224,0,0,0,0,40224,40224,0,40224,40224,0,40224,40224,0,0,0,0,40224,0,0,0,40224,0,0,40224,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};






uint32_t high_scores[SCORE_AMOUNT];
uint32_t sorted_scores[SCORE_AMOUNT];
uint32_t lowestScore = 0;
uint16_t restartGhost = 0;
uint16_t ghostTimer = 0;
uint16_t gameSelect = 0;
uint16_t menu_Index = 1;
uint16_t index_Move = 0;
uint16_t toggle = 1;
char ch;




int main()
{
	// Draw initial scene
    fillRectangle(0,0,128,160,0); // Clear screen
    
	struct player_Data player1;
	struct ghost_data ghost1;
	struct ball_Data ball1;

	
	//int32_t count = 0; //used to debug serial io
	uint16_t playerLives = 3;
	uint16_t playerSpriteID = 0;
	uint16_t ballExist = 0;
	uint16_t spawnTimer = 0;
	uint16_t currentScore = 0;
	uint16_t hinverted = 0;
	uint16_t vinverted = 0;
	uint16_t toggle = 0;
	uint16_t i;
	uint16_t randX; 
	uint16_t randY; 


	player1.position[0] = 50;
	player1.position[1] = 50;
	uint16_t oldx = player1.position[0];
	uint16_t oldy = player1.position[1];
	player1.sprite_Inversion[0] = 0;
	player1.sprite_Inversion[1] = 0;
	player1.moved[0] = 0;
	player1.moved[1] = 0;
	ghost1.position[0] = 20;
	ghost1.position[1] = 20;                                                                                                                                                                                                 
	initClock();
	initSysTick();
	setupIO();
	srand(time(NULL));
	initSerial();
	initSound();
	playBackgroundTune(my_tune_notes,my_tune_times,3,1);
	

	//used to debug writing to memory

	
	//read our high score values
	readSector(0x7f00,high_scores,8*sizeof(uint32_t));
	eputs("stored scores:");
	eputs("\r\n");
	for (uint16_t index=0;index < 8; index++)
	{
		printDecimal(high_scores[index]);
		eputs("\r\n");
	}
	

	//fill the list with scores
	
	/*
	high_scores[0]=2;
	high_scores[1]=7;
	high_scores[2]=11;
	high_scores[3]=9;
	high_scores[4]=3;
	high_scores[5]=21;
	high_scores[6]=10;
	high_scores[7]=9;
	*/

	
	//writeSector(0x7f00,high_scores,8*sizeof(uint32_t));
	


	//fill our second list to be sorted.
	for (i = 0; i < SCORE_AMOUNT; i++)
	{
		sorted_scores[i] = high_scores[i];
	}


	//eraseSector(0x7f00); //WARNING: ONLY ERASE SECTOR IF YOU MUST AND THIS CANNOT BE IN A PROGRAMMING LOOP

	sortScores();
	
	eputs("sorted scores:");
	eputs("\r\n");
	for (uint16_t index=0;index < 8; index++)
	{
		printDecimal(sorted_scores[index]);
		eputs("\r\n");
	}
	
	
	mainMenu(& player1, ball1, &playerLives, &ballExist, &currentScore);
	
	//serial begins
	eputs("starting");

		
	




	while(1)
	{


		
		//serial debug
		/*
		printDecimal(count++);
		eputs("\r\n");


		for (i = 0; i < 3; i++)
		{
			ch = egetchar();
			eputchar(ch);

			if (ch == 'a')
			{
				eputs("testing");
			}
			
		}
		

		eputs("\r\n");
		*/
				
		

		spawnTimer++;
		
		//call functions appropiate to the current gamemode
		switch (gameSelect)
		{
		case 0:
			//gamemode is regular
			if (regularMode(currentScore) == 1)
			{
				victoryScreen(& player1, ball1, &playerLives, &ballExist, &currentScore);
			}
			break;
		//gamemode is endless
		default:
			break;
		}
		
		//draw hud
		playerHud(currentScore, playerLives);

		//spawn ball if requirements met
		ballSpawner(&randX , &randY, &spawnTimer, &ballExist, & ball1);
		
		//check if player ate a ball.
		if (ballCollision(player1, randX, randY, currentScore, &ballExist) == 1)
		{
			currentScore = currentScore + 1;

		}
		
		playerMovement(& player1, &oldx, &oldy, &toggle, currentScore);

		//check if player is dead
		if (playerLives == 0)
		{
			deathScreen(& player1, ball1, &playerLives, &ballExist, &currentScore);
		}
		
		//update player1's structure incase there is a need to redraw or restart
		player1.sprite_ID = playerSpriteID;
		player1.sprite_Inversion[0] = hinverted;
		player1.sprite_Inversion[1] = vinverted;

		//ghost moves towards the player.
		ghostChase(player1, & ghost1, currentScore, &playerLives);
		
		//check if pause button (select) is pressed
		pauseMenu(& player1, ball1, &playerLives, &ballExist, &currentScore);




		delay(50);
	}
	return 0;
}


//sort scores in descending order 
void sortScores(void)
{
	uint16_t i;
	uint16_t j;
	uint16_t temp = 0;
	
	//sort scores using bubble sort
	for (i = 0; i < SCORE_AMOUNT - 1; i++)
	{
		for (j = 0; j < SCORE_AMOUNT - i - 1; j++)
		{	
			//switch numbers if current number is lower than next number.
			if (sorted_scores[j] < sorted_scores[j+1])
			{
				temp = sorted_scores[j];
				sorted_scores[j] = sorted_scores[j+1];
				sorted_scores[j+1] = temp;
			}
			
		}
		
	}

	//set lowest score to the last item, compared with user's score on endless mode post-death
	lowestScore = sorted_scores[SCORE_AMOUNT - 1];
	

}


//movement of the player is moved to a separate function to make main simpler 
void playerMovement(struct player_Data *player1, uint16_t *oldx, uint16_t *oldy,uint16_t *toggle, uint16_t currentScore)
{
	
	player1 -> moved[0] = 0;
	player1 -> moved[1] = 0;

	if ((GPIOB->IDR & (1 << 4))==0) // right pressed
	{	
		//check if player is going out of bounds				
		if (player1 -> position[0] < XLIMITR)
		{
			player1 -> position[0] = player1 -> position[0] + PLAYER_BASE_SPEED + (currentScore / SCORE_DIVISOR);
			player1 -> moved[0] = 2;
			player1 -> sprite_Inversion[0] = 0;
		}						
	}

	if ((GPIOB->IDR & (1 << 5))==0) // left pressed
	{			
		//check if player is going out of bounds
		if (player1 -> position[0] > XLIMITL)
		{
			player1 -> position[0] = player1 -> position[0] - PLAYER_BASE_SPEED - (currentScore / SCORE_DIVISOR);
			player1 -> moved[0] = 2;
			player1 -> sprite_Inversion[0] = 2;
		}			
	}

	if ( (GPIOA->IDR & (1 << 11)) == 0) // down pressed
	{
		//check if player is going out of bounds
		if (player1 -> position[1] < YLIMITD)
		{
			player1 -> position[1] = player1 -> position[1] + PLAYER_BASE_SPEED + (currentScore / SCORE_DIVISOR);			
			player1 -> moved[1] = 2;
			player1 -> sprite_Inversion[1] = 0;
		}
	}

	if ( (GPIOA->IDR & (1 << 8)) == 0) // up pressed
	{	
		//check if player is going out of bounds		
		if (player1 -> position[1] > YLIMITU)
		{
			player1 -> position[1] = player1 -> position[1] - PLAYER_BASE_SPEED - (currentScore / SCORE_DIVISOR);
			player1 -> moved[1] = 2;
			player1 -> sprite_Inversion[1] = 2;
		}
	}



	if ((player1 -> moved[0]) || (player1 -> moved[1]))
	{
		// only redraw if there has been some movement (reduces flicker)
		fillRectangle(*oldx,*oldy,12,16,0);
		*oldx = player1 -> position[0];
		*oldy = player1 -> position[1];	
		
		if (player1 -> moved[0])
		{
			if (*toggle)
			{
			
				putImage(player1 -> position[0],player1 -> position[1],12,16,pacmanLR,player1->sprite_Inversion[0],0);
				player1->sprite_ID = 0;
			}	
			else
			{

				putImage(player1 -> position[0],player1 -> position[1],12,16,pacmanLR2,player1->sprite_Inversion[0],0);
				player1->sprite_ID = 1;
			}
			*toggle = *toggle ^ 1;
		}

		else
		{
			
			putImage(player1 -> position[0],player1 -> position[1],12,16,pacmanUD,0,player1->sprite_Inversion[1]);
			player1->sprite_ID = 2;
		}
	}

}

//draws the ball when spawnTimer is satisfied
void ballSpawner(uint16_t *randX, uint16_t *randY,uint16_t *spawnTimer, uint16_t *ballExist, struct ball_Data *ball1)
{


	if ((*spawnTimer >= 50) && (*ballExist == 0) )
	{	
		//randomise spawn coordinates
		*randX = (rand() % XLIMITR - 24 + 1) + XLIMITL + 18;
		*randY = (rand() % YLIMITD - 32 + 1) + YLIMITU + 24;

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
uint16_t ballCollision(struct player_Data player1, uint16_t x1, uint16_t y1, uint16_t currentScore, uint16_t *ballExist)
{
	




	//check if a collison occured
	if ((isInside(x1,y1,12,16,player1.position[0],player1.position[1]) || isInside(x1,y1,12,16,player1.position[0]+12,player1.position[1]) || isInside(x1,y1,12,16,player1.position[0],player1.position[1]+16) || isInside(x1,y1,12,16,player1.position[0]+12,player1.position[1]+16)) && *ballExist == 1)
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


//navigates menu components where itemCount is amount of components
uint16_t menuIndexer(uint16_t itemCount)
{

	//go to top if no more options remain at the bottom
	if (menu_Index > itemCount)
	{
		menu_Index = 1;
	}
	
	//go to bottom if no more options remain at the top
	else if (menu_Index < 1)
	{
		menu_Index = itemCount;
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
		fillRectangle(15, 55, 12, 40 * itemCount, RGBToWord(0,0,0));
		index_Move = 0;
	}

	delay(100);
	return menu_Index;

}


//halt function. forces the game into a while loop so long as the player is in: main menu, paused, dead, passed the stage.
void pauseMenu(struct player_Data *player1, struct ball_Data ball1, uint16_t *playerLives, uint16_t *ballExist, uint16_t *currentScore)
{	

	menu_Index = 1;
	index_Move = 0;
	toggle = 1;
	uint16_t	wait_Input = 1;

	//check if pause button is pressed
	if ((GPIOA->IDR & (1 << 12)) == 0 ) 
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

			//resume selected, redraw objects
			if (menuIndexer(3) == 1 )
			{		
							
				reDraw(& *player1, ball1, &wait_Input);
				fillRectangle(15, 55, 12, 16, RGBToWord(0,0,0));
			}

			//call main menu
			if (menuIndexer(3) == 2)
			{
				if ((GPIOA->IDR & (1 << 12)) == 0 ) 
				{
					mainMenu(& *player1, ball1, &*playerLives, &*ballExist, &*currentScore);
					delay(50);
					wait_Input = 0;
				}
			}
			

			//reset selected, clear playfield
			if (menuIndexer(3) == 3)
			{
				restartGame(&*playerLives, &*ballExist, &*currentScore, &wait_Input, & *player1);
			}
			
			



			delay(100);
			
		}
		
	}

}


//main menu is the hub to select gamemodes and display the scoreboard
void mainMenu(struct player_Data *player1, struct ball_Data ball1, uint16_t *playerLives, uint16_t *ballExist, uint16_t *currentScore)
{
	
	menu_Index = 1;
	index_Move = 0;
	toggle = 1;
	uint16_t	wait_Input = 1;

					
		
	//print main menu
	fillRectangle(0, 0, 128, 256, RGBToWord(0,0,0));
	printTextX2("Mainmenu", 30, 30, RGBToWord(0,0,255), 0);
	printText("Regular", 30, 60, RGBToWord(0,0,255), 0);
	printText("Endless", 30, 80, RGBToWord(0,0,255), 0);
	printText("Scores", 30, 100, RGBToWord(0,0,255), 0);

		
	//delay so the button does not exit while instantly
	delay(250);

	while (wait_Input == 1)
	{
	
		//regular selected, set playfield
		if (menuIndexer(3) == 1 && ((GPIOA->IDR & (1 << 12)) == 0 ) )
		{	
			gameSelect = 0;
			wait_Input = 0;
			delay(25);
			restartGame(&*playerLives, &*ballExist, &*currentScore, &wait_Input, & *player1);
		}

		//endless selected, set playfield
		if (menuIndexer(3) == 2 && ((GPIOA->IDR & (1 << 12)) == 0 ) )
		{	
			gameSelect = 1;
			wait_Input = 0;
			delay(25);
			restartGame(&*playerLives, &*ballExist, &*currentScore, &wait_Input, & *player1);
		}
		
		//scoreboard selected, display scores
		if (menuIndexer(3) == 3 && ((GPIOA->IDR & (1 << 12)) == 0 ))
		{	
			wait_Input = 0;
			delay(25);
			scoreBoard(& *player1, ball1, &*playerLives, &*ballExist, &*currentScore);
		}



		delay(100);
		
	}
		
	
}


//displays scores in descending order
void scoreBoard(struct player_Data *player1, struct ball_Data ball1, uint16_t *playerLives, uint16_t *ballExist, uint16_t *currentScore)
{
	
	menu_Index = 1;
	index_Move = 0;
	toggle = 1;
	uint16_t	wait_Input = 1;
	uint16_t i;

					
		
	//print scoreboard
	fillRectangle(0, 0, 128, 256, RGBToWord(0,0,0));
	printTextX2("TOPSCORES", 10, 10, RGBToWord(0,0,255), 0);

	for (i = 0; i < SCORE_AMOUNT; i++)
	{
		printNumber(sorted_scores[i], 50, 40 + i * 12, RGBToWord(0,0,255), RGBToWord(0,0,0));
	}
	

		
	//delay so the button does not exit while instantly
	delay(250);

	while (wait_Input == 1)
	{
	
		//check if pause button is pressed
		if ((GPIOA->IDR & (1 << 12)) == 0 ) 
		{	
			wait_Input = 0;
			mainMenu(& *player1, ball1, &*playerLives, &*ballExist, &*currentScore);
		}
		
		delay(100);
		
	}
		
	
}


//displays death screen. if current score is higher than lowest score ask to save the score
void deathScreen(struct player_Data *player1, struct ball_Data ball1, uint16_t *playerLives, uint16_t *ballExist, uint16_t *currentScore)
{
	menu_Index = 1;
	index_Move = 0;
	toggle = 1;
	uint16_t	wait_Input = 1;

	//call endless mode to save new highscore
	if (gameSelect == 1)
	{
		endlessMode(& *currentScore);
	}
	
	
		
	//print death screen
	fillRectangle(0, 0, 128, 256, RGBToWord(0,0,0));
	printTextX2("YOU", 30, 10, RGBToWord(0,0,255), 0);
	printTextX2("DIED!", 30, 30, RGBToWord(0,0,255), 0);
	printText("restart", 30, 60, RGBToWord(0,0,255), 0);
	printText("main menu", 30, 80, RGBToWord(0,0,255), 0);

		
	//delay so the button does not exit while instantly
	delay(250);

	while (wait_Input == 1)
	{
	
		//restart game selected, set playfield
		if (menuIndexer(2) == 1)
		{
			restartGame(&*playerLives, &*ballExist, &*currentScore, &wait_Input, & *player1);
		}

		//main menu selected, navigate to main menu
		if ((menuIndexer(2) == 2) && ((GPIOA->IDR & (1 << 12))==0))
		{	
			*playerLives = 3;
			mainMenu(& *player1, ball1, &*playerLives, &*ballExist, &*currentScore);
			delay(50);
			wait_Input = 0;
		}
		
		



		delay(100);
		
	}
	
}


//display victory screen. 
void victoryScreen(struct player_Data *player1, struct ball_Data ball1, uint16_t *playerLives, uint16_t *ballExist, uint16_t *currentScore)
{
	menu_Index = 1;
	index_Move = 0;
	toggle = 1;
	uint16_t	wait_Input = 1;

					
		
	//print victory screen
	fillRectangle(0, 0, 128, 256, RGBToWord(0,0,0));
	printTextX2("YOU", 30, 10, RGBToWord(0,0,255), 0);
	printTextX2("WIN!", 30, 30, RGBToWord(0,0,255), 0);
	printText("restart", 30, 60, RGBToWord(0,0,255), 0);
	printText("main menu", 30, 80, RGBToWord(0,0,255), 0);

		
	//delay so the button does not exit while instantly
	delay(250);

	while (wait_Input == 1)
	{

		//restart game selected, set playfield
		if (menuIndexer(2) == 1)
		{
			restartGame(&*playerLives, &*ballExist, &*currentScore, &wait_Input, & *player1);
		}

		//main menu selected, navigate to main menu
		if ((menuIndexer(2) == 2) && ((GPIOA->IDR & (1 << 12))==0))
		{	
			*playerLives = 3;
			mainMenu(& *player1, ball1, &*playerLives, &*ballExist, &*currentScore);
			delay(50);
			wait_Input = 0;
		}
		
		



		delay(100);
		
	}
	
}


//default mode, if current score is higher than a given number the game ends.
uint16_t regularMode(uint16_t currentScore)
{
	if (currentScore > 5)
	{
		return 1;
	}
	
	return 0;
}


//used to save scores if the gamemode is endless
void endlessMode(uint16_t *currentScore)
{
	if (*currentScore > lowestScore)
	{
		
		//print highscore menu
		fillRectangle(0, 0, 128, 256, RGBToWord(0,0,0));
		printTextX2("NEW", 30, 30, RGBToWord(0,0,255), 0);
		printTextX2("HIGH", 30, 50, RGBToWord(0,0,255), 0);
		printTextX2("SCORE!", 30, 70, RGBToWord(0,0,255), 0);

		//serial messages
		eputs("\r\n");
		eputs("congratulations!");
		eputs("\r\n");
		eputs("you have a new highscore, would you like to save it?");
		eputs("\r\n");
		eputs("type 'y' for yes.");
		eputs("\r\n");

		//wait  for input
		ch = egetchar();
		eputchar(ch);

		//if user chose to save
		if (ch == 'y')
		{
			eputs("\r\n");
			eputs("saving...");

			//set current score to lowest existing score
			delay(50);
			sorted_scores[SCORE_AMOUNT - 1] = *currentScore;

			//sort the scores again
			delay(50);
			sortScores();

			//delete current scores
			delay(50);
			eraseSector(0x7f00);

			//set new high scores
			readSector(0x7f00,high_scores,8*sizeof(uint32_t));		
			delay(50);
			for (uint16_t i = 0; i < SCORE_AMOUNT; i++)
			{
				high_scores[i] = sorted_scores[i];
			}
			
			//write new high scores to flash memory
			delay(50);
			writeSector(0x7f00,high_scores,8*sizeof(uint32_t));
			
			delay(50);
			eputs("\r\n");
			eputs("score saved!");
			eputs("\r\n");
			eputs("new scores are:");
			eputs("\r\n");

			//print new high scores
			readSector(0x7f00,high_scores,8*sizeof(uint32_t));
			for (uint16_t index=0;index < 8; index++)
			{
				printDecimal(high_scores[index]);
				eputs("\r\n");
			}

		}
		
		eputs("\r\n");
		
	}
	
}


//redraws objects into the playing field. used to resume the game
void reDraw(struct player_Data *player1, struct ball_Data ball1, uint16_t *wait_Input)
{

	if ((GPIOA->IDR & (1 << 12))==0) // select
			{	
				//reset screen
				delay(50);
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


//resetting function. erases score data with current lives and resets positions to original
void restartGame(uint16_t *playerLives, uint16_t *ballExist, uint16_t *currentScore, uint16_t *wait_Input, struct player_Data *player1)
{
	
	if ((GPIOA->IDR & (1 << 12))==0) // select button pressed
	{
		
		//reset player stats and ghost reset flag
		*playerLives = 3;
		*ballExist = 0;
		*currentScore = 0;
		*wait_Input = 0;
		restartGhost = 1;
		ghostTimer = 0;
		player1 -> position[0] = 15;
		player1 -> position[1] = 55;

		delay(50);

		//reset screen
		fillRectangle(0, 0, 128, 256, RGBToWord(0,0,0));
		putImage(player1 -> position[0],player1 -> position[1],12,16,pacmanLR,0,0);
		delay(50);

	}
}



//players hud, shows score and remaining lives
void playerHud(uint16_t userScore, uint16_t playerLives)
{	
	uint16_t i;

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


//function that makes the ghost chase the player. diagonal movement is allowed.
void ghostChase(struct player_Data player1, struct ghost_data *ghost1, uint16_t currentScore, uint16_t *playerLives)
{
	uint16_t toggle = 0;
	uint16_t i;

	//did the ghost recently hit the player?
	if (ghostTimer > 0)
	{
		ghostTimer--;
	}
	
	
	//reset ghost if flag is true
	if (restartGhost == 1)
	{
		ghost1 -> position[0] = 20;
		ghost1 -> position[1] = 20;
		restartGhost = 0;
	}
	

	//check if ghost is at the same position with the player
	if (player1.position[0] != ghost1 -> position[0] && ghostTimer <= 0)
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
				
					putImage(ghost1 -> position[0],ghost1 -> position[1],12,16,ghost,0,0);

				}	
				else
				{

					putImage(ghost1 -> position[0],ghost1 -> position[1],12,16,ghost,0,0);

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
				
					putImage(ghost1 -> position[0],ghost1 -> position[1],12,16,ghost,0,0);

				}	
				else
				{

					putImage(ghost1 -> position[0],ghost1 -> position[1],12,16,ghost,0,0);

				}
				toggle = toggle ^ 1;
		}
	}

	//Checks if the ghost is on the same co ordinates as the player
	if (player1.position[1] != ghost1 -> position[1] && ghostTimer <= 0) 
	{
		//Moves towards the player (below)
		if (player1.position[1] - ghost1 -> position[1] > 0)
		{
				
				fillRectangle(ghost1 -> position[0],ghost1 -> position[1], 12, 16, RGBToWord(0,0,0));
				
				ghost1 -> position[1] = ghost1 -> position[1] + PLAYER_BASE_SPEED + (currentScore / SCORE_DIVISOR) - SPEED_PENALTY;

				if (toggle)
				{
				
					putImage(ghost1 -> position[0],ghost1 -> position[1],12,16,ghost,0,0);

				}	
				else
				{

					putImage(ghost1 -> position[0],ghost1 -> position[1],12,16,ghost,0,0);

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
				
					putImage(ghost1 -> position[0],ghost1 -> position[1],12,16,ghost,0,0);

				}	
				else
				{

					putImage(ghost1 -> position[0],ghost1 -> position[1],12,16,ghost,0,0);

				}
				toggle = toggle ^ 1;
		}
	}	 
	
	
	//check if ghost has collided with the player
	if (isInside(ghost1 -> position[0],ghost1 -> position[1],12,16,player1.position[0],player1.position[1]) || isInside(ghost1 -> position[0],ghost1 -> position[1],12,16,player1.position[0]+12,player1.position[1]) || isInside(ghost1 -> position[0],ghost1 -> position[1],12,16,player1.position[0],player1.position[1]+16) || isInside(ghost1 -> position[0],ghost1 -> position[1],12,16,player1.position[0]+12,player1.position[1]+16) )
	{
		
		//check if player is alive
		if (*playerLives > 0 && ghostTimer <= 0)
		{
			
		
			//erase currnet lives
			for (i = 0; i < *playerLives; i++)
			{
				//putImage(80 + 5 * i, 3, 12, 16, pacmanLR,0,0);
				fillRectangle(85 + 15 * i, 2, 9, 9, RGBToWord(0,0,0));
			}
			
			//player loses a life, ghost receives a cooldown
			(*playerLives)--;
			ghostTimer = 50;
			
		}
		
	}	
	
}

// function to play background theme
void playBackgroundTune(uint32_t * notes, uint32_t * times, uint32_t count, uint32_t repeat)
{
	background_tune_notes=notes;
	background_tune_times=times;
	background_note_count=count;
	background_tune_repeat=repeat; 
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
	static uint16_t index = 0;
	static uint16_t current_note_time=0;
	milliseconds++;
	if (background_tune_notes != 0)
	{	
		//check if current note is valid, when it reaches 0 it has expired
		if (current_note_time == 0)
		{
			//move to the next tune
			index++;
			//if next tune does not exist:
			if (index >= background_note_count)
			{
				//if tune is not on repeat end tune
				if (background_tune_repeat != 0)
				{
					index = 0;
				}

				//if it is on repeat reset index
				else
				{
					background_tune_notes=0;
					playNote(0);
				}
			}
			current_note_time = background_tune_times[index];
			playNote(background_tune_notes[index]);
		}
		else
		{
			//reduce current note's lifetime
			current_note_time--;
		}
	}
	
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
uint16_t isInside(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint16_t px, uint16_t py)
{
	// checks to see if point px,py is within the rectange defined by x,y,w,h
	// where (x1,y1) are top-right of the sprite/image and (x2,y2) is the bottom-left
	// px and py are mostly the player's sprite
	uint16_t x2,y2;
	x2 = x1+w;
	y2 = y1+h;
	uint16_t rvalue = 0;
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
	//pinMode(GPIOB,1,0);
	//pinMode(GPIOB,6,0);
	pinMode(GPIOA,8,0);
	pinMode(GPIOA,11,0);
	pinMode(GPIOA,12,0);
	enablePullUp(GPIOB,4);
	enablePullUp(GPIOB,5);
	//enablePullUp(GPIOB,1);
	//enablePullUp(GPIOB,6);
	enablePullUp(GPIOA,11);
	enablePullUp(GPIOA,12);
	enablePullUp(GPIOA,8);

}
