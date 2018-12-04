#include "nios2_ctrl_reg_macros.h"
#include "address_map_nios2.h"
#include <stdio.h>


//Predfined colors values
/*color is controled by 8 bits in the following pattern:
  R | G | B
 000 000 00
 */
#define BLACK 0													// 0 decimal = BLACK
#define WHITE 255												// 255 decimal = WHITE
#define BLUE 3													// 3 decimal = BLUE
#define GREEN 28												// 28 decimal = GREEN
#define RED 224													// 224 decimal = RED
#define CYAN 31													// 31 decimal = CYAN
#define MAGENTA	227												// 227 decimal = MAGENTA
#define YELLOW 252												// 248 decimal = YELLOW



//time values
#define FOURTHSEC 0x00BEBC20									//1/(50 MHz) x (0x00BEBC20) = 250 msec


//global Variables
volatile int timeout;											//used to synchronize with the timer
volatile int *interval_timer_ptr = INTERVAL_TIMER_BASE;			//pointer to interval timer
volatile int pixel_buffer_start = 0x08000000;					//VGA pixel buffer
const int res_x = 80,											//VGA screen width size
		  res_y = 60,											//VGA screen height size
		  color_back = YELLOW,									//Color of the background
		  box_size = 30;										//pixel size the 2 two box will be

																
//function prototypes
void plot_pixel(int, int, short int);
void clear_screen(short int);
void draw_hline(int, int, int, int, short int);
void draw_vline(int, int, int, int, short int);
void draw_rect(int, int, int, int, short int);
void draw_2box(int, int, short int, short int);
void interval_timer_ISR();



int main(void)
{

	int x,														//used to store the left edge of the 2 tone box
		x_0,													//previous location of left edge of the 2 tone box
		x_1,													//previous location of right edge of the 2 tone box
		x_dir,													//the direction in the x axis of travel
		y,														//used to store the top edge of the 2 tone box
		y_0,													//previous location of top edge of the 2 tone box
		y_1,													//previous location of bottom edge of the 2 tone box
		y_dir;													//the direction in the y axis of travel
															

	volatile int * PS2_ptr = (int *)0x10000100;					//PS/2 keyboard port address

	timeout = 0;												//Initialize the global "update line position" flag 	

	//This segement of code sets up the timer to run
	*(interval_timer_ptr) = 0;									//clears the interval timer status
	int counter = FOURTHSEC;									//sets the new count down value
	*(interval_timer_ptr + 0x2) = (counter & 0xFFFF);			//loads the first half of the counter value
	*(interval_timer_ptr + 0x3) = (counter >> 16) & 0xFFFF;		//loads the second half of the counter value

	//start interval timer looping, enable its interrupts
	*(interval_timer_ptr + 1) = 0x7;							//STOP = 0, START = 1, CONT = 1, ITO = 1 
	
	NIOS2_WRITE_IENABLE(0x01);									//set interrupt mask bits for levels 0 (IntervalTimer)
	NIOS2_WRITE_STATUS(1);										//enable Nios II interrupts
	
	clear_screen(color_back);									//make the VGA screen the background color	

	x = 25, y = 15, y_dir = 1;									//set the intial box position, and direction of travel
	draw_2box(x, y, BLUE, RED);									//draws the first line on the screen


	while (1)
	{
		//Wait for line position update cycle
		while (!timeout) {};

		//set values for old box location
		x_0 = x, x_1 = x + box_size, y_0 = y, y_1 = y + box_size;

		//Erase old box (overwrite using background color)
		draw_rect(x_0, x_1, y_0, y_1, color_back);

		//Boundary check (reverses direction at screen edge)
		if ((y_0 <= 0) || (y_1 >= res_y)) y_dir = -y_dir;

		//Change box position
		y = y + y_dir;

		//Draw new box
		draw_2box(x, y, BLUE, RED);

		//reset timer to wait for next timer reset
		timeout = 0;
	}
}



/*draws a single pixel at a point x, y of the desired color. The
origin is the top left. x & y are positive int values. x is the
distance in pixels from the left of the screen. y is the
distance in pixels from the top of the screen.*/
void plot_pixel(int x, int y, short int pixel_color)
{
	*(volatile char *)(pixel_buffer_start + (y << 7) + x) = pixel_color;
}



//makes the entire screen one color
void clear_screen(short int color)
{
	int y, x;

	for (y = 0; y < res_y; y++)
	{
		for (x = 0; x < res_x; x++) { plot_pixel(x, y, color); }
	}
}



//draws a horizontal line a color from x0 to x1 at height y0
void draw_hline(int x0, int x1, int y0, int y1, short int color)
{
	int x, y = y0;

	for (x = x0; x < x1; x++) { plot_pixel(x, y, color); }
}



//draws a vertical Line a color from y0 to y1 at column x0
void draw_vline(int x0, int x1, int y0, int y1, short int color)
{
	int x = x0, y;

	for (y = y0; y < y1; y++) { plot_pixel(x, y, color); }
}



//draws a rectangle of one color
void draw_rect(int x0, int x1, int y0, int y1, short int color)
{
	int x = x0, y = y0;

	for (y = y0; y < y1; y++)
	{
		for (x = x0; x < x1; x++) { plot_pixel(x, y, color); }
	}
}



//draws a square with a second sqaure 1/3 the size in the center 
void draw_2box(int x, int y, short int color1, short int color2)
{
	int one_third = box_size / 3,
		two_third = 2 * box_size / 3;

	draw_rect(x, x + box_size, y, y + one_third, color1);
	draw_rect(x, x + one_third, y + one_third, y + two_third, color1);
	draw_rect(x + one_third, x + two_third, y + one_third, y + two_third, color2);
	draw_rect(x + two_third, x + box_size, y + one_third, y + two_third, color1);
	draw_rect(x, x + box_size, y + two_third, y + box_size, color1);
}


/*this function is called by the exception handler when the interval
timer generates an interrupt. In previous work it was stored in a
seperate file. Here it is combined int to the main function.*/
void interval_timer_ISR()
{
	volatile int * SW_ptr = (int *)0x10000040;

	*(interval_timer_ptr) = 0; 										//clear the interrupt
	if (!(*SW_ptr & 0x00000001)) { timeout = 1; }					//set global variable if sliding sw0 is 0

	return;
}