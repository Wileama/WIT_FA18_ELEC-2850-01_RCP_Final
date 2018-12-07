#pragma once
#include <time.h>
#include <stdlib.h>
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


//constants
#define RES_X 80												//VGA screen width size
#define RES_Y 60												//VGA screen height size																
#define SPRITE_SIZE 5											//used to define the size of a sprite
#define NUM_ENTITIES 100										//used to define the max number of active entities


/*used to define the top of the game screen. pixels on vertical 
row 4 and below will be the game world. pixels above this row
will provide the player with game info like health, lives,
points, etc.*/
#define GAME_TOP 4

//hardware addresses
volatile int pixel_buffer_start = 0x08000000;					//VGA pixel buffer


//useful constants
const int res_x = 80,											//VGA screen width size
		  res_y = 60,											//VGA screen height size
		  color_back = BLACK,									//Color of the background
		  color_solid = WHITE,									//Color used to define solid obects
		  color_points = GREEN,									//Color used to define bonus points
		  color_power = RED;									//color used to define power ups

int act_entities = 0;											//used to record the number of active entities


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
	int obj_id;													//used to identify the object 
	location x, y;												//distance from origin [center of game grid]
	velocity i, j;												//magnitude of objects velocity [m]
	health hp, sp;												//objects health value & shield value
	solid collision;											//an array of bool vaules to deterime if object is solid or not		object.collsion.map[y][x]
	image sprite;												//stores the sprite the object will use								object.sprite.pixel[y][x]
	unsigned points;											//objects point value, or point total
} object;


object entities[NUM_ENTITIES];									//this global array holds all info on game objects


/*draws a single pixel at a point x, y of the desired color. The
origin is the top left. x & y are positive int values. x is the
distance in pixels from the left of the screen. y is the
distance in pixels from the top of the screen.*/
void plot_pixel(location x, location y, int pixel_color)
{
	*(volatile char *)(pixel_buffer_start + (y << 7) + x) = pixel_color;
}


//returns the integer value of the pixel at position (x,y)
char read_pixel(location x, location y)
{
	char pixel_value;
	pixel_value = *(volatile char *)(pixel_buffer_start + (y << 7) + x);
	return pixel_value;
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



//draws a rectangle of one color using four points & a color
void draw_rect(location x0, location x1, location y0, location y1, int color)
{
	int x = x0, y = y0;

	for (y = y0; y < y1; y++)
	{
		for (x = x0; x < x1; x++) { plot_pixel(x, y, color); }
	}
}


/*draws a square game sprite when given x,y cordinates of the
top left of the sprite, and pointer to
the sprite image.*/
void draw_sprite(location x, location y, image *sprite)
{
	int i, j;

	for (j = 0; j < SPRITE_SIZE; j++)
	{
		for (i = 0; i < SPRITE_SIZE; i++) { plot_pixel(x + i, y + j, (*sprite).pixel[j][i]); }
	}
}


//draws over sprite at location with background color
void erase_sprite(location x, location y)
{
	draw_rect(x, x + SPRITE_SIZE, y, y + SPRITE_SIZE, color_back);
}


/*This function updates the screen buffer for one sprite based
on pointers to the sprites location and the sprites velocity.
Will update the location values. Also requires a pointer to
sprites image.*/
void move_sprite(location *x, location *y, velocity i, velocity j, image *sprite)
{
	erase_sprite(*x, *y);
	*x = (int) *x + i, *y = (int) *y + j;
	draw_sprite(*x, *y, sprite);
}


//updates all active sprites in the entities array
void move_all_sprites()
{
	int i;

	for (i = 0; i < act_entities; i++)
	{
		move_sprite(&entities[i].x, &entities[i].y, entities[i].i, entities[i].j, &entities[i].sprite);
	}
}


/*this function adds a sprite to the entities array, at the 
specified location, with the specified velocity. Then draws the 
new sprite, and increases the count of the active entities*/
void add_sprite(object sprite, location x, location y, velocity i, velocity j)
{

	entities[act_entities] = sprite,
	entities[act_entities].obj_id = time(NULL),
	entities[act_entities].x = x,
	entities[act_entities].y = y,
	entities[act_entities].i = i,
	entities[act_entities].j = j;

	draw_sprite(x, y, &entities[act_entities].sprite);

	act_entities++;

}


/*This function identifies the sprite to be deleted, and erases 
it. Then removes the sprite from the stack, and shifts every
other entity down to fill the hole. The variable indicating the 
number of active entities is then decremented.*/
void delete_sprite(int object_id)
{
	int i;
	bool removed = false;


	//checks to see if the top element first
	if (entities[act_entities].obj_id == object_id) {
		erase_sprite(entities[act_entities].x, entities[act_entities].y);
		act_entities--;
		return;
	}


	/*starts at the bottom of the array and works it's way up
	to the second to last entry.*/
	for (i = 0; i < act_entities - 1; i++)
	{
		/*if the sprite has been removed remaining entries are
		shifted down to fill the resulting hole.*/ 
		if (removed)
		{
			entities[i] = entities[i + 1];
		}
		
		//checks to see if this is the entitity to delete. 
		else
		{
			if (entities[i].obj_id == object_id) {
				erase_sprite(entities[i].x, entities[i].y);
				entities[i] = entities[i + 1];
				act_entities--;
				removed = true;
			}
		}
	}

}

/*redraws the entire screen shifted 1 pixel to the left.
Requires an array of new pixels to draw on the right hand side.
***Does not update object position data***

Works by redrawing the enter screen starting from the top left
corner of the screen and advancing to the right. Each pixel is
set to the value of the next pixel in the proccess. At the right
most edge of the screen the function draws the color of a new 
pixel as specified by an array. The function then moves down a
row and repeats for the entire screen.*/
void adv_screen_l(int right_pixel[RES_Y - GAME_TOP])
{
	int y, x;

	for (y = GAME_TOP; y < res_y; y++)
	{
		for (x = 0; x < res_x - 1; x++) { plot_pixel(x, y, read_pixel(x+1, y)); }
		//draws the new pixel on the right side of the screen
		plot_pixel(res_x - 1, y, right_pixel[y - GAME_TOP]);
	}
}


/*redraws the entire screen shifted 1 pixel to the right.
Requires an array of new pixels to draw on the left hand side.
***Does not update object position data***

Works by redrawing the enter screen starting from the top right
corner of the screen and advancing to the left. Each pixel is
set to the value of the next pixel in the proccess. At the left
most edge of the screen the function draws the color of a new
pixel as specified by an array. The function then moves down a
row and repeats for the entire screen.*/
void adv_screen_r(int left_pixel[RES_Y - GAME_TOP])
{
	int y, x;

	for (y = GAME_TOP; y < res_y; y++)
	{
		for (x = res_x - 1; x > 0 ; x--) { plot_pixel(x, y, read_pixel(x - 1, y)); }
		//draws the new pixel on the left side of the screen
		plot_pixel(0, y, left_pixel[y - GAME_TOP]);
	}
}


/*redraws the entire screen shifted 1 pixel up. Requires an
array of new pixels to draw on the bottom hand side.
***Does not update object position data***

Works by redrawing the enter screen starting from the top left
corner of the screen and advancing to the right. Each pixel is
set to the value of the pixel below it. At the right most edge of
the screen the function moves down a row and repeats the
proccess. Upon reaching the final row the screen now draws the
new pixels as specified by an array.*/
void adv_screen_u(int bottom_pixel[RES_X])
{
	int y, x;

	for (y = GAME_TOP; y < res_y - 1; y++)
	{
		for (x = 0; x < res_x; x++) { plot_pixel(x, y, read_pixel(x , y + 1)); }
	}
	
	//draws the bottom most row
	y = res_y - 1;
	for (x = 0; x < res_x; x++) { plot_pixel(x, y, bottom_pixel[x]); }
}


/*redraws the entire screen shifted 1 pixel up. Requires an
array of new pixels to draw on the bottom hand side.
***Does not update object position data***

Works by redrawing the enter screen starting from the bottom
left corner of the screen and advancing to the right. Each pixel
is set to the value of the pixel above it. At the right most edge
of the screen the function moves up a row and repeats the
proccess. Upon reaching the final row the screen now draws the
new pixels as specified by an array.*/
void adv_screen_d(int top_pixel[RES_X])
{
	int y, x;


	for (y = res_y - 1; y > GAME_TOP; y--)
	{
		for (x = 0; x < res_x; x++) { plot_pixel(x, y, read_pixel(x, y - 1)); }
		plot_pixel(res_x - 1, y, top_pixel[y - GAME_TOP]);
	}

	//draws the top most row
	y = GAME_TOP;
	for (x = 0; x < res_x; x++) { plot_pixel(x, y, top_pixel[x]); }
}


/*These fucnctions are a combinations of the ones above,
performing both steps in a single redraw. Thus improving speed.*/
//advance left and up
void adv_screen_lu(int right_pixel[RES_Y - GAME_TOP], int bottom_pixel[RES_X])
{
	int y, x;

	for (y = GAME_TOP; y < res_y - 1; y++)
	{
		for (x = 0; x < res_x - 1; x++) { plot_pixel(x, y, read_pixel(x + 1, y + 1)); }
		//draws the new pixel on the right side of the screen
		plot_pixel(res_x - 1, y, right_pixel[y - GAME_TOP]);
	}

	//draws the bottom most row
	y = res_y - 1;
	for (x = 0; x < res_x; x++) { plot_pixel(x, y, bottom_pixel[x]); }

}


//advance left and down
void adv_screen_ld(int right_pixel[RES_Y - GAME_TOP], int top_pixel[RES_X])
{
	int y, x;

	for (y = res_y - 1; y > GAME_TOP; y--)
	{
		for (x = 0; x < res_x - 1; x++) { plot_pixel(x, y, read_pixel(x + 1, y - 1)); }
		//draws the new pixel on the right side of the screen
		plot_pixel(res_x - 1, y, right_pixel[y - GAME_TOP]);
	}

	//draws the top most row
	y = GAME_TOP;
	for (x = 0; x < res_x; x++) { plot_pixel(x, y, top_pixel[x]); }

}


//advance right and up
void adv_screen_ru(int left_pixel[RES_Y - GAME_TOP], int bottom_pixel[RES_X])
{
	int y, x;

	for (y = GAME_TOP; y < res_y - 1; y++)
	{
		for (x = res_x - 1; x > 0; x--) { plot_pixel(x, y, read_pixel(x - 1, y + 1)); }
		//draws the new pixel on the left side of the screen
		plot_pixel(0, y, left_pixel[y - GAME_TOP]);
	}

	//draws the bottom most row
	y = res_y - 1;
	for (x = res_x - 1; x > 0; x--) { plot_pixel(x, y, bottom_pixel[x]); }

}


//advance right and down
void adv_screen_rd(int left_pixel[RES_Y - GAME_TOP], int top_pixel[RES_X])
{
	int y, x;

	for (y = res_y - 1; y > GAME_TOP; y--)
	{
		for (x = res_x - 1; x > 0; x--) { plot_pixel(x, y, read_pixel(x - 1, y - 1)); }
		//draws the new pixel on the left side of the screen
		plot_pixel(res_x - 1, y, left_pixel[y - GAME_TOP]);
	}

	//draws the top most row
	y = GAME_TOP;
	for (x = res_x - 1; x > 0; x--) { plot_pixel(x, y, top_pixel[x]); }

}









//game functions
/*This functions checks the pixels underneath a sprite to see
if the color indicates a solid object. If a solid is found
beneath the sprite the function returns true, otherwise false is 
returned*/
bool gnd_chk(location x, location y)
{
	int i;

	for (i = 0; i < SPRITE_SIZE; i++)
	{
		if (read_pixel(x + i, y + SPRITE_SIZE ) == color_solid) { return 1; }
	}

	return 0;	
}



/*This function looks at all the pixels between the avatar and  
it's destination. This is done by reading the pixel values of   
two rectangles. Pixel color detefines the interation.*/
int collision_chk(location x, location y, velocity i, velocity j)
{
	int a, b, c, collision, offset_lr, offset_ud, temp;

	collision = 0;																																																																																																																																																																																																																									

	if (i > 0) { offset_lr = x + SPRITE_SIZE; }
	else {offset_lr = 0}

	for (b = 0; b <= SPRITE_SIZE + j; b++)
	{
		if (b < j) { c = (i / j) * b; }
		else { c = i; }
		
		for (a = 0; a <= c; a++)
		{
			//add an if and else here to choose the whether to check the right or left side
			temp = read_pixel(offset_lr + a, y + b);

			//this is the error checking, leave it after the read as not to fuck up the read.
			plot_pixel( offset_lr + a, y + b, YELLOW);

			if (temp == color_solid) { collision | 0x0001; printf("COLLISION"); }

			else if (temp == color_points) { collision | 0x0002; }

			else if (temp == color_power) { collision | 0x0004; }
		}

	}

	if (i > 0) { offset_ud = y + SPRITE_SIZE; }
	else { offset_ud = 0 }
	
	for (a = 0; a <= SPRITE_SIZE + i; a++)
	{
		if (a < i) { c = (j / i) * a; }
		else { c = j; }

		for (b = 0; b <= c; b++)
		{
			//add an if and else here to choose the whether to check the right or left side
			temp = read_pixel(x + a, offset_ud + b);

			plot_pixel(x + a, offset_ud + b, YELLOW);

			if (temp == color_solid) { collision | 0x0001; printf("COLLISION");
			}
			
			else if (temp == color_points) { collision | 0x0002; }

			else if (temp == color_power) { collision | 0x0004; }
		}

	}

	return collision;
}

