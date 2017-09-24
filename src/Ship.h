#ifndef SHIP_H
#define SHIP_H

#include <stdint.h>
#include <inttypes.h>

namespace std {

#define unk UINT32_MAX/2
#define water unk - 1
#define avail_min water
#define max_ships water - 2

typedef struct {
	uint32_t hit:1;
	uint32_t idn:32 -1;
}Spot;

#define MINE 0
#define ENEMIES 1
#define FAIL -1
#define board_dead -2

Spot unk_def = {.hit = false, .idn = unk};

class Coord {
	public:
	long y,x;
};

class Ship {
	public:
	Coord top_left;
	long height, width;
	
	Coord bot_right(){
		Coord br;
		br.y = top_left.y + height -1;
		br.x = top_left.x + width -1;
		return br;
	}
	
	bool not_exist(Coord sea_max){
		return (this->bot_right().y > sea_max.y-1 || this->bot_right().x > sea_max.x-1
		|| this->top_left.x < 0 || this->top_left.y < 0 || height <= 0 || width <= 0);
	}
};

}// namespace std
#endif