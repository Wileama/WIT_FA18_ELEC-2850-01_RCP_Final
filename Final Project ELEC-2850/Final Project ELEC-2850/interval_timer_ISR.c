extern volatile int timeout;
#include <stdio.h>

/*****************************************************************************
 * Interval timer interrupt service routine
 *                                                                          
 * Controls signals start of line update cycle
 * 
******************************************************************************/

void interval_timer_ISR( )
{
	volatile int * interval_timer_ptr = (int *) 0x10002000;
	volatile int * SW_ptr = (int *) 0x10000040;

	*(interval_timer_ptr) = 0; 										//clear the interrupt
	if(!(*SW_ptr & 0x00000001)) { timeout = 1; }					//set global variable if sliding sw0 is 0

	return;
}

