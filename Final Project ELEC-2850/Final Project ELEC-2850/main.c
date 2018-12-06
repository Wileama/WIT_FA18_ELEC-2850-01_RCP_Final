#include "address_map_nios2.h"
#include "nios2_ctrl_reg_macros.h"
#include "game.h"
#include <stdio.h>
#include <string.h>


//time values
#define ONESEC 0x02FAF080										//1/(50 MHz) x (0x02FAF080) = 1 sec
#define FOURTHSEC 0x00BEBC20									//1/(50 MHz) x (0x00BEBC20) = 250 msec


//Keyboard Values
#define W 119													//
#define A 97													//
#define D 100													//
#define S 115													//


//game values
#define hp_max 25												//Max health value
#define sp_max 75												//Max shield value
#define starting_lives 3										//Default number of lives


//Globals
//hardware addresses
volatile int *LED_ptr = (int *) GREEN_LED_BASE,					//address location for green LEDs 
			 *SW_ptr = (int *) SLIDER_SWITCH_BASE,				//address location for sliding switches
			 *button_ptr = (int *) PUSHBUTTON_BASE,				//address location for push buttons
			 *hex_ptr = (int *) HEX3_HEX0_BASE,					//address location for the hex displays
			 *timer_ptr = (int *) INTERVAL_TIMER_BASE,			//address location for interval timer
			 *JTAG_UART_ptr = (int *)JTAG_UART_BASE;	        //address location for UART bus



//Variables
bool fps = 0;													//This variable is used by timer and ISR


/*These constants are used to define the std starting values
 for the different objects in the game*/

//object player;
const object obj_player = { 10, 0, 0, 0, 0, hp_max, 0,
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
const object obj_obs = { 20, 0, 0, 0, 0, 0, 0,
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
		Button_val,												//used to store soft button values, can be optimized
		lives = starting_lives,									//number of player lives
		pt_total,												//used to hold the sum of the players points
		counter,												//used to store the interval counter value for loading
		i, j, k,												//i, j, k used for various counters									
		new_input;												//??


	/*these arrays hold the color values of all news pixels when
	moving the entire screen left, right, up or down.
	Current only hold a single blue pixel as a test case*/
	int new_pixels_lr[RES_Y - GAME_TOP] = { BLUE },
		new_pixels_ud[RES_X] = { BLUE };

	
	/*my thoughts here are to use the timer to count our frames
	per second, or how many times we are refreshing the entire
	screen. Basically every time the main while loop iterates it
	increments a counter. Once the timer interupts, that counter
	is sent to the hex display. Showing off, and giving us an
	idea of how fast our code is.*/
	//This segement of code sets up the timer to run
	*(timer_ptr) = 0;											//clears the interval timer status
	counter = ONESEC;											//sets the new count down value
	*(timer_ptr + 0x2) = (counter & 0xFFFF);					//loads the first half of the counter value
	*(timer_ptr + 0x3) = (counter >> 16) & 0xFFFF;				//loads the second half of the counter value

	//start interval timer looping, enable its interrupts
	*(timer_ptr + 1) = 0x7;										//STOP = 0, START = 1, CONT = 1, ITO = 1 

	NIOS2_WRITE_IENABLE(0x01);									//set interrupt mask bits for levels 0 (IntervalTimer)
	NIOS2_WRITE_STATUS(1);										//enable Nios II interrupts

	object entities[100];

	clear_screen(BLUE);

	new_input = *(JTAG_UART_ptr);								//reads the JTAG
	
	//loads some entities for testing
	entities[0] = obj_player;
	entities[0].x = 25;
	entities[0].y = 15;
	entities[0].i = 10;
	entities[0].j = 10;

	entities[1] = obj_obs;
	entities[1].x = 25;
	entities[1].y = 20;
	entities[1].i = 0;
	entities[1].j = 0;

	entities[2] = obj_obs;
	entities[2].x = 45;
	entities[2].y = 20;

	entities[3] = obj_obs;
	entities[3].x = res_x;
	/*entities[3].y = 20;
	entities[3].i = 0;
	entities[3].j = 0;
	
	entities[4] = obj_obs;
	entities[4].x = 25;
	entities[4].y = 20;
	entities[4].i = 0;
	entities[4].j = 0;*/

	for (i = 0; i < 2; i++)
	{
		draw_sprite(entities[i].x, entities[i].y, SPRITE_SIZE, &entities[i].sprite);
	}

	//for (i = 1; i < 6; i++)
	//{
	//    for (j = 0; j < ; j++)
	//   {
	//        draw_sprite(entities[i].x, entities[i].y, SPRITE_SIZE, &entities[i].sprite);
	//    }
	//}


	while (1)
	{
		if (new_input & pixel_buffer_start) {
			new_input = 0x000000FF;								// the new input will be stored in the least sig dig
		}


		switch (new_input) {

		case W:													// moves up
			entities[0].i = 0,
			entities[0].j = -1,
			entities[1].i = 0,
			entities[1].j = -1;

			//update_sprite(&entities[0].x, &entities[0].y, entities[0].i, entities[0].j, SPRITE_SIZE, &entities[0].sprite)
			//entities[0].i = 0, perhaps to implement gravity would mean have another update back down, im not sure
			//entities[0].j = 1,
			//entities[1].i = 0,
			//entities[1].j = 1;
			break;

		case A:													//moves left
			entities[0].i = -1,
			entities[0].j = 0,
			entities[1].i = -1,
			entities[1].j = 0;
			break;

		case S:													//moves down
			entities[0].i = 0,
			entities[0].j = 1,
			entities[1].i = 0,
			entities[1].j = 1;
			break;

		case D:													//moves left
			entities[0].i = 1,
			entities[0].j = 0,
			entities[1].i = 1,
			entities[1].j = 0;
			break;

		default:
			/*Rafael I don't know what you want to have
			happen, but I can almost garuntee this section
			isn't going to do what you want.
			you're doing a logical and with the value of
			entities[0].x and entities[1].x that test will
			evalute to true if either one is non-zero and be
			false if both are zero.
			I think what you want is more like:
			(entities[0].x <= 0 && entities[1].x <= 0)
			so you got the test on the left:
			entities[0].x <= 0
			and the test on the right:
			entities[1].x <= 0
			and your performing a logical and on the result
			of both*/
			if ((entities[0].x && entities[1].x) <= 0) {
				entities[0].x = 0;
				entities[1].x = 0;
			}

			//you could replace 70 with x_res - 1 
			//or > x_res
			else if ((entities[0].x && entities[1].x) >= 79) {
				entities[0].x = res_x;
				entities[1].x = res_x;
			}
			
			else if ((entities[0].y && entities[1].y) <= 0) {
				entities[0].y = 0;
				entities[1].y = 0;
			}

			//ditto to the 59 => y_res - 1
			else if ((entities[0].y && entities[1].y) >= 59) {
				entities[0].y = res_y;
				entities[1].y = res_y;
			}

			break;
		}

		//redraws sprite after keyboard input adjusts velocity
		update_sprite(&entities[0].x, &entities[0].y, entities[0].i, entities[0].j, SPRITE_SIZE, &entities[0].sprite);


		//set up collision
		if ((entities[0].x == entities[2].x && entities[0].y == entities[2].y)) {
			remove_sprite(entities[0].x, entities[0].y, SPRITE_SIZE);
		}


		for (i = 0; i < 2; i++)
		{
			draw_sprite(entities[i].x, entities[i].y, SPRITE_SIZE, &entities[i].sprite);
		}
	}
	

	//Code used to test visual functions
	/*while (1)
	{
		
		if (*SW_ptr & 0x01)
		{
			update_sprite(&entities[0].x, &entities[0].y, entities[0].i, entities[0].j, SPRITE_SIZE, &entities[0].sprite);
			entities[0].i = 0;
			entities[0].j = 0;
		}

		if (*SW_ptr & 0x02)
		{
			remove_sprite(entities[0].x, entities[0].y, SPRITE_SIZE);
		}

		if (*SW_ptr & 0x04)
		{
			adv_screen_lu(new_pixels_lr, new_pixels_ud);
		}

		if (*SW_ptr & 0x08)
		{
			adv_screen_ld(new_pixels_lr, new_pixels_ud);
		}

		if (*SW_ptr & 0x10)
		{
			adv_screen_ru(new_pixels_lr, new_pixels_ud);
		}

		if (*SW_ptr & 0x20)
		{
			adv_screen_rd(new_pixels_lr, new_pixels_ud);
		}

		if (*SW_ptr & 0x40)
		{
			adv_screen_d(new_pixels_ud);
		}

	}
	
}*/



/*this function is called by the exception handler when the interval
timer generates an interrupt. In previous work it was stored in a
seperate file. Here it is combined int to the main function.*/
void interval_timer_ISR()
{
	volatile int * SW_ptr = (int *)0x10000040;

	*(timer_ptr) = 0; 											//clear the interrupt
	fps = 1;													

	return;
}
