#pragma once
#include "DE0_graphics.h"

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


typedef struct
{
	bool map[SPRITE_SIZE][SPRITE_SIZE];
}solid;


/*location on the game screen
short -(2^15) [-32768] to (2^15)-1 [32767]*/
typedef short location;
//misc game values
//unsigned short 2^0 [0] to (2^16)-1 [65535]
typedef unsigned short velocity, health;


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


//constants
#define hp_max 25
#define sp_max 75

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