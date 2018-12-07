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
int timeout = 0;													


/*The variable ticks is used to divide up time for the program.
Different functions can thus run at different paces. The ticks
counter ranges from 0 to 59, providing 60 subdivisions of a
second. Therefore 1 tick = 16.666 msec.
if(!(tick%#)) will be true 60/# times per second, with # between
ticks. Real time = # * 16.666 msec. # should range from 2 to 59
tick is updated by the system clock using the
interval_timer_ISR().*/
int tick = 0;														 


/*These constants are used to define the std starting values
 for the different objects in the game*/

//object player;
const object obj_player[2] = { { 10, 0, 35, 25, 0, 0, hp_max, 0,
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
								 0 },
								 { 10, 0, 0, 0, 0, 0, hp_max, 0,
								 { { {0,0,0,0,0},
									 {0,1,0,1,0},
									 {0,0,0,0,0},
									 {0,1,1,1,0},
									 {0,0,0,0,0} } },
								 { { {RED,  RED,  RED,  RED,  RED },
									 {RED,  BLUE, RED,  BLUE, RED },
									 {RED,  RED,  RED,  RED,  RED },
									 {RED,  BLUE, BLUE, BLUE, RED },
									 {RED,  RED,  RED,  RED,  RED } } },
								 0 } };


const object obj_obst[4] = { { 20, 0, 35, 25, 0, 0, 0, 0,
								{ { {0,0,0,0,0},
									{0,1,1,1,0},
									{0,1,1,1,0},
									{0,1,1,1,0},
									{0,0,0,0,0} } },
								{ { {BLACK, BLACK, BLACK, BLACK, BLACK},
									{BLACK, BLACK, BLACK, BLACK, BLACK},
									{BLACK, BLACK, WHITE, BLACK, BLACK},
									{BLACK, BLACK, BLACK, BLACK, BLACK},
									{BLACK, BLACK, BLACK, BLACK, BLACK} } },
								5 },
							  { 20, 0, 35, 25, 0, 0, 0, 0,
								{ { {0,0,0,0,0},
									{0,1,1,1,0},
									{0,1,1,1,0},
									{0,1,1,1,0},
									{0,0,0,0,0} } },
								{ { {BLACK, BLACK, BLACK, BLACK, BLACK},
									{BLACK, WHITE, WHITE, WHITE, BLACK},
									{BLACK, WHITE, WHITE, WHITE, BLACK},
									{BLACK, WHITE, WHITE, WHITE, BLACK},
									{BLACK, BLACK, BLACK, BLACK, BLACK} } },
								10 },
							  { 20, 0, 0, 0, 0, 0, 0, 0,
								{ { {0,0,1,0,0},
									{0,1,1,1,0},
									{1,1,1,1,1},
								{0,1,1,1,0},
								{0,0,1,0,0} } },
								{ { {BLACK, BLACK, WHITE, BLACK, BLACK},
									{BLACK, WHITE, WHITE, WHITE, BLACK},
									{WHITE, WHITE, WHITE, WHITE, WHITE},
									{BLACK, WHITE, WHITE, WHITE, BLACK},
									{BLACK, BLACK, WHITE, BLACK, BLACK} } },
								15 },
							  { 20, 0, 35, 25, 0, 0, 0, 0,
								{ { {1,1,1,1,1},
									{1,1,1,1,1},
									{1,1,1,1,1},
									{1,1,1,1,1},
									{1,1,1,1,1} } },
								{ { {WHITE, WHITE, WHITE, WHITE, WHITE},
									{WHITE, WHITE, WHITE, WHITE, WHITE},
									{WHITE, WHITE, WHITE, WHITE, WHITE},
									{WHITE, WHITE, WHITE, WHITE, WHITE},
									{WHITE, WHITE, WHITE, WHITE, WHITE} } },
								25 } };

//function prototypes
void delay(volatile int);										//used to create a time delay using a for loop
void interval_timer_ISR();										//this code is executed whenever the interval timer generates an interrupt
void rand_sprite();												//this function generates a random sprite for the player to dodge
void death();


void main()
{
	int SW_val,													//used to store sliding sw values, can be optimized
		button_val,												//used to store soft button values, can be optimized
		timer,													//used to store the interval counter value for loading
		i, j, k,												//i, j, k used for various counters									
		data,													//used to store keyboard output from computer
		collision = 0,											//used to store collision code
		speed = 60,												//used to control the speed at which random sprites spawn
		lives = starting_lives,									//number of player lives
		pt_total;												//used to hold the sum of the players points
		
	bool kill = 0;												//This flag tells the main loop if a player has died

	/*these arrays hold the color values of all news pixels when
	moving the entire screen left, right, up or down.
	Current only hold a single blue pixel as a test case*/
	int new_pixels_lr[RES_Y - GAME_TOP] = { BLACK },
		new_pixels_ud[RES_X] = { BLACK };

	srand(time(0));

	clear_screen(color_back);									//makes the screen the background color
	create_hud(lives, entities[0].points);						//generates the hud at the top of the screen

	data = *(JTAG_UART_ptr);									//Read the JTAG_UART data register
	

	//loads player entity for at starting location
	add_sprite(obj_player[0], obj_player[0].x, obj_player[0].y, 0, 0);
	

	//This segement of code sets up the timer to run
	*(timer_ptr) = 0;											//clears the interval timer status
	timer = SIXTY_FPS;											//sets the new count down value
	*(timer_ptr + 0x2) = (timer & 0xFFFF);						//loads the first half of the counter value
	*(timer_ptr + 0x3) = (timer >> 16) & 0xFFFF;				//loads the second half of the counter value 

	NIOS2_WRITE_STATUS(0);										//disable Nios II interrupts

	//start interval timer looping, enable its interrupts
	*(timer_ptr + 1) = 0x6;										//STOP = 0, START = 1, CONT = 1, ITO = 0

	while (1)
	{
		
		/*this while loop prevents the main loop from advancing
		 until the timer has completed a countdown*/
		while ((*(timer_ptr) & 1) == 0) {}
		
		*(timer_ptr) = 0;										//clears countdown flag
		tick = (tick + 1) % 60;									//increments tick from 0 to 59
		

		if(!(tick % speed)){
			rand_sprite();

			if (speed >30)
			{
				speed--;
			}
			 
		}

		if (!(tick % 20)) {
			move_all_obstacle();
			update_HUD(lives, entities[0].points);
		}

		
		//reads keyboard input 10 times per second
		if (!((tick % 6) - 5))
		{

			data = *(JTAG_UART_ptr);							//Read the JTAG_UART data register

			if (data & 0x00008000)								//check RVALID to see if there is new data
			{
				data = data & 0x000000FF;						//the data is in the least significant byte
				printf("\ndata: %d", data);						//code for bug checking reverse engineering keyboard inputs
			}

			//these ifs handles the keyboard input
			switch (data)
			{
				case W:
					if (entities[0].j > -MAX_VELOCITY) { entities[0].j = entities[0].j - 1; }		//accelerates upward up to max velocity
					break;

				case S:
					if (entities[0].j < MAX_VELOCITY) { entities[0].j = entities[0].j + 1; }		//accelerates downward up to max velocity
					break;

				case A:
					if (entities[0].i > -MAX_VELOCITY) { entities[0].i = entities[0].i - 1; }		//accelerates leftward up to max velocity
					break;

				case D:
					if (entities[0].i < MAX_VELOCITY) { entities[0].i = entities[0].i + 1; }		//accelerates rightward up to max velocity
					break;
			}

			//These checks to see if player avatar is at screen edge
			//left boundary check
			//in case avatar has overshoot corrects velocity
			if ( entities[0].x + entities[0].i < 0) { entities[0].i = -entities[0].x; }
			
			//stops player if still moving left
			if (entities[0].x == 0 && entities[0].i < 0) { entities[0].i = 0; }
			

			//right boundary check
			//in case avatar has overshoot corrects velocity
			if (entities[0].x + entities[0].i > res_x - SPRITE_SIZE) { entities[0].i = res_x - SPRITE_SIZE - entities[0].x; }

			//stops player if still moving right
			if (entities[0].x == res_x - SPRITE_SIZE && entities[0].i > 0) { entities[0].i = 0; }			
			

			//top boundary check
			//in case avatar has overshoot corrects velocity
			if (entities[0].y + entities[0].j < GAME_TOP) { entities[0].j = GAME_TOP - entities[0].y; }

			//stops player if still moving up
			if (entities[0].y == GAME_TOP && entities[0].j < 0) { entities[0].j = 0; }


			//bottom boundary check
			//in case avatar has overshoot corrects velocity
			if (entities[0].y + entities[0].j > res_y - SPRITE_SIZE) { entities[0].j = res_y - SPRITE_SIZE - entities[0].y; }

			//stops player if still moving down
			if (entities[0].y == res_y - SPRITE_SIZE && entities[0].j > 0) { entities[0].j = 0; }
		
		}


		//waits 3 ticks after velocity calculations to move object
		if (!(tick % 6))
		{
			
			collision = collision_chk(entities[0].x, entities[0].y, entities[0].i, entities[0].j);
			
			if ((collision & 0x01) == 1) { lives--; death(lives);  }
			
			//redraws player sprite after input adjusts velocity
			move_sprite(&entities[0].x, &entities[0].y, entities[0].i, entities[0].j, &entities[0].sprite);
		}
			
	}
}


void delay(volatile int time) {
	int count = 0;
	time = time * 1250000;
	while (count < time) {
		count++;
	}
}


/*This function randomally add sprites from the left hand side
with a random amount of left momentium.*/
void rand_sprite()
{	
	add_sprite(obj_obst[rand() % 4], RES_X, (rand() % (RES_Y - GAME_TOP)) + GAME_TOP, -(rand() % 5 + 1), 0);
}


void death(int lives)
{
	int	next_tick = (tick + 30) % 60,
		cycles = 0,
		i;

	int new_pixels_lr[RES_Y - GAME_TOP] = { BLACK },
		new_pixels_ud[RES_X] = { BLACK };


	while (cycles < 4)
	{
		while ((*(timer_ptr) & 1) == 0) {}							//keeps program waitting until timer counts down
		*(timer_ptr) = 0;											//clears countdown flag
		tick = (tick + 1) % 60;										//increments tick from 0 to 59
		
		if (tick == next_tick)
		{
			entities[0].i = 0, entities[0].j = 0, entities[0].sprite = obj_player[(cycles + 1) % 2].sprite;
			draw_sprite(entities[0].x, entities[0].y, &entities[0].sprite);
			next_tick = (tick + 30) % 60;
			cycles++;
		}
	}


	for (i = 0; i < RES_Y; i++) { adv_screen_lu(new_pixels_lr, new_pixels_ud); }
	
	act_entities = 1;
	clear_screen(BLACK);
	create_hud(lives, entities[0].points);
	entities[0].x = obj_player[0].x, entities[0].y = obj_player[0].y, entities[0].sprite = obj_player[0].sprite;
	draw_sprite(entities[0].x, entities[0].y, &entities[0].sprite);
		
}

/*this function is called by the exception handler when the interval
timer generates an interrupt. In previous work it was stored in a
seperate file. Here it is combined int to the main function.

could not get interrupts working, code not used.*/
void interval_timer_ISR()
{
	
	*(timer_ptr) = 0; 											//clear the interrupt
	timeout = 1;												//sets timeout true 
	tick = (tick + 1) % 60;										//increments tick from 0 to 59

	//this text for debugging
	//printf("tick value: %d\ttimeout: %d\n", tick, (int) timeout);

	return;
}