#include "address_map_nios2.h"
#include "DE0_graphics.h"
#include "game_obj.h"
#include <stdio.h>
#include <math.h>


//time values
#define ONESEC 0x02FAF080										//1/(50 MHz) x (0x02FAF080) = 1 sec
#define FOURTHSEC 0x00BEBC20									//1/(50 MHz) x (0x00BEBC20) = 250 msec


//Globals



//hardware addresses
volatile int *LED_ptr = GREEN_LED_BASE,							//address location for green LEDs 
			 *SW_ptr = SLIDER_SWITCH_BASE,						//address location for sliding switches
			 *button_ptr = PUSHBUTTON_BASE,						//address location for push buttons
			 *hex_ptr = HEX3_HEX0_BASE,							//address location for the hex displays
			 *timer_ptr = INTERVAL_TIMER_BASE;					//address location for interval timer


//constants
const health hp_max = 25; sp_max = 75;


//Variables
bool fps = 0;													//This variable is used by timer and ISR

//function prototypes
//functions used to draw objects
void new_obj(object *);
void move_obj(object *);
void remove_object(object *);


//functions used in player control
void move(object *);


//functions used in the game engine
void rand_obj(object *);
void calc_dmg(object *);
void destory(object *);

void main()
{
	int SW_val,
		Button_val,
		lives = 3,
		pt_total,
		counter;

	/*my thoughts here are to use the timer to count our frames
	per second, or how many times we are refreshing the entire
	screen. Basically every time the main while loop iterates it
	increments a counter. Once the timer interupts, that counter
	is sent to the hex display. Showing off, and giving us an
	idea of how fast our code is.*/
	//This segement of code sets up the timer to run
	*(timer_ptr) = 0;											//clears the interval timer status
	int counter = ONESEC;										//sets the new count down value
	*(timer_ptr + 0x2) = (counter & 0xFFFF);					//loads the first half of the counter value
	*(timer_ptr + 0x3) = (counter >> 16) & 0xFFFF;				//loads the second half of the counter value

	//start interval timer looping, enable its interrupts
	*(timer_ptr + 1) = 0x7;										//STOP = 0, START = 1, CONT = 1, ITO = 1 

	NIOS2_WRITE_IENABLE(0x01);									//set interrupt mask bits for levels 0 (IntervalTimer)
	NIOS2_WRITE_STATUS(1);										//enable Nios II interrupts


	/*These constants are used to define the std starting values
	for the different objects in the game*/
	const object player_obj = {10, 0, 0, 0, 0, hp_max, 0, 1, 0},
					obs_obj = {20, 0, 0, 0, 0, 0, 0, 1, 10};

	object entities[100];

 
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
