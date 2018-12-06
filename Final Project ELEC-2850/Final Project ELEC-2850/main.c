#include "address_map_nios2.h"
#include "nios2_ctrl_reg_macros.h"
#include "game.h"
#include <stdio.h>


//time values
#define ONESEC			0x02FAF080								//1/(50 MHz) x (0x02FAF080) = 1 sec
#define HALFSEC			0x017D7840								//1/(50 MHz) x (0x017D7840) = 500 msec
#define FOURTHSEC		0x00BEBC20								//1/(50 MHz) x (0x00BEBC20) = 250 msec
#define EIGHTHSEC		0x005F5E10								//1/(50 MHz) x (0x005F5E10) = 125 msec
#define SIXTY_FPS		0x000CB736								//1/(50 MHz) x (0x000CB736) ~= 1 / 60 sec = 16.666 msec
#define THIRTY_FPS		0x00196E6B								//1/(50 MHz) x (0x00196E6B) ~= 1 / 30 sec = 33.333 msec
#define TWENTY_FOUR_FPS 0x001FCA05								//1/(50 MHz) x (0x001FCA05) ~= 1 / 24 sec = 41.666 msec



//Keyboard Values
#define W 119													//
#define A 97													//
#define D 100													//
#define S 115													//


//game values
#define MAX_VELOCITY 5											//maxium velocity the object may move in one direction
#define hp_max 25												//Max health value
#define sp_max 75												//Max shield value
#define starting_lives 3										//Default number of lives


//Globals
//hardware addresses
volatile int *LED_ptr = (int *) GREEN_LED_BASE,					//address location for green LEDs 
			 *SW_ptr = (int *) SLIDER_SWITCH_BASE,				//address location for sliding switches
			 *button_ptr = (int *) PUSHBUTTON_BASE,				//address location for push buttons
			 *hex_ptr = (int *) HEX3_HEX0_BASE,					//address location for the hex displays
			 *PS2_ptr = (int *) PS2_PORT_DUAL_BASE,				//address location for keyboard intpu
			 *timer_ptr = (int *) INTERVAL_TIMER_BASE,			//address location for interval timer
			 *JTAG_UART_ptr = (int *)JTAG_UART_BASE;			//address location for UART bus



//Variables
/*This variable is used to limit the program to running no more
than once per tick. Set true by the interval timer ISR, and is 
reset once the main loop has completed.*/
bool timeout;													


/*The variable ticks is used to divide up time for the program.
Different functions can thus run at different paces. The ticks
counter ranges from 0 to 59, providing 60 subdivisions of a
second. Therefore 1 tick = 16.666 msec.
if(!(tick%#)) will be true 60/# times per second, with # between
ticks. Real time = # * 16.666 msec. # should range from 2 to 59
tick is updated by the system clock using the
interval_timer_ISR().*/
int tick;														 


/*These constants are used to define the std starting values
 for the different objects in the game*/

//object player;
const object obj_player = { 10, 0, 0, 0, 0, 0, hp_max, 0,
							{ { {0,0,0,0,0},
								{0,1,0,1,0},
								{0,0,0,0,0},
								{0,1,1,1,0},
								{0,0,0,0,0} } },
							{ { {BLUE, BLUE, BLUE, BLUE, BLUE},
								{BLUE, RED,  BLUE, RED,  BLUE},
								{BLUE, BLUE, BLUE, BLUE, BLUE},
								{BLUE, RED,  RED,  RED,  BLUE},
								{BLUE, BLUE, BLUE, BLUE, BLUE} } },
							0 };

//object obstacle 
const object obj_obs = { 20, 0, 0, 0, 0, 0, 0, 0,
							{ { {1,1,1,1,1},
								{1,1,1,1,1},
								{1,1,1,1,1},
								{1,1,1,1,1},
								{1,1,1,1,1} } },
							{ { {YELLOW,YELLOW,YELLOW,YELLOW,YELLOW},
								{YELLOW,YELLOW,YELLOW,YELLOW,YELLOW},
								{YELLOW,YELLOW,YELLOW,YELLOW,YELLOW},
								{YELLOW,YELLOW,YELLOW,YELLOW,YELLOW},
								{YELLOW,YELLOW,YELLOW,YELLOW,YELLOW} } },
							10 };


void main()
{
	int SW_val,													//used to store sliding sw values, can be optimized
		button_val,												//used to store soft button values, can be optimized
		timer,													//used to store the interval counter value for loading
		i, j, k,												//i, j, k used for various counters									
		data,													//used to store keyboard output from computer
		lives = starting_lives,									//number of player lives
		pt_total;												//used to hold the sum of the players points
		
	bool draw = 1;												//This flag tells the main loop if a redraw of the sprites is required
	
	timeout = false;											//this flag set to false for the first iteration of the program

	/*these arrays hold the color values of all news pixels when
	moving the entire screen left, right, up or down.
	Current only hold a single blue pixel as a test case*/
	int new_pixels_lr[RES_Y - GAME_TOP] = { BLUE },
		new_pixels_ud[RES_X] = { BLUE };

	
	//This segement of code sets up the timer to run
	*(timer_ptr) = 0;											//clears the interval timer status
	timer = SIXTY_FPS;											//sets the new count down value
	*(timer_ptr + 0x2) = (timer & 0xFFFF);						//loads the first half of the counter value
	*(timer_ptr + 0x3) = (timer >> 16) & 0xFFFF;				//loads the second half of the counter value

	NIOS2_WRITE_IENABLE(0x01);									//set interrupt mask bits for levels 0 (IntervalTimer)
	NIOS2_WRITE_STATUS(1);										//enable Nios II interrupts

	//start interval timer looping, enable its interrupts
	*(timer_ptr + 1) = 0x7;										//STOP = 0, START = 1, CONT = 1, ITO = 1 


	clear_screen(color_back);									//makes the screen the background color

	data = *(JTAG_UART_ptr);									//Read the JTAG_UART data register
	
	add_sprite(obj_player, 35, 25, 1, 0, SPRITE_SIZE);			//loads player entity for at starting location

	while (1)
	{
		//this text for debugging
		printf("main loop reset\n");

		while (!timeout) { 
		};
		timeout = false;
		
		//this text for debugging
		printf("\t\ttimeout: %d\n", (int)timeout);
		printf("timeout loop exited\n");
		
		//reads keyboard input 10 times per second
		//if (!(tick % 6))
		//{

		//	printf("1/10 of a second!\n");
		//	//redraws player sprite after input adjusts velocity
		//	move_sprite(&entities[0].x, &entities[0].y, entities[0].i, entities[0].j, SPRITE_SIZE, &entities[0].sprite);

		//	data = *(JTAG_UART_ptr);							//Read the JTAG_UART data register

		//	if (data & 0x00008000)								//check RVALID to see if there is new data
		//	{
		//		data = data & 0x000000FF;						//the data is in the least significant byte
		//		printf("data: %d", data);
		//	}

		//	//these ifs handles the keyboard input
		//	switch (data)
		//	{
		//		case W:
		//		if (entities[0].j > -MAX_VELOCITY) { entities[0].j -= 1; }		//accelerates upward up to max velocity 
		//		break;

		//		case S:
		//		if (entities[0].j > MAX_VELOCITY) { entities[0].j += 1; }		//accelerates downward up to max velocity
		//		break;

		//		case D:
		//		if (entities[0].i > -MAX_VELOCITY) { entities[0].i -= 1; }		//accelerates rightward up to max velocity
		//		break;

		//		case A:
		//		if (entities[0].i > MAX_VELOCITY) { entities[0].i += 1; }		//accelerates leftward up to max velocity
		//		break;			
		//	}


		//	//checks to see if player avatar is at screen edge
		//	//left boundary check
		//	if (entities[0].x <= 0) {
		//		entities[0].x = 0;										//in case avatar has overshoot corrects location
		//		if (entities[0].i < 0) { entities[0].i = 0; }			//stops player if still moving left
		//	}

		//	//right boundary check
		//	else if (entities[0].x >= res_x - SPRITE_SIZE) {
		//		entities[0].x = res_x - SPRITE_SIZE;					//in case avatar has overshoot corrects location
		//		if (entities[0].i > 0) { entities[0].i = 0; }			//stops player if still moving left
		//	}

		//	//top boundary check
		//	if (entities[0].y <= 0) {
		//		entities[0].y = 0;										//in case avatar has overshoot corrects location
		//		if (entities[0].j < 0) { entities[0].j = 0; }			//stops player if still moving up
		//	}

		//	//bottom boundary check
		//	else if (entities[0].y >= res_y - SPRITE_SIZE) {
		//		entities[0].y = res_y - SPRITE_SIZE;					//in case avatar has overshoot corrects location
		//		if (entities[0].j > 0) { entities[0].j = 0; }			//stops player if still moving down
		//	}
		//	
		//}
		
		
			
	}
}



/*this function is called by the exception handler when the interval
timer generates an interrupt. In previous work it was stored in a
seperate file. Here it is combined int to the main function.*/
void interval_timer_ISR()
{
	
	*(timer_ptr) = 0; 											//clear the interrupt
	timeout = true;												//sets timeout true 
	tick = (tick + 1) % 60;										//increments tick from 0 to 59

	//this text for debugging
	printf("tick value: %d\ttimeout: %d\n", tick, (int) timeout);

	return;
}
