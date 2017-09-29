#ifndef SHIP_H
#define SHIP_H

#include <stdint.h>
#include <inttypes.h>

namespace std {

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