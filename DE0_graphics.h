#pragma once

//colors values
#define BLUE 3													// 3 decimal = BLUE
#define YELLOW 248												// 248 decimal = YELLOW
#define RED 224													// 224 decimal = RED


#define SPRITE_SIZE 5											//used to define the size of a sprite


//these arrays store the game sprites
typedef struct
{
	short int pixel[SPRITE_SIZE][SPRITE_SIZE];
}image;


//useful constants
const int res_x = 80,											//VGA screen width size
		  res_y = 60,											//VGA screen height size
		  color_back = BLUE;									//Color of the background


//fixed game sprites
const image img_player = { { {BLUE, BLUE, BLUE, BLUE, BLUE},
							 {BLUE, RED, BLUE, RED, BLUE},
							 {BLUE, BLUE, BLUE, BLUE, BLUE},
							 {BLUE, RED, RED, RED, BLUE},
							 {BLUE, BLUE, BLUE, BLUE, BLUE} } };

const image img_obst = { { {YELLOW,YELLOW,YELLOW,YELLOW,YELLOW},
						   {YELLOW,YELLOW,YELLOW,YELLOW,YELLOW},
						   {YELLOW,YELLOW,YELLOW,YELLOW,YELLOW},
						   {YELLOW,YELLOW,YELLOW,YELLOW,YELLOW},
						   {YELLOW,YELLOW,YELLOW,YELLOW,YELLOW},} };



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

