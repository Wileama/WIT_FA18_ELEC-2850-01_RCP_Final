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


//constants
#define hp_max 25												//Max health value
#define sp_max 75												//Max shield value

//hardware addresses
volatile int pixel_buffer_start = 0x08000000;					//VGA pixel buffer


//useful constants
const int res_x = 80,											//VGA screen width size
		  res_y = 60,											//VGA screen height size
		  color_back = BLACK;									//Color of the background


//custom enum/typedef/structures
//this enum creates boolean vars
typedef enum { false, true } bool;


//used to define the types of objects the game will have
typedef enum {
	empty,
	player = 10,
	obstacle = 20,
	bakground = 30,
	power_up = 40,
	bonus = 50
} object_class;


/*this array could be used to describe locations where the
sprite is solid*/
typedef struct
{
	bool map[SPRITE_SIZE][SPRITE_SIZE];
}solid;


/*these arrays store arrays of color values used to draw
sprites*/
typedef struct
{
	short int pixel[SPRITE_SIZE][SPRITE_SIZE];
}image;


/*velocity = # of pixels to move
	neg i = left
	pos i = right
	neg j = up
	pos j = down
short -(2^15) [-32768] to (2^15)-1 [32767]*/
typedef short int velocity;


/*location = distance in pixels from origin [top left]
health used to store hit points 0 = dead
unsigned short 2^0 [0] to (2^16)-1 [65535]*/
typedef unsigned short location, health;


//used to define info required for an in-game object
//req's DEO_graphics
typedef struct
{
	object_class type;											//Object class, as define by an enum
	location x, y;												//distance from origin [center of game grid]
	velocity i, j;												//magnitude of objects velocity [m]
	health hp, sp;												//objects health value & shield value
	solid collision;											//an array of bool vaules to deterime if object is solid or not		object.collsion.map[y][x]
	image sprite;												//stores the sprite the object will use								object.sprite.pixel[y][x]
	unsigned points;											//objects point value, or point total
} object;


/*These constants are used to define the std starting values
 for the different objects in the game*/

//object player_obj;

//player_obj.collision = col_player;


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


/*draws a single pixel at a point x, y of the desired color. The
origin is the top left. x & y are positive int values. x is the
distance in pixels from the left of the screen. y is the
distance in pixels from the top of the screen.*/
void plot_pixel(location x, location y, int pixel_color)
{
	*(volatile char *)(pixel_buffer_start + (y << 7) + x) = pixel_color;
}


//makes the entire screen one color
void clear_screen(int color)
{
	int y, x;

	for (y = 0; y < res_y; y++)
	{
		for (x = 0; x < res_x; x++) { plot_pixel(x, y, color); }
	}
}



//draws a horizontal line a color from x0 to x1 at height y0
void draw_hline(location x0, location x1, location y0, location y1, int color)
{
	int x, y = y0;

	for (x = x0; x < x1; x++) { plot_pixel(x, y, color); }
}



//draws a vertical Line a color from y0 to y1 at column x0
void draw_vline(location x0, location x1, location y0, location y1, int color)
{
	int x = x0, y;

	for (y = y0; y < y1; y++) { plot_pixel(x, y, color); }
}



//draws a rectangle of one color
void draw_rect(location x0, location x1, location y0, location y1, int color)
{
	int x = x0, y = y0;

	for (y = y0; y < y1; y++)
	{
		for (x = x0; x < x1; x++) { plot_pixel(x, y, color); }
	}
}


/*draws a square game sprite when given x,y cordinates of the
top left of the sprite, size, and pointer to game sprite*/
void draw_sprite(location x, location y, int size, image *sprite)
{
	int i, j;

	for (j = 0; j < size; j++)
	{
		for (i = 0; i < size; i++) { plot_pixel(x + i, y + j, (*sprite).pixel[j][i]); }
	}
}


void remove_sprite(location x, location y, int size)
{
	draw_rect(x, x + size, y, y + size, color_back);
}


/*This function updates the screen buffer for one sprite based
on pointers to the sprites location and the sprites velocity.*/
void update_sprite(location *x, location *y, velocity i, velocity j, int size, image *sprite)
{
	remove_sprite(*x, *y, size);
	*x = (int) *x + i, *y = (int) *y + j;
	draw_sprite(*x, *y, size, sprite);
}