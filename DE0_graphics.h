#pragma once

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


#define SPRITE_SIZE 5											//used to define the size of a sprite


//these arrays store the game sprites
typedef struct
{
	short int pixel[SPRITE_SIZE][SPRITE_SIZE];
}image;


//hardware addresses
volatile int pixel_buffer_start = 0x08000000;					//VGA pixel buffer


//useful constants
const int res_x = 80,											//VGA screen width size
		  res_y = 60,											//VGA screen height size
		  color_back = BLUE;									//Color of the background


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


/*draws a square game sprite when given x,y cordinates of the
top left of the sprite, size, and pointer to game sprite*/
void draw_sprite(int x, int y, short int size, image *sprite)
{
	int i, j;

	for (j = 0; j < size; j++)
	{
		for (i = 0; i < size; i++) { plot_pixel(x + i, y + j, (*sprite).pixel[j][i]); }
	}
}
