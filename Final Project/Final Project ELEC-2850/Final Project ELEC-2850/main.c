#include "stdio.h"
#include "math.h"

//this enum creates boolean vars
typedef enum { false, true } bool;

volatile int *LED_ptr = 0x1000001F,			//address location for green LEDs 
			 *SW_ptr = 0x1000002F,			//address location for sliding switches
			 *Button_ptr = 0x1000005F,		//address location for push buttons
			 *Timer_ptr = 0x1000201F;		//address location for interval timer

/*used to define the types of objects the
game will have*/
typedef enum {
	empty,
	player = 10,
	small_asteriod = 20,
	medium_asteriod,
	large_asteriod,
	power_up = 30,
	bonus = 40
} object_class;

/*creates labels & definitions for certain game values.
loca
short -(2^15) [-32768] to (2^15)-1 [32767]*/
typedef short location;
//unsigned short 2^0 [0] to (2^16)-1 [65535]
typedef unsigned short velocity, health, oxygen, heat;
//double = 15 decimal places
typedef double angle;

/*used to define info required for a game
object*/
typedef struct
{
	object_class type;							//Object class, as define by an enum
	location x, y;								//distance from origin [center of game grid]
	velocity i, j, m, w;						//magnitude of objects velocity [m] and rotation [w]
	angle O;									//angle object is pointing
	health hp, sp;								//objects health value & shield value
	oxygen oxy;									//objects amount of oxygen
	heat h_eng;									//objects amount of heat engery
	unsigned points;
} object;


/*This structure descibes which of the players
systems are turned on or off. This has affects
on gameplay weapon 1-3 affect which weapons are
on and the speed at which they fire. Shield 1-3
affect how many shield points the player has and
how quickly they regenerator. Radar allows the
player to see other objects on the screen. The
life support system refills the players oxygen
meter keeping them alive. The engines allow the
player to thrust and rotate their ship. The
radiator is a liquid droplet radiator. This very
effectively cools the ship, but disables it's
ability to manuever. IE it lowers the ship heat
energy more rapidly, but prevents the player
from manuvering. I'm wornder if it would be
better to have an array of bools of a structure
of bools. I think the array would be easier to
iterate through, but the structure would be
conceptually easier to work with. The structure
should also be easier to update.*/

typedef struct 
{
	bool wpn1,
		 wpn2,
		 wpn3,
		 shield1,
		 shield2,
		 shield3,
		 eng,
		 radar,
		 air,
		 rad;
} systems;

//bool systems[] = { 0,							//weapon system 1
//				   0,							//weapon system 2
//				   0,							//weapon system 3
//				   0,							//shield generator 1
//				   0,							//shield generator 2
//				   0,							//shield generator 3
//				   0,							//engines
//				   0,							//radar
//				   0,							//life support
//				   0 };							//liquid radiator

/*These constants are useful shortcuts we might 
need while calculating, but also serve as single
point to update if values change*/
const angle right = 0, up = 90, left = 180, down = 270;
const velocity m_max = 50000, w_max = 1440;
const health hp_max = 25, sp_max = 75;
const oxygen oxy_max = 100;
const heat heat_max = 100;

//functions used to draw objects
void draw_obj(object *);

//functions used in player control
void thrust(object *);
void rotate(object *);
void power(systems *);

//functions used in the game engine
void rand_obj(object *);
void calc_dmg(object *);
void calc_air(object *);
void calc_heat(object *);
void destory(object *);

void main()
{
	int SW_val,
		Button_val,
		lives = 3,
		pt_total;

	/*These constants are used to define the std
	starting values for the different objects in
	the game*/
	const object player_obj = {10, 0, 0, 0, 0, up, hp_max, 0, oxy_max, 0, 0},
					 sm_Ast = {20, 0, 0, 0, 0, 0, 10, 0, 0, 0, 10},
					med_Ast = {21, 0, 0, 0, 0, 0, 25, 0, 0, 0, 25},
					 lg_Ast = {22, 0, 0, 0, 0, 0, 50, 0, 0, 0, 50};

	object entities[100];

	//systems player_sys = { 0, 0, 0, 0, 0, 0, 1, 1, 1, 0 };
 
}

