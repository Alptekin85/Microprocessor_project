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


void initClock(void);
void initSysTick(void);
void SysTick_Handler(void);
void delay(volatile uint32_t dly);
void setupIO();
int isInside(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint16_t px, uint16_t py);
void enablePullUp(GPIO_TypeDef *Port, uint32_t BitNumber);
void pinMode(GPIO_TypeDef *Port, uint32_t BitNumber, uint32_t Mode);
void mainMenu(void);
void playerHud(int);
void ballSpawner(int *spawnTimer);
int ballCollision(int x, int y, int x1, int y1, int currentScore);

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

const uint16_t ball1[]=
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


//global variables
int ballExist = 0;
int randX;
int randY;


int main()
{
	// Draw initial scene
    fillRectangle(0,0,128,160,0); // Clear screen
    
	

	int spawnTimer = 0;
	int currentScore = 0;
	int hinverted = 0;
	int vinverted = 0;
	int toggle = 0;
	int hmoved = 0;
	int vmoved = 0;
	extern int randX; 
	extern int randY; 

	uint16_t x = 50;
	uint16_t y = 50;
	uint16_t oldx = x;
	uint16_t oldy = y;
	initClock();
	initSysTick();
	setupIO();
	srand(time(NULL));
	//putImage(randX,randY,12,16,dg1,0,0);
	putImage(50,20,12,16,bomb,0,0);
	fillCircle(30, 40, 5, RGBToWord(0,0,255));




		
	




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
		

		
		mainMenu();
		playerHud(currentScore);
		ballSpawner(&spawnTimer);
		

		if (ballCollision(x, y, randX, randY, currentScore) == 1)
		{
			currentScore = currentScore + 1;

		}
		

		hmoved = vmoved = 0;
		hinverted = vinverted = 0;

		if ((GPIOB->IDR & (1 << 4))==0) // right pressed
		{					
			if (x < xLimitR)
			{
				x = x + 1 + (currentScore / SCORE_DIVISOR);
				hmoved = 2;
				hinverted=0;
			}						
		}

		if ((GPIOB->IDR & (1 << 5))==0) // left pressed
		{			
			
			if (x > xLimitL)
			{
				x = x - 1 - (currentScore / SCORE_DIVISOR);
				hmoved = 2;
				hinverted=2;
			}			
		}

		if ( (GPIOA->IDR & (1 << 11)) == 0) // down pressed
		{
			if (y < yLimitD)
			{
				y = y + 1 + (currentScore / SCORE_DIVISOR);			
				vmoved = 2;
				vinverted = 0;
			}
		}

		if ( (GPIOA->IDR & (1 << 8)) == 0) // up pressed
		{			
			if (y > yLimitU)
			{
				y = y - 1 - (currentScore / SCORE_DIVISOR);
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
					putImage(x,y,12,16,pacmanLR,hinverted,0);
					
				else
					putImage(x,y,12,16,pacmanLR2,hinverted,0);
				
				toggle = toggle ^ 1;
			}

			else
			{
				putImage(x,y,12,16,pacmanUD,0,vinverted);
			}



			// Now check for an overlap by checking to see if ANY of the 4 corners of deco are within the target area
			if (isInside(20,80,12,16,x,y) || isInside(20,80,12,16,x+12,y) || isInside(20,80,12,16,x,y+16) || isInside(20,80,12,16,x+12,y+16) )
			{
				//printTextX2("GLUG!", 10, 20, RGBToWord(0xff,0xff,0), 0);
				//currentScore++;

			}


		}		
		delay(50);
	}
	return 0;
}



//draws the ball when spawnTimer is satisfied
void ballSpawner(int *spawnTimer)
{

	extern int randX; 
	extern int randY; 
	extern int ballExist;

	if ((*spawnTimer >= 50) && (ballExist == 0) )
	{	
		//randomise spawn coordinates
		randX = (rand() % xLimitR - 12 + 1) + xLimitL;
		randY = (rand() % yLimitD - 16 + 1) + yLimitU + 12;

		//draw the ball
		putImage(randX,randY,12,16,dg1,0,0);
		
		//reset spawn timer, set ball's existence to true
		*spawnTimer = 0;
		ballExist = 1;

		//a small delay so the score isn't incremented more than once.
		delay(50);

	}


	
}



//checks if player has made contact with a ball, increment the score if collision occured
int ballCollision(int x, int y, int x1, int y1, int currentScore)
{
	
	//global variable imported to check if the ball is existent.
	extern int ballExist;


	//check if a collison occured
	if ((isInside(x1,y1,12,16,x,y) || isInside(x1,y1,12,16,x+12,y) || isInside(x1,y1,12,16,x,y+16) || isInside(x1,y1,12,16,x+12,y+16)) && ballExist == 1)
	{	

		//collision occured, set existence to false
		ballExist = 0;

		//delete the image of the ball
		fillRectangle(x1, y1, 12, 16, RGBToWord(0,0,0));
		
		//return true so player score is incremented.
		return 1;

	}

	//no collision occured
	return 0;
}


//halt function. forces the game into a while loop so long as the player is in: main menu, paused, dead, passed the stage.
void mainMenu(void)
{	

	//check if pause button is pressed
	if ((GPIOB->IDR & (1 << 1)) == 0 ) 
	{					
		
		//delay so the button does not exit while instantly
		printTextX2("Paused", 30, 30, RGBToWord(0,0,255), 0);
		delay(500);

		while (1)
		{
			
			if ((GPIOB->IDR & (1 << 1))==0) // right pressed
			{	
				//delay so the button does not force into while instantly
				printTextX2("Paused", 30, 30, RGBToWord(0,0,0), 0);
				delay(500);				
				break;
			}
		}
		
	}

}


//players hud, shows score and remaining lives
void playerHud(int userScore)
{	
	//draw the frame of the hud
	drawRectangle(0, 0, 127, 12, RGBToWord(0,255,0));
	
	//players score
	printText("Score:",3, 3, RGBToWord(0,255,0), RGBToWord(0,0,0));
	printNumber(userScore, 44, 3, RGBToWord(0,255,0), RGBToWord(0,0,0));
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
