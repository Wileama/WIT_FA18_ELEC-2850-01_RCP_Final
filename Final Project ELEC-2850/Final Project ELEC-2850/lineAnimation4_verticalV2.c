#include "nios2_ctrl_reg_macros.h"
#include <stdio.h>



/* This program implements a basic animation by drawing a VERTICAL line that moves 
 * across the screen.  
 * Global variable "timeout" is set=1 by the intervalTimerISR(). 
 * Whenever it detects that timeout=1, the main() function updates the line position,
 * sets timeout=0, then waits for the intervalTimer to set timeout=1 again. 
 * When timeout=1 the loop is repeated.
 */

volatile int timeout;				// used to synchronize with the timer
volatile int pixel_buffer_start = 0x08000000;// VGA pixel buffer
int resolution_x = 80; // VGA screen size
int resolution_y = 60; // VGA screen size


void clear_screen(void);
void draw_hline(int, int, int, int, int);
void draw_vline(int, int, int, int, int);
void draw_line(int, int, int, int, int);
void plot_pixel(int, int, short int);


int main(void)
{
	
	
	
	int x_0, y, x_1, y_dir, color;
	int y_0, x, y_1, x_dir;	
	//volatile int * pixel_ctrl_ptr = (int *) PIXEL_BUF_CTRL_BASE; pixel controller
	
	volatile int * interval_timer_ptr = (int *) 0x10002000;	// interal timer base address
	volatile int * PS2_ptr = (int *) 0x10000100; // PS/2 keyboard port address

	/* initialize some variables */
	
	timeout = 0;					// Initialize the global "update line position" flag 


	/* set the interval timer period for scrolling the HEX displays */
	int counter = 0x960000;				// 1/(50 MHz) x (0x960000) ~= 200 msec
	//int counter = 0x02FAF080;				// 1/(50 MHz) x (0x02FAF080) = 1sec
	*(interval_timer_ptr + 0x2) = (counter & 0xFFFF);
	*(interval_timer_ptr + 0x3) = (counter >> 16) & 0xFFFF;

	/* start interval timer, enable its interrupts */
	*(interval_timer_ptr + 1) = 0x7;	// STOP = 0, START = 1, CONT = 1, ITO = 1 
	
  
	NIOS2_WRITE_IENABLE( 0x01 );	// set interrupt mask bits for levels 0 (IntervalTimer)

	NIOS2_WRITE_STATUS( 1 );		// enable Nios II interrupts



	clear_screen ( );					// Make the VGA screen BLUE	
	y_0 = 4;
	y_1 = 55;
	x = 30;
	x_dir = 1;

	color = 224;	// 224 decimal = RED
	//draw_line (x_0, y, x_1, y, color);
		draw_vline (y_0, x, y_1, x, color); // NOTE draw_sline

	while (1)
	{
		// Wait for line position update cycle
		while(!timeout)
		{};
	
		// Only gets here when the intervalTimer_ISR 
		// has set global variable timeout = 1
		
		//Erase old line (re-draw using background color)
		color = 3;	// BLUE
		
		//draw_line (x_0, y, x_1, y, color);
		draw_vline (y_0, x, y_1, x, color);
		
		// Change line position.
		x = x + x_dir;
		
        //Boundary check
		if ((x <= 0) || (x >= 79)) x_dir = -x_dir;
		
		// Draw new line
		color = 224;	// RED
		//draw_line (x_0, y, x_1, y, color);
		draw_vline (y_0, x, y_1, x, color);
		
	    /* Set global variable timeout=0,  and loop back to wait for it to be 
		 * set=1 by the IntervalTimerISR()
		 */
		 timeout = 0;
	}
}






/* Function to blank the VGA screen */

/*
void clear_screen( )
{
	int y, x;
	int pixel_ptr;

	for (y = 0; y < resolution_y; y++)
	{
		for (x = 0; x < resolution_x; x++)
		{
			volatile char * pixel_ptr = (volatile char *) (pixel_buffer_start + (y << 7)+ x);
			*(pixel_ptr) = 3;	// clear pixel 
			pixel_ptr += 1;	// For DE0-CV increment by 2 because each pixel takes two byte addresses
		}
	}
}

*/

void clear_screen( )
{
	int clearColor = 3; // BLUE
	
/*  Normally make the VGA screen BLACK. 
	BLUE for highlighting erasure here.
	for DE0-CV substitute 0x001F
*/	
	int y, x;
	int pixel_ptr;

	for (y = 0; y < resolution_y; y++)
	{
		for (x = 0; x < resolution_x; x++)
		{		
			plot_pixel(x, y, clearColor);	// clear pixel			
		}
	}
}






/* Horizontal Line */
void draw_hline(int x0, int y0, int x1, int y1, int color)
{
	int y=y0;
	int x;
	
	for(x=x0;x<x1;x++)
	{
		plot_pixel(x,y,color);
	}
}

/* Vertical Line */
void draw_vline(int y0, int x0, int y1, int x1, int color)
{
	int x=x0;
	int y;
	
	for(y=y0;y<y1;y++)
	{
		plot_pixel(x,y,color);
	}
}

 

/*
void plot_pixel(int x, int y, short int line_color)
{
	*(volatile char *)(pixel_buffer_start + (y << 7)+x) = line_color;
	// *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color; For DE0-CV
}
*/



void plot_pixel(int x, int y, short int pixel_color)
{
	*(volatile char *)(pixel_buffer_start + (y << 7)+x) = pixel_color;
	// *(volatile short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = pixel_color; For DE0-CV
}