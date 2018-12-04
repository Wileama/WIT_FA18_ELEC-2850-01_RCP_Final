#include "address_map_nios2.h"
#include "nios2_ctrl_reg_macros.h"
#include "game.h"
#include <stdio.h>


//time values
#define ONESEC 0x02FAF080										//1/(50 MHz) x (0x02FAF080) = 1 sec
#define FOURTHSEC 0x00BEBC20									//1/(50 MHz) x (0x00BEBC20) = 250 msec


//Globals
//hardware addresses
volatile int *LED_ptr = (int *) GREEN_LED_BASE,					//address location for green LEDs 
			 *SW_ptr = (int *) SLIDER_SWITCH_BASE,				//address location for sliding switches
			 *button_ptr = (int *) PUSHBUTTON_BASE,				//address location for push buttons
			 *hex_ptr = (int *) HEX3_HEX0_BASE,					//address location for the hex displays
			 *timer_ptr = (int *) INTERVAL_TIMER_BASE;			//address location for interval timer


//Variables
bool fps = 0;													//This variable is used by timer and ISR


void main()
{
	int SW_val,
		Button_val,
		lives = 3,
		pt_total,
		counter,
		i;

	int new_pixels_lr[RES_Y] = { BLUE };
	int new_pixels_ud[RES_X] = { BLUE };

	

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

	clear_screen(BLACK);

	entities[0] = obj_player;
	entities[1] = obj_obs;

	entities[0].x = 25;
	entities[0].y = 15;
	entities[0].i = 10;
	entities[0].j = 10;

	entities[1].x = 25;
	entities[1].y = 20;
	entities[1].i = 0;
	entities[1].j = 0;



	for (i = 0; i < 2; i++)
	{
		draw_sprite(entities[i].x, entities[i].y, SPRITE_SIZE, &entities[i].sprite);
	}
	
	while (1)
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
			adv_screen_l(new_pixels_lr);
		}

		if (*SW_ptr & 0x08)
		{
			adv_screen_r(new_pixels_lr);
		}

		if (*SW_ptr & 0x10)
		{
			adv_screen_u(new_pixels_ud);
		}

		if (*SW_ptr & 0x20)
		{
			adv_screen_u(new_pixels_ud);
		}
	}
	
}



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
